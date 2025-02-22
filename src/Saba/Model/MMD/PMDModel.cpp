//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "PMDModel.h"
#include "PMDFile.h"
#include "MMDPhysics.h"

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/Log.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <algorithm>

namespace saba
{
	namespace
	{
		std::string ResolveToonTexturePath(
			const std::string& mmdDataDir,
			const std::string& mmdLoadDir,
			const std::string& texName
		)
		{
			std::string filepath = PathUtil::Combine(mmdLoadDir, texName);

			File file;
			if (file.Open(filepath))
			{
				return filepath;
			}

			filepath = PathUtil::Combine(mmdDataDir, texName);
			if (file.Open(filepath))
			{
				return filepath;
			}

			SABA_WARN("Toon Texture File Not Found. [{}]", texName);
			return "";
		}
	}

	struct PMDModel::MorphVertex
	{
		uint32_t	m_index;
		glm::vec3	m_position;

		explicit  MorphVertex(const uint32_t index = 0, const glm::vec3& position = glm::vec3{0})
			: m_index(index)
			, m_position(position){}
	};

	void PMDModel::InitializeAnimation()
	{
		ClearBaseAnimation();

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->SetAnimationTranslate(glm::vec3(0));
			node->SetAnimationRotate(glm::quat(1, 0, 0, 0));
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->UpdateLocalTransform();
		}

		for (const auto& morph : *m_morphMan.GetMorphs())
		{
			morph->SetWeight(0);
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			if (node->GetParent() == nullptr)
			{
				node->UpdateGlobalTransform();
			}
		}

		for (const auto& solver : *m_ikSolverMan.GetIKSolvers())
		{
			solver->Enable(true);
			solver->Solve();
		}

