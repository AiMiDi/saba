﻿//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "MMDIkSolver.h"

#include <algorithm>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>

namespace saba
{
	MMDIkSolver::MMDIkSolver()
		: m_ikNode(nullptr)
		, m_ikTarget(nullptr)
		, m_iterateCount(1)
		, m_limitAngle(glm::pi<float>() * 2.0f)
		, m_enable(true)
		, m_baseAnimEnable(true)
	{
	}

	void MMDIkSolver::AddIKChain(MMDNode * node, const bool isKnee)
	{
		m_chains.emplace_back(node,
			isKnee,
			isKnee ? glm::vec3(glm::radians(0.5f), 0, 0) : glm::vec3(0.0f),
			isKnee ? glm::vec3(glm::radians(180.0f), 0, 0) : glm::vec3(0.0f),
			glm::quat(1, 0, 0, 0));
	}

	void MMDIkSolver::AddIKChain(
		MMDNode * node,
		const bool axisLimit,
		const glm::vec3 & limitMin,
		const glm::vec3 & limitMax
	)
	{
		m_chains.emplace_back(node, axisLimit, limitMin, limitMax, glm::quat(1, 0, 0, 0));
	}

	void MMDIkSolver::Solve()
	{
		if (!m_enable)
		{
			return;
		}

		if (m_ikNode == nullptr || m_ikTarget == nullptr)
		{
			// wrong ik
			return;
		}

		// Initialize IKChain
		for (auto& chain : m_chains)
		{
			chain.m_prevAngle = glm::vec3(0);
			chain.m_node->SetIKRotate(glm::quat(1, 0, 0, 0));
			chain.m_planeModeAngle = 0;

			chain.m_node->UpdateLocalTransform();
			chain.m_node->UpdateGlobalTransform();
		}

		float maxDist = std::numeric_limits<float>::max();
		for (uint32_t i = 0; i < m_iterateCount; i++)
		{
			SolveCore(i);

			auto targetPos = glm::vec3(m_ikTarget->GetGlobalTransform()[3]);
			auto ikPos = glm::vec3(m_ikNode->GetGlobalTransform()[3]);
			const float dist = length(targetPos - ikPos);
			if (dist < maxDist)
			{
				maxDist = dist;
				for (auto& chain : m_chains)
				{
					chain.m_saveIKRot = chain.m_node->GetIKRotate();
				}
			}
			else
			{
				for (auto& chain : m_chains)
				{
					chain.m_node->SetIKRotate(chain.m_saveIKRot);
					chain.m_node->UpdateLocalTransform();
					chain.m_node->UpdateGlobalTransform();
				}
				break;
			}
		}
	}

	namespace
	{
		float NormalizeAngle(const float angle)
		{
			float ret = angle;
			while (ret >= glm::two_pi<float>())
			{
				ret -= glm::two_pi<float>();
			}
			while (ret < 0)
			{
				ret += glm::two_pi<float>();
			}

			return ret;
		}

		float DiffAngle(const float a, const float b)
		{
			const float diff = NormalizeAngle(a) - NormalizeAngle(b);
			if (diff > glm::pi<float>())
			{
				return diff - glm::two_pi<float>();
			}
			if (diff < -glm::pi<float>())
			{
				return diff + glm::two_pi<float>();
			}
			return diff;
		}

