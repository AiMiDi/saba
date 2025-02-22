//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "MMDModel.h"
#include "MMDPhysics.h"
#include "VPDFile.h"
#include "VMDAnimation.h"

#include <glm/gtc/matrix_transform.hpp>

#include <Saba/Base/Log.h>

namespace saba
{
	MMDPhysicsManager::MMDPhysicsManager() = default;

	MMDPhysicsManager::~MMDPhysicsManager()
	{
		for (auto& joint : m_joints)
		{
			m_mmdPhysics->RemoveJoint(joint.get());
		}
		m_joints.clear();

		for (auto& rb : m_rigidBodys)
		{
			m_mmdPhysics->RemoveRigidBody(rb.get());
		}
		m_rigidBodys.clear();

		m_mmdPhysics.reset();
	}

	bool MMDPhysicsManager::Create()
	{
		m_mmdPhysics = std::make_unique<MMDPhysics>();
		return m_mmdPhysics->Create();
	}

	MMDPhysics* MMDPhysicsManager::GetMMDPhysics() const
	{
		return m_mmdPhysics.get();
	}

	MMDRigidBody* MMDPhysicsManager::AddRigidBody()
	{
		SABA_ASSERT(m_mmdPhysics != nullptr);
		return m_rigidBodys.emplace_back(std::make_unique<MMDRigidBody>()).get();
	}

	MMDJoint* MMDPhysicsManager::AddJoint()
	{
		SABA_ASSERT(m_mmdPhysics != nullptr);
		return m_joints.emplace_back(std::make_unique<MMDJoint>()).get();
	}

	void MMDModel::SaveBaseAnimation()
	{
		const auto nodeMan = GetNodeManager();
		for (size_t i = 0; i < nodeMan->GetNodeCount(); i++)
		{
			const auto node = nodeMan->GetMMDNode(i);
			node->SaveBaseAnimation();
		}

		const auto morphMan = GetMorphManager();
		for (size_t i = 0; i < morphMan->GetMorphCount(); i++)
		{
			const auto morph = morphMan->GetMorph(i);
			morph->SaveBaseAnimation();
		}

		const auto ikMan = GetIKManager();
		for (size_t i = 0; i < ikMan->GetIKSolverCount(); i++)
		{
			const auto ikSolver = ikMan->GetMMDIKSolver(i);
			ikSolver->SaveBaseAnimation();
		}
	}

	void MMDModel::LoadBaseAnimation()
	{
		const auto nodeMan = GetNodeManager();
		for (size_t i = 0; i < nodeMan->GetNodeCount(); i++)
		{
			const auto node = nodeMan->GetMMDNode(i);
			node->LoadBaseAnimation();
		}

		const auto morphMan = GetMorphManager();
		for (size_t i = 0; i < morphMan->GetMorphCount(); i++)
		{
			const auto morph = morphMan->GetMorph(i);
			morph->LoadBaseAnimation();
		}

		const auto ikMan = GetIKManager();
		for (size_t i = 0; i < ikMan->GetIKSolverCount(); i++)
		{
			const auto ikSolver = ikMan->GetMMDIKSolver(i);
			ikSolver->LoadBaseAnimation();
		}
	}

	void MMDModel::ClearBaseAnimation()
	{
		const auto nodeMan = GetNodeManager();
		for (size_t i = 0; i < nodeMan->GetNodeCount(); i++)
		{
			const auto node = nodeMan->GetMMDNode(i);
			node->ClearBaseAnimation();
		}

		const auto morphMan = GetMorphManager();
		for (size_t i = 0; i < morphMan->GetMorphCount(); i++)
		{
			const auto morph = morphMan->GetMorph(i);
			morph->ClearBaseAnimation();
		}

		const auto ikMan = GetIKManager();
		for (size_t i = 0; i < ikMan->GetIKSolverCount(); i++)
		{
			const auto ikSolver = ikMan->GetMMDIKSolver(i);
			ikSolver->ClearBaseAnimation();
		}
	}

