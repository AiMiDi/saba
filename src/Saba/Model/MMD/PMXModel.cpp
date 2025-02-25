//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
#define GLM_ENABLE_EXPERIMENTAL

#include "PMXModel.h"

#include "PMXFile.h"
#include "MMDPhysics.h"

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/Log.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <limits>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>
#include <stack>

namespace saba
{
	struct PMXModel::PositionMorph
	{
		explicit PositionMorph(uint32_t index = 0, const glm::vec3& position = glm::vec3{})
			:m_index(index), m_position(position) {}

		uint32_t	m_index;
		glm::vec3	m_position;
	};

	struct PMXModel::PositionMorphData
	{
		std::vector<PositionMorph>	m_morphVertices;

		PositionMorphData() = default;
		~PositionMorphData() = default;
		PositionMorphData(const PositionMorphData&) = delete;
		PositionMorphData& operator=(const PositionMorphData&) = delete;
		PositionMorphData(PositionMorphData&& other) noexcept: m_morphVertices(std::move(other.m_morphVertices)){}
		PositionMorphData& operator=(PositionMorphData&& other) noexcept
		{
			if (this != &other)
			{
				m_morphVertices = std::move(other.m_morphVertices);
			}
			return *this;
		}
	};

	struct PMXModel::UVMorph
	{
		explicit UVMorph(uint32_t index = 0, const glm::vec4& uv = glm::vec4{})
			: m_index(index), m_uv(uv) {}

		uint32_t	m_index;
		glm::vec4	m_uv;
	};

	struct PMXModel::UVMorphData
	{
		std::vector<UVMorph>	m_morphUVs;

		UVMorphData() = default;
		~UVMorphData() = default;
		UVMorphData(const UVMorphData&) = delete;
		UVMorphData& operator=(const UVMorphData&) = delete;
		UVMorphData(UVMorphData&& other) noexcept: m_morphUVs(std::move(other.m_morphUVs)){}
		UVMorphData& operator=(UVMorphData&& other) noexcept
		{
			if (this != &other)
			{
				m_morphUVs = std::move(other.m_morphUVs);
			}
			return *this;
		}
	};

	struct PMXModel::MaterialFactor
	{
		explicit MaterialFactor(
			const glm::vec3& diffuse = glm::vec3(1),
			const float alpha = 1,
			const glm::vec3& specular = glm::vec3(1),
			const float specularPower = 1,
			const glm::vec3& ambient = glm::vec3(1),
			const glm::vec4& edgeColor = glm::vec4(1),
			const float edgeSize = 1,
			const glm::vec4& textureFactor = glm::vec4(1),
			const glm::vec4& spTextureFactor = glm::vec4(1),
			const glm::vec4& toonTextureFactor = glm::vec4(1)
			): m_diffuse(diffuse),
			m_alpha(alpha),
			m_specular(specular),
			m_specularPower(specularPower),
			m_ambient(ambient),
			m_edgeColor(edgeColor),
			m_edgeSize(edgeSize),
			m_textureFactor(textureFactor),
			m_spTextureFactor(spTextureFactor),
			m_toonTextureFactor(toonTextureFactor)
		{}

		explicit MaterialFactor(const PMXFileMorph::MaterialMorph& pmxMat):
			m_diffuse(pmxMat.m_diffuse),
			m_alpha(pmxMat.m_diffuse.a),
			m_specular(pmxMat.m_specular),
			m_specularPower(pmxMat.m_specularPower),
			m_ambient(pmxMat.m_ambient),
			m_edgeColor(pmxMat.m_edgeColor),
			m_edgeSize(pmxMat.m_edgeSize),
			m_textureFactor(pmxMat.m_textureFactor),
			m_spTextureFactor(pmxMat.m_sphereTextureFactor),
			m_toonTextureFactor(pmxMat.m_toonTextureFactor)
		{}

		void Mul(const MaterialFactor& val, float weight)
		{
			m_diffuse = mix(m_diffuse, m_diffuse * val.m_diffuse, weight);
			m_alpha = glm::mix(m_alpha, m_alpha * val.m_alpha, weight);
			m_specular = mix(m_specular, m_specular * val.m_specular, weight);
			m_specularPower = glm::mix(m_specularPower, m_specularPower * val.m_specularPower, weight);
			m_ambient = mix(m_ambient, m_ambient * val.m_ambient, weight);
			m_edgeColor = mix(m_edgeColor, m_edgeColor * val.m_edgeColor, weight);
			m_edgeSize = glm::mix(m_edgeSize, m_edgeSize * val.m_edgeSize, weight);
			m_textureFactor = mix(m_textureFactor, m_textureFactor * val.m_textureFactor, weight);
			m_spTextureFactor = mix(m_spTextureFactor, m_spTextureFactor * val.m_spTextureFactor, weight);
			m_toonTextureFactor = mix(m_toonTextureFactor, m_toonTextureFactor * val.m_toonTextureFactor, weight);
		}

		void Add(const MaterialFactor& val, float weight)
		{
			m_diffuse += val.m_diffuse * weight;
			m_alpha += val.m_alpha * weight;
			m_specular += val.m_specular * weight;
			m_specularPower += val.m_specularPower * weight;
			m_ambient += val.m_ambient * weight;
			m_edgeColor += val.m_edgeColor * weight;
			m_edgeSize += val.m_edgeSize * weight;
			m_textureFactor += val.m_textureFactor * weight;
			m_spTextureFactor += val.m_spTextureFactor * weight;
			m_toonTextureFactor += val.m_toonTextureFactor * weight;
		}

		glm::vec3	m_diffuse;
		float		m_alpha;
		glm::vec3	m_specular;
		float		m_specularPower;
		glm::vec3	m_ambient;
		glm::vec4	m_edgeColor;
		float		m_edgeSize;
		glm::vec4	m_textureFactor;
		glm::vec4	m_spTextureFactor;
		glm::vec4	m_toonTextureFactor;
	};