		glm::vec3 Decompose(const glm::mat3& m, const glm::vec3& before)
		{
			glm::vec3 r;
			float sy = -m[0][2];
			constexpr float e = 1.0e-6f;
			if (1.0f - std::abs(sy) < e)
			{
				r.y = std::asin(sy);
				// 180°に近いほうを探す
				float sx = std::sin(before.x);
				float sz = std::sin(before.z);
				if (std::abs(sx) < std::abs(sz))
				{
					// Xのほうが0または180
					float cx = std::cos(before.x);
					if (cx > 0)
					{
						r.x = 0;
						r.z = std::asin(-m[1][0]);
					}
					else
					{
						r.x = glm::pi<float>();
						r.z = std::asin(m[1][0]);
					}
				}
				else
				{
					float cz = std::cos(before.z);
					if (cz > 0)
					{
						r.z = 0;
						r.x = std::asin(-m[2][1]);
					}
					else
					{
						r.z = glm::pi<float>();
						r.x = std::asin(m[2][1]);
					}
				}
			}
			else
			{
				r.x = std::atan2(m[1][2], m[2][2]);
				r.y = std::asin(-m[0][2]);
				r.z = std::atan2(m[0][1], m[0][0]);
			}

			constexpr auto pi = glm::pi<float>();
			glm::vec3 tests[] =
			{
				{ r.x + pi, pi - r.y, r.z + pi },
				{ r.x + pi, pi - r.y, r.z - pi },
				{ r.x + pi, -pi - r.y, r.z + pi },
				{ r.x + pi, -pi - r.y, r.z - pi },
				{ r.x - pi, pi - r.y, r.z + pi },
				{ r.x - pi, pi - r.y, r.z - pi },
				{ r.x - pi, -pi - r.y, r.z + pi },
				{ r.x - pi, -pi - r.y, r.z - pi },
			};

			float errX = std::abs(DiffAngle(r.x, before.x));
			float errY = std::abs(DiffAngle(r.y, before.y));
			float errZ = std::abs(DiffAngle(r.z, before.z));
			float minErr = errX + errY + errZ;
			for (const auto test : tests)
			{
				float err = std::abs(DiffAngle(test.x, before.x))
					+ std::abs(DiffAngle(test.y, before.y))
					+ std::abs(DiffAngle(test.z, before.z));
				if (err < minErr)
				{
					minErr = err;
					r = test;
				}
			}
			return r;
		}
	}

	void MMDIkSolver::SolveCore(uint32_t iteration)
	{
		auto ikPos = glm::vec3(m_ikNode->GetGlobalTransform()[3]);
		//for (auto& chain : m_chains)
		for (size_t chainIdx = 0; chainIdx < m_chains.size(); chainIdx++)
		{
			auto& chain = m_chains[chainIdx];
			MMDNode* chainNode = chain.m_node;
			if (chainNode == m_ikTarget)
			{
				/*
				ターゲットとチェインが同じ場合、 chainTargetVec が0ベクトルとなる。
				その後の計算で求める回転値がnanになるため、計算を行わない
				対象モデル：ぽんぷ長式比叡.pmx
				*/
				continue;
			}

			if (chain.m_enableAxisLimit)
			{
				// X,Y,Z 軸のいずれかしか回転しないものは専用の Solver を使用する
				if ((chain.m_limitMin.x != 0 || chain.m_limitMax.x != 0) &&
					(chain.m_limitMin.y == 0 || chain.m_limitMax.y == 0) &&
					(chain.m_limitMin.z == 0 || chain.m_limitMax.z == 0)
					)
				{
					SolvePlane(iteration, chainIdx, SolveAxis::X);
					continue;
				}
				if ((chain.m_limitMin.y != 0 || chain.m_limitMax.y != 0) &&
					(chain.m_limitMin.x == 0 || chain.m_limitMax.x == 0) &&
					(chain.m_limitMin.z == 0 || chain.m_limitMax.z == 0)
				)
				{
					SolvePlane(iteration, chainIdx, SolveAxis::Y);
					continue;
				}
				if ((chain.m_limitMin.z != 0 || chain.m_limitMax.z != 0) &&
					(chain.m_limitMin.x == 0 || chain.m_limitMax.x == 0) &&
					(chain.m_limitMin.y == 0 || chain.m_limitMax.y == 0)
					)
				{
					SolvePlane(iteration, chainIdx, SolveAxis::Z);
					continue;
				}
			}

			auto targetPos = glm::vec3(m_ikTarget->GetGlobalTransform()[3]);

			auto invChain = inverse(chain.m_node->GetGlobalTransform());

			auto chainIkPos = glm::vec3(invChain * glm::vec4(ikPos, 1));
			auto chainTargetPos = glm::vec3(invChain * glm::vec4(targetPos, 1));

			auto chainIkVec = normalize(chainIkPos);
			auto chainTargetVec = normalize(chainTargetPos);

			auto dot = glm::dot(chainTargetVec, chainIkVec);
			dot = glm::clamp(dot, -1.0f, 1.0f);

			float angle = std::acos(dot);
			float angleDeg = glm::degrees(angle);
			if (angleDeg < 1.0e-3f)
			{
				continue;
			}
			angle = glm::clamp(angle, -m_limitAngle, m_limitAngle);
			auto cross = normalize(glm::cross(chainTargetVec, chainIkVec));
			auto rot = rotate(glm::quat(1, 0, 0, 0), angle, cross);

			auto chainRot = chainNode->GetIKRotate() * chainNode->AnimateRotate() * rot;
			if (chain.m_enableAxisLimit)
			{
				auto chainRotM = mat3_cast(chainRot);
				auto rotXYZ = Decompose(chainRotM, chain.m_prevAngle);
				glm::vec3 clampXYZ;
				clampXYZ = clamp(rotXYZ, chain.m_limitMin, chain.m_limitMax);

				clampXYZ = clamp(clampXYZ - chain.m_prevAngle, -m_limitAngle, m_limitAngle) + chain.m_prevAngle;
				auto r = rotate(glm::quat(1, 0, 0, 0), clampXYZ.x, glm::vec3(1, 0, 0));
				r = rotate(r, clampXYZ.y, glm::vec3(0, 1, 0));
				r = rotate(r, clampXYZ.z, glm::vec3(0, 0, 1));
				chainRotM = mat3_cast(r);
				chain.m_prevAngle = clampXYZ;

				chainRot = quat_cast(chainRotM);
			}

			auto ikRot = chainRot * inverse(chainNode->AnimateRotate());
			chainNode->SetIKRotate(ikRot);

			chainNode->UpdateLocalTransform();
			chainNode->UpdateGlobalTransform();
		}
	}