		ResetPhysics();
	}

	void PMDModel::BeginAnimation()
	{
		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->BeginUpdateTransform();
		}
	}

	void PMDModel::EndAnimation()
	{
		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->EndUpdateTransform();
		}
	}

	void PMDModel::UpdateMorphAnimation()
	{
	}

	void PMDModel::UpdateNodeAnimation(const bool afterPhysicsAnim)
	{
		if (afterPhysicsAnim)
		{
			return;
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->UpdateLocalTransform();
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			if (node->GetParent() == nullptr)
			{
				node->UpdateGlobalTransform();
			}
		}

		for (const auto& solver : *m_ikSolverMan.GetIKSolvers())
		{
			solver->Solve();
		}
	}

	void PMDModel::ResetPhysics()
	{
		MMDPhysicsManager* physicsMan = GetPhysicsManager();
		const auto physics = physicsMan->GetMMDPhysics();

		if (physics == nullptr)
		{
			return;
		}

		const auto rigidbodys = physicsMan->GetRigidBodys();
		for (const auto& rb : *rigidbodys)
		{
			rb->SetActivation(false);
			rb->ResetTransform();
		}

		physics->Update(1.0f / 60.0f);

		for (const auto& rb : *rigidbodys)
		{
			rb->ReflectGlobalTransform();
		}
		for (const auto& rb : *rigidbodys)
		{
			rb->CalcLocalTransform();
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			if (node->GetParent() == nullptr)
			{
				node->UpdateGlobalTransform();
			}
		}

		for (const auto& rb : *rigidbodys)
		{
			rb->Reset(physics);
		}
	}

	void PMDModel::UpdatePhysicsAnimation(const float elapsed)
	{
		MMDPhysicsManager* physicsMan = GetPhysicsManager();
		const auto physics = physicsMan->GetMMDPhysics();

		if (physics == nullptr)
		{
			return;
		}

		const auto rigidbodys = physicsMan->GetRigidBodys();
		for (const auto& rb : *rigidbodys)
		{
			rb->SetActivation(true);
		}

		physics->Update(elapsed);

		for (const auto& rb : *rigidbodys)
		{
			rb->ReflectGlobalTransform();
		}
		for (const auto& rb : *rigidbodys)
		{
			rb->CalcLocalTransform();
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			if (node->GetParent() == nullptr)
			{
				node->UpdateGlobalTransform();
			}
		}
	}

	void PMDModel::Update()
	{
		const auto* bone = &m_bones[0];
		const auto* boneWeight = &m_boneWeights[0];
		auto* updatePosition = &m_updatePositions[0];
		auto* updateNormal = &m_updateNormals[0];

		 // 顶点复制
		std::copy(m_positions.begin(), m_positions.end(), m_updatePositions.begin());
		std::copy(m_normals.begin(), m_normals.end(), m_updateNormals.begin());

		// Morph 处理
		if (m_baseMorph.m_vertices.empty())
		{
			for (const auto& morph : *m_morphMan.GetMorphs())
			{
				const float weight = morph->GetWeight();
				if (weight == 0.0f)
				{
					continue;
				}
				for (const auto& [m_index, m_position] : morph->m_vertices)
				{
					updatePosition[m_index] += m_position * weight;
				}
			}
		}
		else
		{
			for (const auto& [m_index, m_position] : m_baseMorph.m_vertices)
			{
				updatePosition[m_index] = m_position;
			}
			for (const auto& morph : *m_morphMan.GetMorphs())
			{
				const float weight = morph->GetWeight();
				if (weight == 0.0f)
				{
					continue;
				}
				for (const auto& [m_index, m_position] : morph->m_vertices)
				{
					const auto& [m_base_index, m_base_position] = m_baseMorph.m_vertices[m_index];
					updatePosition[m_base_index] += m_position * weight;
				}
			}
		}

		 // スキンメッシュに使用する変形マトリクスを事前計算
		const auto& nodes = *m_nodeMan.GetNodes();
		for (size_t i = 0; i < nodes.size(); i++)
		{
			m_transforms[i] = nodes[i]->GetGlobalTransform() * nodes[i]->GetInverseInitTransform();
		}

		for (size_t i = 0; i < m_positions.size(); i++)
		{
			const auto w0 = boneWeight->x;
			const auto w1 = boneWeight->y;
			const auto& m0 = m_transforms[bone->x];
			const auto& m1 = m_transforms[bone->y];

			auto m = m0 * w0 + m1 * w1;
			*updatePosition = glm::vec3(m * glm::vec4(*updatePosition, 1));
			*updateNormal = normalize(glm::mat3(m) * *updateNormal);

			bone++;
			boneWeight++;
			updatePosition++;
			updateNormal++;
		}
	}

	bool PMDModel::Load(const std::string& filepath, const std::string& mmdDataDir)
	{
		Destroy();

		PMDFile pmd;
		if (!ReadPMDFile(&pmd, filepath.c_str()))
		{
			return false;
		}

		std::string dirPath = PathUtil::GetDirectoryName(filepath);

		size_t vertexCount = pmd.m_vertices.size();
		m_positions.reserve(vertexCount);
		m_normals.reserve(vertexCount);
		m_uvs.reserve(vertexCount);
		m_bones.reserve(vertexCount);
		m_boneWeights.reserve(vertexCount);
		m_bboxMax = glm::vec3(-std::numeric_limits<float>::max());
		m_bboxMin = glm::vec3(std::numeric_limits<float>::max());
		for (const auto& [m_position, m_normal, m_uv, m_bone, m_boneWeight, m_edge] : pmd.m_vertices)
		{
			glm::vec3 pos = m_position * glm::vec3(1, 1, -1);
			glm::vec3 nor = m_normal * glm::vec3(1, 1, -1);
			auto uv = glm::vec2(m_uv.x, 1.0f - m_uv.y);
			m_positions.push_back(pos);
			m_normals.push_back(nor);
			m_uvs.push_back(uv);
			m_bones.emplace_back(m_bone[0], m_bone[1]);
			float boneWeight = static_cast<float>(m_boneWeight) / 100.0f;
			m_boneWeights.emplace_back(boneWeight, 1.0f - boneWeight);

			m_bboxMax = max(m_bboxMax, pos);
			m_bboxMin = min(m_bboxMin, pos);
		}
		m_updatePositions.resize(m_positions.size());
		m_updateNormals.resize(m_normals.size());

		m_indices.reserve(pmd.m_faces.size() * 3);
		for (const auto& [m_vertices] : pmd.m_faces)
		{
			for (int i = 0; i < 3; i++)
			{
				auto vi = m_vertices[3 - i - 1];
				m_indices.push_back(vi);
			}
		}

		std::vector<std::string> toonTextures;
		toonTextures.reserve(pmd.m_toonTextureNames.size());
		for (const auto& toonTexName : pmd.m_toonTextureNames)
		{
			toonTextures.emplace_back(
				ResolveToonTexturePath(
				mmdDataDir,
				dirPath,
				toonTexName.ToUtf8String()
				));
		}

		// Materialをコピー
		m_materials.reserve(pmd.m_materials.size());
		m_subMeshes.reserve(pmd.m_materials.size());
		uint32_t beginIndex = 0;
		for (const auto& [m_diffuse, m_alpha, m_specularPower, m_specular, m_ambient, m_toonIndex, m_edgeFlag, m_faceVertexCount, m_textureName] : pmd.m_materials)
		{
			MMDMaterial mat;
			mat.m_diffuse = m_diffuse;
			mat.m_alpha = m_alpha;
			mat.m_specularPower = m_specularPower;
			mat.m_specular = m_specular;
			mat.m_ambient = m_ambient;
			mat.m_edgeFlag = m_edgeFlag;
			mat.m_edgeSize = m_edgeFlag == 0 ? 0.0f : 1.0f;
			mat.m_spTextureMode = MMDMaterial::SphereTextureMode::None;
			mat.m_bothFace = false;

			std::string orgTexName = m_textureName.ToUtf8String();
			std::string texName;
			std::string spTexName;
			if (auto asterPos = orgTexName.find_first_of('*'); asterPos == std::string::npos)
			{
				if (std::string ext = PathUtil::GetExt(orgTexName); ext == "sph")
				{
					spTexName = orgTexName;
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Mul;
				}
				else if (ext == "spa")
				{
					spTexName = orgTexName;
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Add;
				}
				else
				{
					texName = orgTexName;
				}
			}
			else
			{
				texName = orgTexName.substr(0, asterPos);
				spTexName = orgTexName.substr(asterPos + 1);
				if (std::string ext = PathUtil::GetExt(spTexName); ext == "sph")
				{
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Mul;
				}
				else if (ext == "spa")
				{
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Add;
				}
			}

			if (!texName.empty())
			{
				std::string texPath = PathUtil::Combine(dirPath, texName);
				mat.m_texture = PathUtil::Normalize(texPath);
			}

			if (!spTexName.empty())
			{
				std::string spTexPath = PathUtil::Combine(dirPath, spTexName);
				mat.m_spTexture = PathUtil::Normalize(spTexPath);
			}

			if (m_toonIndex != 255)
			{
				mat.m_toonTexture = toonTextures[m_toonIndex];
			}

			m_materials.emplace_back(mat);
			m_subMeshes.emplace_back(static_cast<int>(beginIndex),
				static_cast<int>(m_faceVertexCount),
				static_cast<int>(m_materials.size() - 1));

			beginIndex = beginIndex + m_faceVertexCount;
		}

		for (const auto& [m_morphName, m_morphType, m_vertices, m_englishShapeNameExt] : pmd.m_morphs)
		{
			PMDMorph* morph;
			if (m_morphType == saba::PMDMorph::Base)
			{
				morph = &m_baseMorph;
			}
			else
			{
				morph = m_morphMan.AddMorph();
				morph->SetName(m_morphName.ToUtf8String());
			}
			morph->SetWeight(0.0f);
			morph->m_vertices.reserve(m_vertices.size());
			for (const auto [m_vertexIndex, m_position] : m_vertices)
			{
				morph->m_vertices.emplace_back(m_vertexIndex, m_position * glm::vec3(1, 1, -1));
			}
		}

		// Nodeの作成
		m_nodeMan.GetNodes()->reserve(pmd.m_bones.size());
		for (const auto& bone : pmd.m_bones)
		{
			auto* node = m_nodeMan.AddNode();
			node->SetName(bone.m_boneName.ToUtf8String());
		}
		for (size_t i = 0; i < pmd.m_bones.size(); i++)
		{
			const auto& [m_boneName, m_parent, m_tail, m_boneType, m_ikParent, m_position, m_englishBoneNameExt] = pmd.m_bones[i];
			auto* node = m_nodeMan.GetNode(i);
			if (m_parent != 0xFFFF)
			{
				const auto& parentBone = pmd.m_bones[m_parent];
				auto* parentNode = m_nodeMan.GetNode(m_parent);
				parentNode->AddChild(node);
				glm::vec3 localPos = m_position - parentBone.m_position;
				localPos.z *= -1;

				node->SetTranslate(localPos);
			}
			else
			{
				glm::vec3 localPos = m_position;
				localPos.z *= -1;

				node->SetTranslate(localPos);
			}
			glm::mat4 init = translate(
				glm::mat4(1),
				m_position * glm::vec3(1, 1, -1)
			);
			node->SetGlobalTransform(init);
			node->CalculateInverseInitTransform();
			node->SaveInitialTRS();
		}
		m_transforms.resize(m_nodeMan.GetNodeCount());

		// IKを作成
		m_ikSolverMan.GetIKSolvers()->reserve(pmd.m_iks.size());
		for (const auto& [m_ikNode, m_ikTarget, m_numChain, m_numIteration, m_rotateLimit, m_chanins] : pmd.m_iks)
		{
			auto solver = m_ikSolverMan.AddIKSolver();
			auto* ikNode = m_nodeMan.GetNode(m_ikNode);
			solver->SetIKNode(ikNode);

			auto* targetNode = m_nodeMan.GetNode(m_ikTarget);
			solver->SetTargetNode(targetNode);

			for (const auto& chain : m_chanins)
			{
				auto* chainNode = m_nodeMan.GetNode(chain);
				auto findPos = chainNode->GetName().find(u8"ひざ");
				bool isKnee = false;
				if (findPos != std::string::npos)
				{
					isKnee = true;
				}
				solver->AddIKChain(chainNode, isKnee);
				chainNode->EnableIK(true);
			}

			solver->SetIterateCount(m_numIteration);
			solver->SetLimitAngle(m_rotateLimit * 4.0f);
		}

		if (!m_physicsMan.Create())
		{
			SABA_ERROR("Create Physics Fail.");
			return false;
		}

		for (const auto& pmdRB : pmd.m_rigidBodies)
		{
			auto rb = m_physicsMan.AddRigidBody();
			MMDNode* node = nullptr;
			if (pmdRB.m_boneIndex != 0xFFFF)
			{
				node = m_nodeMan.GetMMDNode(pmdRB.m_boneIndex);
			}
			if (!rb->Create(pmdRB, this, node))
			{
				SABA_ERROR("Create Rigid Body Fail.\n");
				return false;
			}
			m_physicsMan.GetMMDPhysics()->AddRigidBody(rb);
		}

		for (const auto& pmdJoint : pmd.m_joints)
		{
			if (pmdJoint.m_rigidBodyA != -1 &&
				pmdJoint.m_rigidBodyB != -1 &&
				pmdJoint.m_rigidBodyA != pmdJoint.m_rigidBodyB)
			{
				auto joint = m_physicsMan.AddJoint();
				auto rigidBodys = m_physicsMan.GetRigidBodys();
				bool ret = joint->CreateJoint(
					pmdJoint,
					(*rigidBodys)[pmdJoint.m_rigidBodyA].get(),
					(*rigidBodys)[pmdJoint.m_rigidBodyB].get()
				);
				if (!ret)
				{
					SABA_ERROR("Create Joint Fail.\n");
					return false;
				}
				m_physicsMan.GetMMDPhysics()->AddJoint(joint);
			}
			else
			{
				SABA_WARN("Illegal Joint [{}]", pmdJoint.m_jointName.ToUtf8String());
			}
		}

		ResetPhysics();

		return true;
	}

	void PMDModel::Destroy()
	{
		m_materials.clear();
		m_subMeshes.clear();

		m_positions.clear();
		m_normals.clear();
		m_uvs.clear();
		m_bones.clear();
		m_boneWeights.clear();

		m_indices.clear();

		m_nodeMan.GetNodes()->clear();
	}

}