	struct PMXModel::MaterialMorphData
	{
		std::vector<PMXFileMorph::MaterialMorph>	m_materialMorphs;

		MaterialMorphData() = default;
		~MaterialMorphData() = default;
		MaterialMorphData(const MaterialMorphData&) = delete;
		MaterialMorphData& operator=(const MaterialMorphData&) = delete;
		MaterialMorphData(MaterialMorphData&& other) noexcept
			: m_materialMorphs(std::move(other.m_materialMorphs)){}
		MaterialMorphData& operator=(MaterialMorphData&& other) noexcept
		{
			if (this != &other)
			{
				m_materialMorphs = std::move(other.m_materialMorphs);
			}
			return *this;
		}
	};

	struct PMXModel::BoneMorphElement
	{
		explicit BoneMorphElement(MMDNode* node = nullptr, const glm::vec3& position = glm::vec3{}, const glm::quat& rotate = glm::quat{})
			: m_node(node), m_position(position), m_rotate(rotate) {}

		MMDNode*	m_node;
		glm::vec3	m_position;
		glm::quat	m_rotate;
	};

	struct PMXModel::BoneMorphData
	{
		std::vector<BoneMorphElement>	m_boneMorphs;

		BoneMorphData() = default;
		~BoneMorphData() = default;
		BoneMorphData(const BoneMorphData&) = delete;
		BoneMorphData& operator=(const BoneMorphData&) = delete;
		BoneMorphData(BoneMorphData&& other) noexcept : m_boneMorphs(std::move(other.m_boneMorphs)){}
		BoneMorphData& operator=(BoneMorphData&& other) noexcept
		{
			if (this != &other)
			{
				m_boneMorphs = std::move(other.m_boneMorphs);
			}
			return *this;
		}
	};

	struct PMXModel::GroupMorphData
	{
		std::vector<PMXFileMorph::GroupMorph>		m_groupMorphs;

		GroupMorphData() = default;
		~GroupMorphData() = default;
		GroupMorphData(const GroupMorphData&) = delete;
		GroupMorphData& operator=(const GroupMorphData&) = delete;
		GroupMorphData(GroupMorphData&& other) noexcept: m_groupMorphs(std::move(other.m_groupMorphs)){}
		GroupMorphData& operator=(GroupMorphData&& other) noexcept
		{
			if (this != &other)
			{
				m_groupMorphs = std::move(other.m_groupMorphs);
			}
			return *this;
		}
	};

	enum class PMXModel::MorphType
	{
		None,
		Position,
		UV,
		Material,
		Bone,
		Group,
	};

	class PMXModel::PMXMorph : public MMDMorph
	{
	public:
		MorphType	m_morphType{MorphType::None};
		size_t		m_dataIndex{};
	};

	struct PMXModel::UpdateRange
	{
		explicit UpdateRange(size_t vertexOffset = 0, size_t vertexCount = 0)
			: m_vertexOffset(vertexOffset), m_vertexCount(vertexCount) {}

		size_t	m_vertexOffset;
		size_t	m_vertexCount;
	};

	PMXModel::PMXModel()
		: m_indexCount(0), m_indexElementSize(0), m_bboxMin(), m_bboxMax(), m_parallelUpdateCount(0)
	{
	}

	PMXModel::~PMXModel()
	{
		Destroy();
	}

	void PMXModel::InitializeAnimation()
	{
		ClearBaseAnimation();

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->SetAnimationTranslate(glm::vec3(0));
			node->SetAnimationRotate(glm::quat(1, 0, 0, 0));
		}