	void MMDIkSolver::SolvePlane(uint32_t iteration, size_t chainIdx, SolveAxis solveAxis)
	{
		int RotateAxisIndex = 0; // X axis
		auto RotateAxis = glm::vec3(1, 0, 0);
		switch (solveAxis)
		{
		case SolveAxis::X:
			RotateAxisIndex = 0; // X axis
			RotateAxis = glm::vec3(1, 0, 0);
			break;
		case SolveAxis::Y:
			RotateAxisIndex = 1; // Y axis
			RotateAxis = glm::vec3(0, 1, 0);
			break;
		case SolveAxis::Z:
			RotateAxisIndex = 2; // Z axis
			RotateAxis = glm::vec3(0, 0, 1);
			break;
		default:
			break;
		}

		auto& chain = m_chains[chainIdx];
		auto ikPos = glm::vec3(m_ikNode->GetGlobalTransform()[3]);

		auto targetPos = glm::vec3(m_ikTarget->GetGlobalTransform()[3]);

		auto invChain = inverse(chain.m_node->GetGlobalTransform());

		auto chainIkPos = glm::vec3(invChain * glm::vec4(ikPos, 1));
		auto chainTargetPos = glm::vec3(invChain * glm::vec4(targetPos, 1));

		auto chainIkVec = normalize(chainIkPos);
		auto chainTargetVec = normalize(chainTargetPos);

		auto dot = glm::dot(chainTargetVec, chainIkVec);
		dot = glm::clamp(dot, -1.0f, 1.0f);

		float angle = std::acos(dot);

		angle = glm::clamp(angle, -m_limitAngle, m_limitAngle);

		auto rot1 = rotate(glm::quat(1, 0, 0, 0), angle, RotateAxis);
		auto targetVec1 = rot1 * chainTargetVec;
		auto dot1 = glm::dot(targetVec1, chainIkVec);

		auto rot2 = rotate(glm::quat(1, 0, 0, 0), -angle, RotateAxis);
		auto targetVec2 = rot2 * chainTargetVec;
		auto dot2 = glm::dot(targetVec2, chainIkVec);

		auto newAngle = chain.m_planeModeAngle;
		if (dot1 > dot2)
		{
			newAngle += angle;
		}
		else
		{
			newAngle -= angle;
		}
		if (iteration == 0)
		{
			if (newAngle < chain.m_limitMin[RotateAxisIndex] || newAngle > chain.m_limitMax[RotateAxisIndex])
			{
				if (-newAngle > chain.m_limitMin[RotateAxisIndex] && -newAngle < chain.m_limitMax[RotateAxisIndex])
				{
					newAngle *= -1;
				}
				else
				{
					auto halfRad = (chain.m_limitMin[RotateAxisIndex] + chain.m_limitMax[RotateAxisIndex]) * 0.5f;
					if (glm::abs(halfRad - newAngle) > glm::abs(halfRad + newAngle))
					{
						newAngle *= -1;
					}
				}
			}
		}

		newAngle = glm::clamp(newAngle, chain.m_limitMin[RotateAxisIndex], chain.m_limitMax[RotateAxisIndex]);
		chain.m_planeModeAngle = newAngle;

		auto ikRotM = rotate(glm::quat(1, 0, 0, 0), newAngle, RotateAxis) * inverse(chain.m_node->AnimateRotate());
		chain.m_node->SetIKRotate(ikRotM);

		chain.m_node->UpdateLocalTransform();
		chain.m_node->UpdateGlobalTransform();
	}
}