	namespace
	{
		glm::mat3 InvZ(const glm::mat3& m)
		{
			const glm::mat3 invZ = scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));
			return invZ * m * invZ;
		}
		glm::quat InvZ(const glm::quat& q)
		{
			const auto rot0 = mat3_cast(q);
			const auto rot1 = InvZ(rot0);
			return quat_cast(rot1);
		}
	}

	void MMDModel::UpdateAllAnimation(const VMDAnimation * vmdAnim, const float vmdFrame, const float physicsElapsed)
	{
		if (vmdAnim != nullptr)
		{
			vmdAnim->Evaluate(vmdFrame);
		}

		UpdateMorphAnimation();

		UpdateNodeAnimation(false);

		UpdatePhysicsAnimation(physicsElapsed);

		UpdateNodeAnimation(true);
	}

	void MMDModel::LoadPose(const VPDFile & vpd, const int frameCount)
	{
		struct Pose
		{
			MMDNode*	m_node;
			glm::vec3	m_beginTranslate;
			glm::vec3	m_endTranslate;
			glm::quat	m_beginRotate;
			glm::quat	m_endRotate;

			explicit Pose(
				MMDNode* node = nullptr,
				const glm::vec3& beginTranslate = glm::vec3(0.0f),
				const glm::vec3& endTranslate = glm::vec3(0.0f),
				const glm::quat& beginRotate = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
				const glm::quat& endRotate = glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
			)
				: m_node(node),
				  m_beginTranslate(beginTranslate),
				  m_endTranslate(endTranslate),
				  m_beginRotate(beginRotate),
				  m_endRotate(endRotate)
			{
			}
		};
		std::vector<Pose> poses;
		for (const auto& bone : vpd.m_bones)
		{
			if (const auto nodeIdx = GetNodeManager()->FindNodeIndex(bone.m_boneName); MMDNodeManager::NPos != nodeIdx)
			{
				const auto node = GetNodeManager()->GetMMDNode(bone.m_boneName);
				poses.emplace_back(node,
					node->GetAnimationTranslate(),
					bone.m_translate * glm::vec3(1, 1, -1),
					node->GetAnimationRotate(),
					InvZ(bone.m_quaternion));
			}
		}

		struct Morph
		{
			MMDMorph*	m_morph;
			float		m_beginWeight;
			float		m_endWeight;

			explicit Morph(
				MMDMorph* morph = nullptr,
				const float beginWeight = 0.0f,
				const float endWeight = 0.0f
			)
				: m_morph(morph),
				  m_beginWeight(beginWeight),
				  m_endWeight(endWeight)
			{
			}
		};
		std::vector<Morph> morphs;
		for (const auto& [m_morphName, m_weight] : vpd.m_morphs)
		{
			if (const auto morphIdx = GetMorphManager()->FindMorphIndex(m_morphName); MMDMorphManager::NPos != morphIdx)
			{
				const auto morph = GetMorphManager()->GetMorph(m_morphName);
				morphs.emplace_back(morph, morph->GetWeight(), m_weight);
			}
		}

		// Apply Physics
		for (int i = 0; i < frameCount; i++)
		{
			BeginAnimation();

			// evaluate
			float w = static_cast<float>(1 + i) / static_cast<float>(frameCount);
			for (auto& pose : poses)
			{
				auto t = mix(pose.m_beginTranslate, pose.m_endTranslate, w);
				auto q = slerp(pose.m_beginRotate, pose.m_endRotate, w);
				pose.m_node->SetAnimationTranslate(t);
				pose.m_node->SetAnimationRotate(q);
			}

			for (auto& morph : morphs)
			{
				auto weight = glm::mix(morph.m_beginWeight, morph.m_endWeight, w);
				morph.m_morph->SetWeight(weight);
			}

			UpdateMorphAnimation();
			UpdateNodeAnimation(false);
			UpdatePhysicsAnimation(1.0f / 30.0f);
			UpdateNodeAnimation(true);

			EndAnimation();
		}

		//SaveBaseAnimation();
	}
}