		BeginAnimation();

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->UpdateLocalTransform();
		}

		for (const auto& morph : *m_morphMan.GetMorphs())
		{
			morph->SetWeight(0);
		}

		for (const auto& ikSolver : *m_ikSolverMan.GetIKSolvers())
		{
			ikSolver->Enable(true);
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			if (node->GetParent() == nullptr)
			{
				node->UpdateGlobalTransform();
			}
		}

		for (const auto pmxNode : m_sortedNodes)
		{
			if (pmxNode->GetAppendNode() != nullptr)
			{
				pmxNode->UpdateAppendTransform();
				pmxNode->UpdateGlobalTransform();
			}
			if (pmxNode->GetIKSolver() != nullptr)
			{
				const auto ikSolver = pmxNode->GetIKSolver();
				ikSolver->Solve();
				pmxNode->UpdateGlobalTransform();
			}
		}

		for (const auto& node : *m_nodeMan.GetNodes())
		{
			if (node->GetParent() == nullptr)
			{
				node->UpdateGlobalTransform();
			}
		}

		EndAnimation();

		ResetPhysics();
	}

	void PMXModel::BeginAnimation()
	{
		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->BeginUpdateTransform();
		}
		const size_t vtxCount = m_morphPositions.size();
		for (size_t vtxIdx = 0; vtxIdx < vtxCount; vtxIdx++)
		{
			m_morphPositions[vtxIdx] = glm::vec3(0);
			m_morphUVs[vtxIdx] = glm::vec4(0);
		}
	}

	void PMXModel::EndAnimation()
	{
		for (const auto& node : *m_nodeMan.GetNodes())
		{
			node->EndUpdateTransform();
		}
	}

	void PMXModel::UpdateMorphAnimation()
	{
		// Process morphs
		BeginMorphMaterial();

		const auto& morphs = *m_morphMan.GetMorphs();
		for (const auto & morph : morphs)
		{
				Morph(morph.get(), morph->GetWeight());
		}

		EndMorphMaterial();
	}

	void PMXModel::UpdateNodeAnimation(const bool afterPhysicsAnim)
	{
		for (const auto pmxNode : m_sortedNodes)
		{
			if (pmxNode->IsDeformAfterPhysics() != afterPhysicsAnim)
			{
				continue;
			}

			pmxNode->UpdateLocalTransform();
		}

		for (const auto pmxNode : m_sortedNodes)
		{
			if (pmxNode->IsDeformAfterPhysics() != afterPhysicsAnim)
			{
				continue;
			}

			if (pmxNode->GetParent() == nullptr)
			{
				pmxNode->UpdateGlobalTransform();
			}
		}

		for (const auto pmxNode : m_sortedNodes)
		{
			if (pmxNode->IsDeformAfterPhysics() != afterPhysicsAnim)
			{
				continue;
			}

			if (pmxNode->GetAppendNode() != nullptr)
			{
				pmxNode->UpdateAppendTransform();
				pmxNode->UpdateGlobalTransform();
			}
			if (pmxNode->GetIKSolver() != nullptr)
			{
				const auto ikSolver = pmxNode->GetIKSolver();
				ikSolver->Solve();
				pmxNode->UpdateGlobalTransform();
			}
		}

		for (const auto pmxNode : m_sortedNodes)
		{
			if (pmxNode->IsDeformAfterPhysics() != afterPhysicsAnim)
			{
				continue;
			}

			if (pmxNode->GetParent() == nullptr)
			{
				pmxNode->UpdateGlobalTransform();
			}
		}
	}

	void PMXModel::ResetPhysics()
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

	void PMXModel::UpdatePhysicsAnimation(const float elapsed)
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

	void PMXModel::Update()
	{
		const auto& nodes = *m_nodeMan.GetNodes();

		// Pre-calculate deformation matrices for skinning mesh
		for (size_t i = 0; i < nodes.size(); i++)
		{
			m_transforms[i] = nodes[i]->GetGlobalTransform() * nodes[i]->GetInverseInitTransform();
		}

		if (m_parallelUpdateCount != m_updateRanges.size())
		{
			SetupParallelUpdate();
		}

		const size_t futureCount = m_parallelUpdateFutures.size();
		for (size_t i = 0; i < futureCount; i++)
		{
			if (size_t rangeIndex = i + 1; m_updateRanges[rangeIndex].m_vertexCount != 0)
			{
				m_parallelUpdateFutures[i] = std::async(
					std::launch::async,
					[this, rangeIndex] { this->Update(this->m_updateRanges[rangeIndex]); }
				);
			}
		}

		Update(m_updateRanges[0]);

		for (size_t i = 0; i < futureCount; i++)
		{
			if (const size_t rangeIndex = i + 1; m_updateRanges[rangeIndex].m_vertexCount != 0)
			{
				m_parallelUpdateFutures[i].wait();
			}
		}
	}

	void PMXModel::SetParallelUpdateHint(const uint32_t parallelCount)
	{
		m_parallelUpdateCount = parallelCount;
	}

	bool PMXModel::Load(const std::string& filepath, const std::string& mmdDataDir)
	{
		Destroy();

		PMXFile pmx;
		if (!ReadPMXFile(&pmx, filepath.c_str()))
		{
			return false;
		}

		std::string dirPath = PathUtil::GetDirectoryName(filepath);

		// Pre-allocate memory
		const size_t vertexCount = pmx.m_vertices.size();
		m_positions.resize(vertexCount);
		m_normals.resize(vertexCount);
		m_uvs.resize(vertexCount);
		m_vertexBoneInfos.resize(vertexCount);
		m_updatePositions.resize(vertexCount);
		m_updateNormals.resize(vertexCount);
		m_updateUVs.resize(vertexCount);
		m_morphPositions.resize(vertexCount);
		m_morphUVs.resize(vertexCount);

		m_bboxMax = glm::vec3(-std::numeric_limits<float>::max());
		m_bboxMin = glm::vec3(std::numeric_limits<float>::max());

		// Read vertex data
		for (size_t i = 0; i < vertexCount; i++)
		{
			const auto& vertex = pmx.m_vertices[i];
			m_positions[i] = vertex.m_position;
			m_normals[i] = vertex.m_normal;
			m_uvs[i] = vertex.m_uv;
			m_bboxMax = glm::max(m_bboxMax, vertex.m_position);
			m_bboxMin = glm::min(m_bboxMin, vertex.m_position);

			// Process vertex bone weights
			auto& vtxBoneInfo = m_vertexBoneInfos[i];
			switch (vertex.m_weightType)
			{
			case PMXVertexWeight::BDEF1:
				vtxBoneInfo.m_skinningType = SkinningType::Weight1;
				vtxBoneInfo.m_boneIndex[0] = vertex.m_boneIndices[0];
				vtxBoneInfo.m_boneWeight[0] = 1.0f;
				break;
			case PMXVertexWeight::BDEF2:
				vtxBoneInfo.m_skinningType = SkinningType::Weight2;
				vtxBoneInfo.m_boneIndex[0] = vertex.m_boneIndices[0];
				vtxBoneInfo.m_boneIndex[1] = vertex.m_boneIndices[1];
				vtxBoneInfo.m_boneWeight[0] = vertex.m_boneWeights[0];
				vtxBoneInfo.m_boneWeight[1] = 1.0f - vertex.m_boneWeights[0];
				break;
			case PMXVertexWeight::BDEF4:
				vtxBoneInfo.m_skinningType = SkinningType::Weight4;
				vtxBoneInfo.m_boneIndex[0] = vertex.m_boneIndices[0];
				vtxBoneInfo.m_boneIndex[1] = vertex.m_boneIndices[1];
				vtxBoneInfo.m_boneIndex[2] = vertex.m_boneIndices[2];
				vtxBoneInfo.m_boneIndex[3] = vertex.m_boneIndices[3];
				vtxBoneInfo.m_boneWeight[0] = vertex.m_boneWeights[0];
				vtxBoneInfo.m_boneWeight[1] = vertex.m_boneWeights[1];
				vtxBoneInfo.m_boneWeight[2] = vertex.m_boneWeights[2];
				vtxBoneInfo.m_boneWeight[3] = vertex.m_boneWeights[3];
				break;
			case PMXVertexWeight::SDEF:
				vtxBoneInfo.m_skinningType = SkinningType::SDEF;
				vtxBoneInfo.m_sdef.m_boneIndex[0] = vertex.m_boneIndices[0];
				vtxBoneInfo.m_sdef.m_boneIndex[1] = vertex.m_boneIndices[1];
				vtxBoneInfo.m_sdef.m_boneWeight = vertex.m_boneWeights[0];
				vtxBoneInfo.m_sdef.m_sdefC = vertex.m_sdefC;
				vtxBoneInfo.m_sdef.m_sdefR0 = vertex.m_sdefR0;
				vtxBoneInfo.m_sdef.m_sdefR1 = vertex.m_sdefR1;
				break;
			case PMXVertexWeight::QDEF:
				vtxBoneInfo.m_skinningType = SkinningType::Weight4;
				vtxBoneInfo.m_boneIndex[0] = vertex.m_boneIndices[0];
				vtxBoneInfo.m_boneIndex[1] = vertex.m_boneIndices[1];
				vtxBoneInfo.m_boneIndex[2] = vertex.m_boneIndices[2];
				vtxBoneInfo.m_boneIndex[3] = vertex.m_boneIndices[3];
				vtxBoneInfo.m_boneWeight[0] = vertex.m_boneWeights[0];
				vtxBoneInfo.m_boneWeight[1] = vertex.m_boneWeights[1];
				vtxBoneInfo.m_boneWeight[2] = vertex.m_boneWeights[2];
				vtxBoneInfo.m_boneWeight[3] = vertex.m_boneWeights[3];
				break;
			default:
				SABA_ERROR("PMX Load Error: Unknown vertex weight type");
				return false;
			}
		}

		// Process face data
		const size_t faceCount = pmx.m_faces.size();
		m_indexCount = faceCount * 3;
		m_indices.resize(m_indexCount);

		// Handle face data based on index size
		switch (pmx.m_header.m_vertexIndexSize)
		{
		case 1:
			{
				for (size_t i = 0; i < faceCount; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						m_indices[i * 3 + j] = static_cast<uint32_t>(pmx.m_faces[i].m_vertices[j]);
					}
				}
			}
			break;
		case 2:
			{
				for (size_t i = 0; i < faceCount; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						m_indices[i * 3 + j] = static_cast<uint32_t>(pmx.m_faces[i].m_vertices[j]);
					}
				}
			}
			break;
		case 4:
			{
				for (size_t i = 0; i < faceCount; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						m_indices[i * 3 + j] = pmx.m_faces[i].m_vertices[j];
					}
				}
			}
			break;
		default:
			SABA_ERROR("PMX Load Error: Unknown vertex index size [{}]", pmx.m_header.m_vertexIndexSize);
			return false;
		}

		m_indexElementSize = pmx.m_header.m_vertexIndexSize;

		// Load materials
		for (const auto& pmxMat : pmx.m_materials)
		{
			MMDMaterial mat;
			mat.m_diffuse = glm::vec3(pmxMat.m_diffuse.r, pmxMat.m_diffuse.g, pmxMat.m_diffuse.b);
			mat.m_alpha = pmxMat.m_diffuse.a;
			mat.m_specular = glm::vec3(pmxMat.m_specular.r, pmxMat.m_specular.g, pmxMat.m_specular.b);
			mat.m_specularPower = pmxMat.m_specularPower;
			mat.m_ambient = glm::vec3(pmxMat.m_ambient.r, pmxMat.m_ambient.g, pmxMat.m_ambient.b);
			mat.m_bothFace = !!(static_cast<uint8_t>(pmxMat.m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::BothFace));
			mat.m_edgeFlag = (static_cast<uint8_t>(pmxMat.m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::DrawEdge)) == 0 ? 0 : 1;
			mat.m_edgeSize = pmxMat.m_edgeSize;
			mat.m_edgeColor = pmxMat.m_edgeColor;
			mat.m_spTextureMode = MMDMaterial::SphereTextureMode::None;

			// Process textures
			if (pmxMat.m_textureIndex != -1)
			{
				mat.m_texture = PathUtil::Combine(dirPath, pmx.m_textures[pmxMat.m_textureIndex].m_textureName);
			}

			// Process sphere textures
			if (pmxMat.m_sphereTextureIndex != -1)
			{
				mat.m_spTexture = PathUtil::Combine(dirPath, pmx.m_textures[pmxMat.m_sphereTextureIndex].m_textureName);
				if (pmxMat.m_sphereMode == PMXSphereMode::Mul)
				{
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Mul;
				}
				else if (pmxMat.m_sphereMode == PMXSphereMode::Add)
				{
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Add;
				}
			}

			// Process Toon textures
			if (pmxMat.m_toonMode == PMXToonMode::Common)
			{
				if (pmxMat.m_toonTextureIndex != -1)
				{
					std::stringstream ss;
					ss << "toon" << std::setfill('0') << std::setw(2) << pmxMat.m_toonTextureIndex + 1 << ".bmp";
					mat.m_toonTexture = PathUtil::Combine(mmdDataDir, ss.str());
				}
			}
			else if (pmxMat.m_toonMode == PMXToonMode::Separate)
			{
				if (pmxMat.m_toonTextureIndex != -1)
				{
					mat.m_toonTexture = PathUtil::Combine(dirPath, pmx.m_textures[pmxMat.m_toonTextureIndex].m_textureName);
				}
			}

			// Set other flags
			mat.m_groundShadow = !!(static_cast<uint8_t>(pmxMat.m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::GroundShadow));
			mat.m_shadowCaster = !!(static_cast<uint8_t>(pmxMat.m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::CastSelfShadow));
			mat.m_shadowReceiver = !!(static_cast<uint8_t>(pmxMat.m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::RecieveSelfShadow));

			m_materials.emplace_back(std::move(mat));
		}

		// Create bone nodes
		m_nodeMan.GetNodes()->reserve(pmx.m_bones.size());
		for (const auto& bone : pmx.m_bones)
		{
			auto* node = m_nodeMan.AddNode();
			node->SetName(bone.m_name);
		}

		// Set bone hierarchy and transforms
		for (size_t i = 0; i < pmx.m_bones.size(); i++)
		{
			const auto& bone = pmx.m_bones[i];
			auto* node = static_cast<PMXNode*>(m_nodeMan.GetNode(i));

			// Set parent-child relationships
			if (bone.m_parentBoneIndex != -1)
			{
				auto* parentNode = m_nodeMan.GetNode(bone.m_parentBoneIndex);
				parentNode->AddChild(node);
			}

			// Set initial transforms
			node->SetTranslate(bone.m_position);
			glm::mat4 init = translate(glm::mat4(1), bone.m_position);
			node->SetGlobalTransform(init);
			node->CalculateInverseInitTransform();
			node->SaveInitialTRS();

			// Set other properties
			node->SetDeformDepth(bone.m_deformDepth);
			node->EnableDeformAfterPhysics(!!(static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::DeformAfterPhysics)));

			// Process append transforms
			if (static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendRotate) ||
				static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendTranslate))
			{
				if (bone.m_appendBoneIndex != -1)
				{
					auto* appendNode = static_cast<PMXNode*>(m_nodeMan.GetNode(bone.m_appendBoneIndex));
					node->SetAppendNode(appendNode);
					node->EnableAppendRotate(!!(static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendRotate)));
					node->EnableAppendTranslate(!!(static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendTranslate)));
					node->EnableAppendLocal(!!(static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendLocal)));
					node->SetAppendWeight(bone.m_appendWeight);
				}
			}

			// Process IK
			if (static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::IK))
			{
				auto* ikSolver = m_ikSolverMan.AddIKSolver();
				ikSolver->SetIterateCount(bone.m_ikIterationCount);
				ikSolver->SetLimitAngle(bone.m_ikLimit);
				ikSolver->SetTargetNode(node);

				for (const auto& ikLink : bone.m_ikLinks)
				{
					auto* ikNode = m_nodeMan.GetNode(ikLink.m_ikBoneIndex);
					if (ikLink.m_enableLimit != 0)
					{
						ikSolver->AddIKChain(ikNode, true, ikLink.m_limitMin, ikLink.m_limitMax);
					}
					else
					{
						ikSolver->AddIKChain(ikNode, false, glm::vec3(0), glm::vec3(0));
					}
				}

				node->SetIKSolver(ikSolver);
			}
		}

		// Process morphs
		for (const auto& morph : pmx.m_morphs)
		{
			auto* pmxMorph = static_cast<PMXMorph*>(m_morphMan.AddMorph());
			pmxMorph->SetName(morph.m_name);
			pmxMorph->SetWeight(0);

			switch (morph.m_morphType)
			{
			case PMXMorphType::Position:
				{
					pmxMorph->m_morphType = MorphType::Position;
					auto& morphData = m_positionMorphDatas.emplace_back();
					morphData.m_morphVertices.reserve(morph.m_positionMorph.size());
					for (const auto& vtx : morph.m_positionMorph)
					{
						morphData.m_morphVertices.emplace_back(vtx.m_vertexIndex, vtx.m_position);
					}
					pmxMorph->m_dataIndex = m_positionMorphDatas.size() - 1;
				}
				break;
			case PMXMorphType::UV:
			case PMXMorphType::AddUV1:
			case PMXMorphType::AddUV2:
			case PMXMorphType::AddUV3:
			case PMXMorphType::AddUV4:
				{
					pmxMorph->m_morphType = MorphType::UV;
					auto& morphData = m_uvMorphDatas.emplace_back();
					morphData.m_morphUVs.reserve(morph.m_uvMorph.size());
					for (const auto& uv : morph.m_uvMorph)
					{
						morphData.m_morphUVs.emplace_back(uv.m_vertexIndex, uv.m_uv);
					}
					pmxMorph->m_dataIndex = m_uvMorphDatas.size() - 1;
				}
				break;
			case PMXMorphType::Material:
				{
					pmxMorph->m_morphType = MorphType::Material;
					auto& morphData = m_materialMorphDatas.emplace_back();
					morphData.m_materialMorphs = morph.m_materialMorph;
					pmxMorph->m_dataIndex = m_materialMorphDatas.size() - 1;
				}
				break;
			case PMXMorphType::Bone:
				{
					pmxMorph->m_morphType = MorphType::Bone;
					auto& morphData = m_boneMorphDatas.emplace_back();
					morphData.m_boneMorphs.reserve(morph.m_boneMorph.size());
					for (const auto& bone : morph.m_boneMorph)
					{
						if (bone.m_boneIndex != -1)
						{
							auto* node = m_nodeMan.GetNode(bone.m_boneIndex);
							morphData.m_boneMorphs.emplace_back(node, bone.m_position, bone.m_quaternion);
						}
					}
					pmxMorph->m_dataIndex = m_boneMorphDatas.size() - 1;
				}
				break;
			case PMXMorphType::Group:
				{
					pmxMorph->m_morphType = MorphType::Group;
					auto& morphData = m_groupMorphDatas.emplace_back();
					morphData.m_groupMorphs = morph.m_groupMorph;
					pmxMorph->m_dataIndex = m_groupMorphDatas.size() - 1;
				}
				break;
			default:
				SABA_ERROR("PMX Load Error: Unknown morph type");
				break;
			}
		}

		// Set default parallel update configuration
		SetupParallelUpdate();

		return true;
	}

	void PMXModel::Destroy()
	{
		m_materials.clear();
		m_subMeshes.clear();

		m_positions.clear();
		m_normals.clear();
		m_uvs.clear();
		m_vertexBoneInfos.clear();

		m_indices.clear();

		m_nodeMan.GetNodes()->clear();

		m_updateRanges.clear();
	}

	void PMXModel::SetupParallelUpdate()
	{
		if (m_parallelUpdateCount == 0)
		{
			// Get CPU core count as default thread count
			m_parallelUpdateCount = std::max(1u, std::thread::hardware_concurrency());
		}
		
		// Limit maximum thread count
		constexpr size_t maxParallelCount = 32;
		if (m_parallelUpdateCount > maxParallelCount)
		{
			SABA_WARN("PMXModel::SetParallelUpdateCount parallelCount > {}", maxParallelCount);
			m_parallelUpdateCount = maxParallelCount;
		}

		SABA_INFO("Select PMX Parallel Update Count : {}", m_parallelUpdateCount);

		m_updateRanges.resize(m_parallelUpdateCount);
		m_parallelUpdateFutures.resize(m_parallelUpdateCount - 1);

		// Optimize task allocation strategy
		const size_t vertexCount = m_positions.size();
		constexpr size_t minVerticesPerThread = 1000; // Minimum vertices per thread
		
		if (vertexCount < m_updateRanges.size() * minVerticesPerThread)
		{
			// If vertex count is small, reduce thread count to avoid thread switching overhead
			const size_t numRanges = (vertexCount + minVerticesPerThread - 1) / minVerticesPerThread;
			const size_t verticesPerRange = (vertexCount + numRanges - 1) / numRanges;
			
			for (size_t rangeIdx = 0; rangeIdx < m_updateRanges.size(); rangeIdx++)
			{
				auto& range = m_updateRanges[rangeIdx];
				if (rangeIdx < numRanges)
				{
					range.m_vertexOffset = rangeIdx * verticesPerRange;
					range.m_vertexCount = std::min(verticesPerRange, vertexCount - range.m_vertexOffset);
				}
				else
				{
					range.m_vertexOffset = 0;
					range.m_vertexCount = 0;
				}
			}
		}
		else 
		{
			// For large vertex counts, distribute evenly across all threads
			const size_t verticesPerThread = vertexCount / m_updateRanges.size();
			size_t remainingVertices = vertexCount % m_updateRanges.size();
			size_t currentOffset = 0;

			for (size_t rangeIdx = 0; rangeIdx < m_updateRanges.size(); rangeIdx++)
			{
				auto& range = m_updateRanges[rangeIdx];
				range.m_vertexOffset = currentOffset;
				range.m_vertexCount = verticesPerThread + (remainingVertices > 0 ? 1 : 0);
				currentOffset += range.m_vertexCount;
				if (remainingVertices > 0) remainingVertices--;
			}
		}
	}

	void PMXModel::Update(const UpdateRange& range)
	{
		// Use aligned pointers to optimize SIMD access
		const auto* __restrict position = m_positions.data() + range.m_vertexOffset;
		const auto* __restrict normal = m_normals.data() + range.m_vertexOffset;
		const auto* __restrict uv = m_uvs.data() + range.m_vertexOffset;
		const auto* __restrict morphPos = m_morphPositions.data() + range.m_vertexOffset;
		const auto* __restrict morphUV = m_morphUVs.data() + range.m_vertexOffset;
		const auto* __restrict vtxInfo = m_vertexBoneInfos.data() + range.m_vertexOffset;
		const auto* __restrict transforms = m_transforms.data();
		auto* __restrict updatePosition = m_updatePositions.data() + range.m_vertexOffset;
		auto* __restrict updateNormal = m_updateNormals.data() + range.m_vertexOffset;
		auto* __restrict updateUV = m_updateUVs.data() + range.m_vertexOffset;

		// Prefetch data to cache
		_mm_prefetch(reinterpret_cast<const char*>(position), _MM_HINT_T0);
		_mm_prefetch(reinterpret_cast<const char*>(normal), _MM_HINT_T0);
		_mm_prefetch(reinterpret_cast<const char*>(vtxInfo), _MM_HINT_T0);

		// Batch process vertices
		constexpr size_t batchSize = 4; // SIMD width
		const size_t vectorizedCount = range.m_vertexCount / batchSize * batchSize;
		
		// Vectorized main loop
		for (size_t i = 0; i < vectorizedCount; i += batchSize)
		{
			// Prefetch next batch of data
			_mm_prefetch(reinterpret_cast<const char*>(position + i + batchSize), _MM_HINT_T0);
			_mm_prefetch(reinterpret_cast<const char*>(normal + i + batchSize), _MM_HINT_T0);
			_mm_prefetch(reinterpret_cast<const char*>(vtxInfo + i + batchSize), _MM_HINT_T0);

			for (size_t j = 0; j < batchSize; ++j)
			{
				const size_t idx = i + j;
				glm::mat4 m;
				
				// Calculate transform matrix based on skinning type
				switch (vtxInfo[idx].m_skinningType)
				{
				case SkinningType::Weight1:
					{
						const auto i0 = vtxInfo[idx].m_boneIndex[0];
						m = transforms[i0];
						break;
					}
				case SkinningType::Weight2:
					{
						const auto i0 = vtxInfo[idx].m_boneIndex[0];
						const auto i1 = vtxInfo[idx].m_boneIndex[1];
						const auto w0 = vtxInfo[idx].m_boneWeight[0];
						m = transforms[i0] * w0 + transforms[i1] * (1.0f - w0);
						break;
					}
				case SkinningType::Weight4:
					{
						const auto i0 = vtxInfo[idx].m_boneIndex[0];
						const auto i1 = vtxInfo[idx].m_boneIndex[1];
						const auto i2 = vtxInfo[idx].m_boneIndex[2];
						const auto i3 = vtxInfo[idx].m_boneIndex[3];
						const auto w0 = vtxInfo[idx].m_boneWeight[0];
						const auto w1 = vtxInfo[idx].m_boneWeight[1];
						const auto w2 = vtxInfo[idx].m_boneWeight[2];
						const auto w3 = vtxInfo[idx].m_boneWeight[3];
						m = transforms[i0] * w0 + transforms[i1] * w1 + 
							transforms[i2] * w2 + transforms[i3] * w3;
						break;
					}
				default:
					m = glm::mat4(1.0f);
					break;
				}

				// Apply transforms
				updatePosition[idx] = glm::vec3(m * glm::vec4(position[idx] + morphPos[idx], 1.0f));
				updateNormal[idx] = glm::normalize(glm::mat3(m) * normal[idx]);
				updateUV[idx] = glm::vec2(uv[idx].x + morphUV[idx].x, uv[idx].y + morphUV[idx].y);
			}
		}

		// Process remaining vertices
		for (size_t i = vectorizedCount; i < range.m_vertexCount; ++i)
		{
			glm::mat4 m;
			switch (vtxInfo[i].m_skinningType)
			{
			case SkinningType::Weight1:
				{
					const auto i0 = vtxInfo[i].m_boneIndex[0];
					m = transforms[i0];
					break;
				}
			case SkinningType::Weight2:
				{
					const auto i0 = vtxInfo[i].m_boneIndex[0];
					const auto i1 = vtxInfo[i].m_boneIndex[1];
					const auto w0 = vtxInfo[i].m_boneWeight[0];
					m = transforms[i0] * w0 + transforms[i1] * (1.0f - w0);
					break;
				}
			case SkinningType::Weight4:
				{
					const auto i0 = vtxInfo[i].m_boneIndex[0];
					const auto i1 = vtxInfo[i].m_boneIndex[1];
					const auto i2 = vtxInfo[i].m_boneIndex[2];
					const auto i3 = vtxInfo[i].m_boneIndex[3];
					const auto w0 = vtxInfo[i].m_boneWeight[0];
					const auto w1 = vtxInfo[i].m_boneWeight[1];
					const auto w2 = vtxInfo[i].m_boneWeight[2];
					const auto w3 = vtxInfo[i].m_boneWeight[3];
					m = transforms[i0] * w0 + transforms[i1] * w1 + 
						transforms[i2] * w2 + transforms[i3] * w3;
					break;
				}
			default:
				m = glm::mat4(1.0f);
				break;
			}

			updatePosition[i] = glm::vec3(m * glm::vec4(position[i] + morphPos[i], 1.0f));
			updateNormal[i] = glm::normalize(glm::mat3(m) * normal[i]);
			updateUV[i] = glm::vec2(uv[i].x + morphUV[i].x, uv[i].y + morphUV[i].y);
		}
	}

	void PMXModel::Morph(const PMXMorph* morph, const float weight)
	{
		std::stack<std::pair<const PMXMorph*, float>> morphStack;
		morphStack.emplace(morph, weight);

		while (!morphStack.empty())
		{
			auto [currentMorph, currentWeight] = morphStack.top();
			morphStack.pop();

			switch (currentMorph->m_morphType)
			{
			case MorphType::Position:
				MorphPosition(m_positionMorphDatas[currentMorph->m_dataIndex], currentWeight);
				break;
			case MorphType::UV:
				MorphUV(m_uvMorphDatas[currentMorph->m_dataIndex], currentWeight);
				break;
			case MorphType::Material:
				MorphMaterial(m_materialMorphDatas[currentMorph->m_dataIndex], currentWeight);
				break;
			case MorphType::Bone:
				MorphBone(m_boneMorphDatas[currentMorph->m_dataIndex], currentWeight);
				break;
			case MorphType::Group:
			{
				const auto& [m_groupMorphs] = m_groupMorphDatas[currentMorph->m_dataIndex];
				for (const auto& [m_morphIndex, m_weight] : m_groupMorphs)
				{
					if (m_morphIndex == -1)
					{
						continue;
					}
					auto& elemMorph = (*m_morphMan.GetMorphs())[m_morphIndex];
					morphStack.emplace(elemMorph.get(), m_weight * currentWeight);
				}
				break;
			}
			default:
				break;
			}
		}
	}

	void PMXModel::MorphPosition(const PositionMorphData & morphData, const float weight)
	{
		if (weight == 0)
		{
			return;
		}

		for (const auto& [m_index, m_position] : morphData.m_morphVertices)
		{
			m_morphPositions[m_index] += m_position * weight;
		}
	}

	void PMXModel::MorphUV(const UVMorphData & morphData, const float weight)
	{
		if (weight == 0)
		{
			return;
		}

		for (const auto& [m_index, m_uv] : morphData.m_morphUVs)
		{
			m_morphUVs[m_index] += m_uv * weight;
		}
	}

	void PMXModel::BeginMorphMaterial()
	{
		MaterialFactor initMul{
			glm::vec3(1), 1, glm::vec3(1), 1, glm::vec3(1),
			glm::vec4(1), 1, glm::vec4(1), glm::vec4(1), glm::vec4(1)
		};

		MaterialFactor initAdd{
			glm::vec3(0), 0, glm::vec3(0), 0, glm::vec3(0),
			glm::vec4(0), 0, glm::vec4(0), glm::vec4(0), glm::vec4(0)
		};

		const size_t matCount = m_materials.size();
		for (size_t matIdx = 0; matIdx < matCount; matIdx++)
		{
			m_mulMaterialFactors[matIdx] = initMul;
			m_mulMaterialFactors[matIdx].m_diffuse = m_materials[matIdx].m_diffuse;
			m_mulMaterialFactors[matIdx].m_alpha = m_materials[matIdx].m_alpha;
			m_mulMaterialFactors[matIdx].m_specular = m_materials[matIdx].m_specular;
			m_mulMaterialFactors[matIdx].m_specularPower = m_materials[matIdx].m_specularPower;
			m_mulMaterialFactors[matIdx].m_ambient = m_materials[matIdx].m_ambient;

			m_addMaterialFactors[matIdx] = initAdd;
		}
	}

	void PMXModel::EndMorphMaterial()
	{
		const size_t matCount = m_materials.size();
		for (size_t matIdx = 0; matIdx < matCount; matIdx++)
		{
			MaterialFactor matFactor = m_mulMaterialFactors[matIdx];
			matFactor.Add(m_addMaterialFactors[matIdx], 1.0f);

			m_materials[matIdx].m_diffuse = matFactor.m_diffuse;
			m_materials[matIdx].m_alpha = matFactor.m_alpha;
			m_materials[matIdx].m_specular = matFactor.m_specular;
			m_materials[matIdx].m_specularPower = matFactor.m_specularPower;
			m_materials[matIdx].m_ambient = matFactor.m_ambient;
			m_materials[matIdx].m_textureMulFactor = m_mulMaterialFactors[matIdx].m_textureFactor;
			m_materials[matIdx].m_textureAddFactor = m_addMaterialFactors[matIdx].m_textureFactor;
			m_materials[matIdx].m_spTextureMulFactor = m_mulMaterialFactors[matIdx].m_spTextureFactor;
			m_materials[matIdx].m_spTextureAddFactor = m_addMaterialFactors[matIdx].m_spTextureFactor;
			m_materials[matIdx].m_toonTextureMulFactor = m_mulMaterialFactors[matIdx].m_toonTextureFactor;
			m_materials[matIdx].m_toonTextureAddFactor = m_addMaterialFactors[matIdx].m_toonTextureFactor;
		}
	}

	void PMXModel::MorphMaterial(const MaterialMorphData & morphData, const float weight)
	{
		for (const auto& matMorph : morphData.m_materialMorphs)
		{
			if (matMorph.m_materialIndex != -1)
			{
				const auto mi = matMorph.m_materialIndex;
				switch (matMorph.m_opType)
				{
				case saba::PMXFileMorph::MaterialMorph::OpType::Mul:
					m_mulMaterialFactors[mi].Mul(
						MaterialFactor(matMorph),
						weight
					);
					break;
				case saba::PMXFileMorph::MaterialMorph::OpType::Add:
					m_addMaterialFactors[mi].Add(
						MaterialFactor(matMorph),
						weight
					);
					break;
				default:
					break;
				}
			}
			else
			{
				switch (matMorph.m_opType)
				{
				case saba::PMXFileMorph::MaterialMorph::OpType::Mul:
					for (size_t i = 0; i < m_materials.size(); i++)
					{
						m_mulMaterialFactors[i].Mul(
							MaterialFactor(matMorph),
							weight
						);
					}
					break;
				case saba::PMXFileMorph::MaterialMorph::OpType::Add:
					for (size_t i = 0; i < m_materials.size(); i++)
					{
						m_addMaterialFactors[i].Add(
							MaterialFactor(matMorph),
							weight
						);
					}
					break;
				default:
					break;
				}
			}
		}
	}

	void PMXModel::MorphBone(const BoneMorphData & morphData, const float weight)
	{
		for (const auto& [m_node, m_position, m_rotate] : morphData.m_boneMorphs)
		{
			const auto node = m_node;
			glm::vec3 t = mix(glm::vec3(0), m_position, weight);
			node->SetTranslate(node->GetTranslate() + t);
			glm::quat q = slerp(node->GetRotate(), m_rotate, weight);
			node->SetRotate(q);
		}
	}

	PMXNode::PMXNode()
		: m_deformDepth(-1)
		, m_isDeformAfterPhysics(false)
		, m_appendNode(nullptr)
		, m_isAppendRotate(false)
		, m_isAppendTranslate(false)
		, m_isAppendLocal(false)
		, m_appendWeight(0)
		, m_ikSolver(nullptr)
	{
	}

	void PMXNode::UpdateAppendTransform()
	{
		if (m_appendNode == nullptr)
		{
			return;
		}

		if (m_isAppendRotate)
		{
			glm::quat appendRotate;
			if (m_isAppendLocal)
			{
				appendRotate = m_appendNode->AnimateRotate();
			}
			else
			{
				if (m_appendNode->GetAppendNode() != nullptr)
				{
					appendRotate = m_appendNode->GetAppendRotate();
				}
				else
				{
					appendRotate = m_appendNode->AnimateRotate();
				}
			}

			if (m_appendNode->m_enableIK)
			{
				appendRotate = m_appendNode->GetIKRotate() * appendRotate;
			}

			const glm::quat appendQ = slerp(
				glm::quat(1, 0, 0, 0),
				appendRotate,
				GetAppendWeight()
			);
			m_appendRotate = appendQ;
		}

		if (m_isAppendTranslate)
		{
			glm::vec3 appendTranslate{};
			if (m_isAppendLocal)
			{
				appendTranslate = m_appendNode->GetTranslate() - m_appendNode->GetInitialTranslate();
			}
			else
			{
				if (m_appendNode->GetAppendNode() != nullptr)
				{
					appendTranslate = m_appendNode->GetAppendTranslate();
				}
				else
				{
					appendTranslate = m_appendNode->GetTranslate() - m_appendNode->GetInitialTranslate();
				}
			}

			m_appendTranslate = appendTranslate * GetAppendWeight();
		}

		UpdateLocalTransform();
	}

	void PMXNode::OnBeginUpdateTransform()
	{
		m_appendTranslate = glm::vec3(0);
		m_appendRotate = glm::quat(1, 0, 0, 0);
	}

	void PMXNode::OnEndUpdateTransfrom()
	{
	}

	void PMXNode::OnUpdateLocalTransform()
	{
		glm::vec3 t = AnimateTranslate();
		if (m_isAppendTranslate)
		{
			t += m_appendTranslate;
		}

		glm::quat r = AnimateRotate();
		if (m_enableIK)
		{
			r = GetIKRotate() * r;
		}
		if (m_isAppendRotate)
		{
			r = r * m_appendRotate;
		}

		const glm::vec3 s = GetScale();

		m_local = translate(glm::mat4(1), t)
			* mat4_cast(r)
			* scale(glm::mat4(1), s);
	}
}
