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
#include <mutex>
#include <stack>
#include <condition_variable>

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
		// Morph の処理
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

		// スキンメッシュに使用する変形マトリクスを事前計算
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

		size_t vertexCount = pmx.m_vertices.size();
		m_positions.reserve(vertexCount);
		m_normals.reserve(vertexCount);
		m_uvs.reserve(vertexCount);
		m_vertexBoneInfos.reserve(vertexCount);
		m_bboxMax = glm::vec3(-std::numeric_limits<float>::max());
		m_bboxMin = glm::vec3(std::numeric_limits<float>::max());

		bool warnSDEF = false;
		bool infoQDEF = false;
		for (const auto& [m_position, m_normal, m_uv, m_addUV, m_weightType, m_boneIndices, m_boneWeights, m_sdefC, m_sdefR0, m_sdefR1, m_edgeMag] : pmx.m_vertices)
		{
			glm::vec3 pos = m_position * glm::vec3(1, 1, -1);
			glm::vec3 nor = m_normal * glm::vec3(1, 1, -1);
			auto uv = glm::vec2(m_uv.x, 1.0f - m_uv.y);
			m_positions.push_back(pos);
			m_normals.push_back(nor);
			m_uvs.push_back(uv);
			VertexBoneInfo vtxBoneInfo{};
			if (PMXVertexWeight::SDEF != m_weightType)
			{
				vtxBoneInfo.m_boneIndex[0] = m_boneIndices[0];
				vtxBoneInfo.m_boneIndex[1] = m_boneIndices[1];
				vtxBoneInfo.m_boneIndex[2] = m_boneIndices[2];
				vtxBoneInfo.m_boneIndex[3] = m_boneIndices[3];

				vtxBoneInfo.m_boneWeight[0] = m_boneWeights[0];
				vtxBoneInfo.m_boneWeight[1] = m_boneWeights[1];
				vtxBoneInfo.m_boneWeight[2] = m_boneWeights[2];
				vtxBoneInfo.m_boneWeight[3] = m_boneWeights[3];
			}

			switch (m_weightType)
			{
			case PMXVertexWeight::BDEF1:
				vtxBoneInfo.m_skinningType = SkinningType::Weight1;
				break;
			case PMXVertexWeight::BDEF2:
				vtxBoneInfo.m_skinningType = SkinningType::Weight2;
				vtxBoneInfo.m_boneWeight[1] = 1.0f - vtxBoneInfo.m_boneWeight[0];
				break;
			case PMXVertexWeight::BDEF4:
				vtxBoneInfo.m_skinningType = SkinningType::Weight4;
				break;
			case PMXVertexWeight::SDEF:
				if (!warnSDEF)
				{
					SABA_WARN("Use SDEF");
					warnSDEF = true;
				}
				vtxBoneInfo.m_skinningType = SkinningType::SDEF;
				{
					auto w0 = m_boneWeights[0];
					auto w1 = 1.0f - w0;

					auto center = m_sdefC * glm::vec3(1, 1, -1);
					auto r0 = m_sdefR0 * glm::vec3(1, 1, -1);
					auto r1 = m_sdefR1 * glm::vec3(1, 1, -1);
					auto rw = r0 * w0 + r1 * w1;
					r0 = center + r0 - rw;
					r1 = center + r1 - rw;
					auto cr0 = (center + r0) * 0.5f;
					auto cr1 = (center + r1) * 0.5f;

					vtxBoneInfo.m_sdef.m_boneIndex[0] = m_boneIndices[0];
					vtxBoneInfo.m_sdef.m_boneIndex[1] = m_boneIndices[1];
					vtxBoneInfo.m_sdef.m_boneWeight = m_boneWeights[0];
					vtxBoneInfo.m_sdef.m_sdefC = center;
					vtxBoneInfo.m_sdef.m_sdefR0 = cr0;
					vtxBoneInfo.m_sdef.m_sdefR1 = cr1;
				}
				break;
			case PMXVertexWeight::QDEF:
				vtxBoneInfo.m_skinningType = SkinningType::DualQuaternion;
				if (!infoQDEF)
				{
					SABA_INFO("Use QDEF");
					infoQDEF = true;
				}
				break;
			default:
				vtxBoneInfo.m_skinningType = SkinningType::Weight1;
				SABA_ERROR("Unknown PMX Vertex Weight Type: {}", static_cast<int>(m_weightType));
				break;
			}
			m_vertexBoneInfos.push_back(vtxBoneInfo);

			m_bboxMax = max(m_bboxMax, pos);
			m_bboxMin = min(m_bboxMin, pos);
		}
		m_morphPositions.resize(m_positions.size());
		m_morphUVs.resize(m_positions.size());
		m_updatePositions.resize(m_positions.size());
		m_updateNormals.resize(m_normals.size());
		m_updateUVs.resize(m_uvs.size());


		m_indexElementSize = pmx.m_header.m_vertexIndexSize;
		m_indices.resize(pmx.m_faces.size() * 3 * m_indexElementSize);
		m_indexCount = pmx.m_faces.size() * 3;
		switch (m_indexElementSize)
		{
		case 1:
		{
			int idx = 0;
			auto indices = reinterpret_cast<uint8_t*>(m_indices.data());
			for (const auto& [m_vertices] : pmx.m_faces)
			{
				for (int i = 0; i < 3; i++)
				{
					auto vi = m_vertices[3 - i - 1];
					indices[idx] = static_cast<uint8_t>(vi);
					idx++;
				}
			}
			break;
		}
		case 2:
		{
			int idx = 0;
			auto indices = reinterpret_cast<uint16_t*>(m_indices.data());
			for (const auto& [m_vertices] : pmx.m_faces)
			{
				for (int i = 0; i < 3; i++)
				{
					auto vi = m_vertices[3 - i - 1];
					indices[idx] = static_cast<uint16_t>(vi);
					idx++;
				}
			}
			break;
		}
		case 4:
		{
			int idx = 0;
			auto indices = reinterpret_cast<uint32_t*>(m_indices.data());
			for (const auto& [m_vertices] : pmx.m_faces)
			{
				for (int i = 0; i < 3; i++)
				{
					auto vi = m_vertices[3 - i - 1];
					indices[idx] = vi;
					idx++;
				}
			}
			break;
		}
		default:
			SABA_ERROR("Unsupported Index Size: [{}]", m_indexElementSize);
			return false;
		}

		std::vector<std::string> texturePaths;
		texturePaths.reserve(pmx.m_textures.size());
		for (const auto& [m_textureName] : pmx.m_textures)
		{
			std::string texPath = PathUtil::Combine(dirPath, m_textureName);
			texturePaths.emplace_back(std::move(texPath));
		}

		// Materialをコピー
		m_materials.reserve(pmx.m_materials.size());
		m_subMeshes.reserve(pmx.m_materials.size());
		uint32_t beginIndex = 0;
		for (const auto& [m_name, m_englishName, m_diffuse, m_specular, m_specularPower, m_ambient, m_drawMode, m_edgeColor, m_edgeSize, m_textureIndex, m_sphereTextureIndex, m_sphereMode, m_toonMode, m_toonTextureIndex, m_memo, m_numFaceVertices] : pmx.m_materials)
		{
			MMDMaterial mat;
			mat.m_diffuse = m_diffuse;
			mat.m_alpha = m_diffuse.a;
			mat.m_specularPower = m_specularPower;
			mat.m_specular = m_specular;
			mat.m_ambient = m_ambient;
			mat.m_spTextureMode = MMDMaterial::SphereTextureMode::None;
			mat.m_bothFace = !!(static_cast<uint8_t>(m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::BothFace));
			mat.m_edgeFlag = (static_cast<uint8_t>(m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::DrawEdge)) == 0 ? 0 : 1;
			mat.m_groundShadow = !!(static_cast<uint8_t>(m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::GroundShadow));
			mat.m_shadowCaster = !!(static_cast<uint8_t>(m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::CastSelfShadow));
			mat.m_shadowReceiver = !!(static_cast<uint8_t>(m_drawMode) & static_cast<uint8_t>(PMXDrawModeFlags::RecieveSelfShadow));
			mat.m_edgeSize = m_edgeSize;
			mat.m_edgeColor = m_edgeColor;

			// Texture
			if (m_textureIndex != -1)
			{
				mat.m_texture = PathUtil::Normalize(texturePaths[m_textureIndex]);
			}

			// ToonTexture
			if (m_toonMode == PMXToonMode::Common)
			{
				if (m_toonTextureIndex != -1)
				{
					std::stringstream ss;
					ss << "toon" << std::setfill('0') << std::setw(2) << m_toonTextureIndex + 1 << ".bmp";
					mat.m_toonTexture = PathUtil::Combine(mmdDataDir, ss.str());
				}
			}
			else if (m_toonMode == PMXToonMode::Separate)
			{
				if (m_toonTextureIndex != -1)
				{
					mat.m_toonTexture = PathUtil::Normalize(texturePaths[m_toonTextureIndex]);
				}
			}

			// SpTexture
			if (m_sphereTextureIndex != -1)
			{
				mat.m_spTexture = PathUtil::Normalize(texturePaths[m_sphereTextureIndex]);
				mat.m_spTextureMode = MMDMaterial::SphereTextureMode::None;
				if (m_sphereMode == PMXSphereMode::Mul)
				{
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Mul;
				}
				else if (m_sphereMode == PMXSphereMode::Add)
				{
					mat.m_spTextureMode = MMDMaterial::SphereTextureMode::Add;
				}
				else if (m_sphereMode == PMXSphereMode::SubTexture)
				{
					// TODO: SphareTexture が SubTexture の処理
				}
			}

			m_materials.emplace_back(mat);
			m_subMeshes.emplace_back(static_cast<int>(beginIndex), static_cast<int>(m_numFaceVertices), static_cast<int>(m_materials.size() - 1));

			beginIndex = beginIndex + m_numFaceVertices;
		}
		m_initMaterials = m_materials;
		m_mulMaterialFactors.resize(m_materials.size());
		m_addMaterialFactors.resize(m_materials.size());

		// Node
		m_nodeMan.GetNodes()->reserve(pmx.m_bones.size());
		for (const auto& bone : pmx.m_bones)
		{
			auto* node = m_nodeMan.AddNode();
			node->SetName(bone.m_name);
		}
		for (size_t i = 0; i < pmx.m_bones.size(); i++)
		{
			auto boneIndex = static_cast<int32_t>(pmx.m_bones.size() - i - 1);
			const auto& [m_name, m_englishName, m_position, m_parentBoneIndex,
				m_deformDepth, m_boneFlag, m_positionOffset, m_linkBoneIndex,
				m_appendBoneIndex, m_appendWeight, m_fixedAxis,
				m_localXAxis, m_localZAxis, m_keyValue, m_ikTargetBoneIndex, m_ikIterationCount,
				m_ikLimit, m_ikLinks] = pmx.m_bones[boneIndex];
			auto* node = m_nodeMan.GetNode(boneIndex);

			// Check if the node is looping
			bool isLooping = false;
			if (m_parentBoneIndex != -1)
			{
				MMDNode* parent = m_nodeMan.GetNode(m_parentBoneIndex);
				while (parent != nullptr)
				{
					if (parent == node)
					{
						isLooping = true;
						SABA_ERROR("This bone hierarchy is a loop: bone={}", boneIndex);
						break;
					}
					parent = parent->GetParent();
				}
			}

			// Check parent node index
			if (m_parentBoneIndex != -1)
			{
				if (m_parentBoneIndex >= boneIndex)
				{
					SABA_WARN("The parent index of this node is big: bone={}", boneIndex);
				}
			}

			if (m_parentBoneIndex != -1 && !isLooping)
			{
				const auto& parentBone = pmx.m_bones[m_parentBoneIndex];
				auto* parent = m_nodeMan.GetNode(m_parentBoneIndex);
				parent->AddChild(node);
				auto localPos = m_position - parentBone.m_position;
				localPos.z *= -1;
				node->SetTranslate(localPos);
			}
			else
			{
				auto localPos = m_position;
				localPos.z *= -1;
				node->SetTranslate(localPos);
			}
			glm::mat4 init = translate(
				glm::mat4(1),
				m_position * glm::vec3(1, 1, -1)
			);
			node->SetGlobalTransform(init);
			node->CalculateInverseInitTransform();

			node->SetDeformDepth(m_deformDepth);
			bool deformAfterPhysics = !!(static_cast<uint16_t>(m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::DeformAfterPhysics));
			node->EnableDeformAfterPhysics(deformAfterPhysics);
			bool appendRotate = (static_cast<uint16_t>(m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendRotate)) != 0;
			bool appendTranslate = (static_cast<uint16_t>(m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendTranslate)) != 0;
			node->EnableAppendRotate(appendRotate);
			node->EnableAppendTranslate(appendTranslate);
			if ((appendRotate || appendTranslate) && m_appendBoneIndex != -1)
			{
				if (m_appendBoneIndex >= boneIndex)
				{
					SABA_WARN("The parent(morph assignment) index of this node is big: bone={}", boneIndex);
				}
				bool appendLocal = (static_cast<uint16_t>(m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::AppendLocal)) != 0;
				auto appendNode = m_nodeMan.GetNode(m_appendBoneIndex);
				float appendWeight = m_appendWeight;
				node->EnableAppendLocal(appendLocal);
				node->SetAppendNode(appendNode);
				node->SetAppendWeight(appendWeight);
			}
			node->SaveInitialTRS();
		}
		m_transforms.resize(m_nodeMan.GetNodeCount());

		m_sortedNodes.clear();
		m_sortedNodes.reserve(m_nodeMan.GetNodeCount());
		auto* pmxNodes = m_nodeMan.GetNodes();
		for (auto& pmxNode : *pmxNodes)
		{
			m_sortedNodes.push_back(pmxNode.get());
		}
		std::stable_sort(
			m_sortedNodes.begin(),
			m_sortedNodes.end(),
			[](const PMXNode* x, const PMXNode* y) {return x->GetDeformDepth() < y->GetDeformDepth(); }
		);

		// IK
		for (size_t i = 0; i < pmx.m_bones.size(); i++)
		{
			if (const auto& bone = pmx.m_bones[i]; static_cast<uint16_t>(bone.m_boneFlag) & static_cast<uint16_t>(PMXBoneFlags::IK))
			{
				auto solver = m_ikSolverMan.AddIKSolver();
				auto* ikNode = m_nodeMan.GetNode(i);
				solver->SetIKNode(ikNode);
				ikNode->SetIKSolver(solver);

				if (bone.m_ikTargetBoneIndex < 0 || bone.m_ikTargetBoneIndex >= static_cast<int>(m_nodeMan.GetNodeCount()))
				{
					SABA_ERROR("Wrong IK Target: bone={} target={}", i, bone.m_ikTargetBoneIndex);
					continue;
				}

				auto* targetNode = m_nodeMan.GetNode(bone.m_ikTargetBoneIndex);
				solver->SetTargetNode(targetNode);

				for (const auto& [m_ikBoneIndex, m_enableLimit, m_limitMin, m_limitMax] : bone.m_ikLinks)
				{
					auto* linkNode = m_nodeMan.GetNode(m_ikBoneIndex);
					if (m_enableLimit)
					{
						glm::vec3 limitMax = m_limitMin * glm::vec3(-1);
						glm::vec3 limitMin = m_limitMax * glm::vec3(-1);
						solver->AddIKChain(linkNode, true, limitMin, limitMax);
					}
					else
					{
						solver->AddIKChain(linkNode);
					}
					linkNode->EnableIK(true);
				}

				solver->SetIterateCount(bone.m_ikIterationCount);
				solver->SetLimitAngle(bone.m_ikLimit);
			}
		}

		// Morph
		for (const auto& [m_name, m_englishName, m_controlPanel, m_morphType, m_positionMorph, m_uvMorph, m_boneMorph, m_materialMorph, m_groupMorph, m_flipMorph, m_impulseMorph] : pmx.m_morphs)
		{
			auto morph = m_morphMan.AddMorph();
			morph->SetName(m_name);
			morph->SetWeight(0.0f);
			morph->m_morphType = MorphType::None;
			if (m_morphType == PMXMorphType::Position)
			{
				morph->m_morphType = MorphType::Position;
				morph->m_dataIndex = m_positionMorphDatas.size();
				PositionMorphData morphData;
				for (const auto& [m_vertexIndex, m_position] : m_positionMorph)
				{
					PositionMorph morphVtx{};
					morphVtx.m_index = m_vertexIndex;
					morphVtx.m_position = m_position * glm::vec3(1, 1, -1);
					morphData.m_morphVertices.push_back(morphVtx);
				}
				m_positionMorphDatas.emplace_back(std::move(morphData));
			}
			else if (m_morphType == PMXMorphType::UV)
			{
				morph->m_morphType = MorphType::UV;
				morph->m_dataIndex = m_uvMorphDatas.size();
				UVMorphData morphData;
				for (const auto& [m_vertexIndex, m_uv] : m_uvMorph)
				{
					UVMorph morphUV{};
					morphUV.m_index = m_vertexIndex;
					morphUV.m_uv = m_uv;
					morphData.m_morphUVs.push_back(morphUV);
				}
				m_uvMorphDatas.emplace_back(std::move(morphData));
			}
			else if (m_morphType == PMXMorphType::Material)
			{
				morph->m_morphType = MorphType::Material;
				morph->m_dataIndex = m_materialMorphDatas.size();

				MaterialMorphData materialMorphData;
				materialMorphData.m_materialMorphs = m_materialMorph;
				m_materialMorphDatas.emplace_back(std::move(materialMorphData));
			}
			else if (m_morphType == PMXMorphType::Bone)
			{
				morph->m_morphType = MorphType::Bone;
				morph->m_dataIndex = m_boneMorphDatas.size();

				BoneMorphData boneMorphData;
				for (const auto& [m_boneIndex, m_position, m_quaternion] : m_boneMorph)
				{
					BoneMorphElement boneMorphElem{};
					boneMorphElem.m_node = m_nodeMan.GetMMDNode(m_boneIndex);
					boneMorphElem.m_position = m_position * glm::vec3(1, 1, -1);
					const glm::quat q = m_quaternion;
					auto invZ = glm::mat3(scale(glm::mat4(1), glm::vec3(1, 1, -1)));
					auto rot0 = mat3_cast(q);
					auto rot1 = invZ * rot0 * invZ;
					boneMorphElem.m_rotate = quat_cast(rot1);
					boneMorphData.m_boneMorphs.push_back(boneMorphElem);
				}
				m_boneMorphDatas.emplace_back(std::move(boneMorphData));
			}
			else if (m_morphType == PMXMorphType::Group)
			{
				morph->m_morphType = MorphType::Group;
				morph->m_dataIndex = m_groupMorphDatas.size();

				GroupMorphData groupMorphData;
				groupMorphData.m_groupMorphs = m_groupMorph;
				m_groupMorphDatas.emplace_back(std::move(groupMorphData));
			}
			else
			{
				SABA_WARN("Not Supported Morp Type({}): [{}]",
					static_cast<uint8_t>(m_morphType),
					m_name
				);
			}
		}

		// Check whether Group Morph infinite loop.
		{
			std::vector<int32_t> groupMorphStack;
			std::function<void(int32_t)> fixInifinitGropuMorph;
			fixInifinitGropuMorph = [this, &fixInifinitGropuMorph, &groupMorphStack](const int32_t morphIdx)
			{
				const auto& morphs = *m_morphMan.GetMorphs();
				if (const auto& morph = morphs[morphIdx]; morph->m_morphType == MorphType::Group)
				{
					auto& [m_groupMorphs] = m_groupMorphDatas[morph->m_dataIndex];
					for (size_t i = 0; i < m_groupMorphs.size(); i++)
					{
						auto& [m_morphIndex, m_weight] = m_groupMorphs[i];

						auto findIt = std::find(
							groupMorphStack.begin(),
							groupMorphStack.end(),
							m_morphIndex
						);
						if (findIt != groupMorphStack.end())
						{
							SABA_WARN("Infinit Group Morph:[{}][{}][{}]",
								morphIdx, morph->GetName(), i
							);
							m_morphIndex = -1;
						}
						else
						{
							groupMorphStack.push_back(morphIdx);
							if (m_morphIndex>0)
								fixInifinitGropuMorph(m_morphIndex);
							else
								SABA_ERROR("Invalid morph index: group={}, morph={}", m_morphIndex, morphIdx);
							groupMorphStack.pop_back();
						}
					}
				}
			};

			for (int32_t morphIdx = 0; morphIdx < static_cast<int32_t>(m_morphMan.GetMorphCount()); morphIdx++)
			{
				fixInifinitGropuMorph(morphIdx);
				groupMorphStack.clear();
			}

		}

		// Physics
		if (!m_physicsMan.Create())
		{
			SABA_ERROR("Create Physics Fail.");
			return false;
		}

		for (const auto& pmxRB : pmx.m_rigidbodies)
		{
			auto rb = m_physicsMan.AddRigidBody();
			MMDNode* node = nullptr;
			if (pmxRB.m_boneIndex != -1)
			{
				node = m_nodeMan.GetMMDNode(pmxRB.m_boneIndex);
			}
			if (!rb->Create(pmxRB, this, node))
			{
				SABA_ERROR("Create Rigid Body Fail.\n");
				return false;
			}
			m_physicsMan.GetMMDPhysics()->AddRigidBody(rb);
		}

		for (const auto& pmxJoint : pmx.m_joints)
		{
			if (pmxJoint.m_rigidbodyAIndex != -1 &&
				pmxJoint.m_rigidbodyBIndex != -1 &&
				pmxJoint.m_rigidbodyAIndex != pmxJoint.m_rigidbodyBIndex)
			{
				auto joint = m_physicsMan.AddJoint();
				auto rigidBodys = m_physicsMan.GetRigidBodys();
				bool ret = joint->CreateJoint(
					pmxJoint,
					(*rigidBodys)[pmxJoint.m_rigidbodyAIndex].get(),
					(*rigidBodys)[pmxJoint.m_rigidbodyBIndex].get()
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
				SABA_WARN("Illegal Joint [{}]", pmxJoint.m_name.c_str());
			}
		}

		ResetPhysics();

		SetupParallelUpdate();

		return true;
	}

	bool PMXModel::Save(const std::string& filepath, const std::string& mmdDataDir)
	{
		// TODO: Save PMX
		return false;
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
			m_parallelUpdateCount = std::thread::hardware_concurrency();
		}
		if (const size_t maxParallelCount = std::max(static_cast<size_t>(16), static_cast<size_t>(std::thread::hardware_concurrency()));
			m_parallelUpdateCount > maxParallelCount)
		{
			SABA_WARN("PMXModel::SetParallelUpdateCount parallelCount > {}", maxParallelCount);
			m_parallelUpdateCount = 16;
		}

		SABA_INFO("Select PMX Parallel Update Count : {}", m_parallelUpdateCount);

		m_updateRanges.resize(m_parallelUpdateCount);
		m_parallelUpdateFutures.resize(m_parallelUpdateCount - 1);

		const size_t vertexCount = m_positions.size();
		if (constexpr size_t LowerVertexCount = 1000; vertexCount < m_updateRanges.size() * LowerVertexCount)
		{
			const size_t numRanges = (vertexCount + LowerVertexCount - 1) / LowerVertexCount;
			for (size_t rangeIdx = 0; rangeIdx < m_updateRanges.size(); rangeIdx++)
			{
				auto& [m_vertexOffset, m_vertexCount] = m_updateRanges[rangeIdx];
				if (rangeIdx < numRanges)
				{
					m_vertexOffset = rangeIdx * LowerVertexCount;
					m_vertexCount = std::min(LowerVertexCount, vertexCount - m_vertexOffset);
				}
				else
				{
					m_vertexOffset = 0;
					m_vertexCount = 0;
				}
			}
		}
		else
		{
			const size_t numVertexCount = vertexCount / m_updateRanges.size();
			size_t offset = 0;
			for (size_t rangeIdx = 0; rangeIdx < m_updateRanges.size(); rangeIdx++)
			{
				auto& [m_vertexOffset, m_vertexCount] = m_updateRanges[rangeIdx];
				m_vertexOffset = offset;
				m_vertexCount = numVertexCount;
				if (rangeIdx == 0)
				{
					m_vertexCount += vertexCount % m_updateRanges.size();
				}
				offset = m_vertexOffset + m_vertexCount;
			}
		}
	}

	void PMXModel::Update(const UpdateRange & range)
	{
		const auto* position = m_positions.data() + range.m_vertexOffset;
		const auto* normal = m_normals.data() + range.m_vertexOffset;
		const auto* uv = m_uvs.data() + range.m_vertexOffset;
		const auto* morphPos = m_morphPositions.data() + range.m_vertexOffset;
		const auto* morphUV = m_morphUVs.data() + range.m_vertexOffset;
		const auto* vtxInfo = m_vertexBoneInfos.data() + range.m_vertexOffset;
		const auto* transforms = m_transforms.data();
		auto* updatePosition = m_updatePositions.data() + range.m_vertexOffset;
		auto* updateNormal = m_updateNormals.data() + range.m_vertexOffset;
		auto* updateUV = m_updateUVs.data() + range.m_vertexOffset;

		for (size_t i = 0; i < range.m_vertexCount; i++)
		{
			glm::mat4 m;
			switch (vtxInfo->m_skinningType)
			{
			case SkinningType::Weight1:
			{
				const auto i0 = vtxInfo->m_boneIndex[0];
				const auto& m0 = transforms[i0];
				m = m0;
				break;
			}
			case SkinningType::Weight2:
			{
				const auto i0 = vtxInfo->m_boneIndex[0];
				const auto i1 = vtxInfo->m_boneIndex[1];
				const auto w0 = vtxInfo->m_boneWeight[0];
				const auto w1 = vtxInfo->m_boneWeight[1];
				const auto& m0 = transforms[i0];
				const auto& m1 = transforms[i1];
				m = m0 * w0 + m1 * w1;
				break;
			}
			case SkinningType::Weight4:
			{
				const auto i0 = vtxInfo->m_boneIndex[0];
				const auto i1 = vtxInfo->m_boneIndex[1];
				const auto i2 = vtxInfo->m_boneIndex[2];
				const auto i3 = vtxInfo->m_boneIndex[3];
				const auto w0 = vtxInfo->m_boneWeight[0];
				const auto w1 = vtxInfo->m_boneWeight[1];
				const auto w2 = vtxInfo->m_boneWeight[2];
				const auto w3 = vtxInfo->m_boneWeight[3];
				const auto& m0 = transforms[i0];
				const auto& m1 = transforms[i1];
				const auto& m2 = transforms[i2];
				const auto& m3 = transforms[i3];
				m = m0 * w0 + m1 * w1 + m2 * w2 + m3 * w3;
				break;
			}
			case SkinningType::SDEF:
			{
				// https://github.com/powroupi/blender_mmd_tools/blob/dev_test/mmd_tools/core/sdef.py

				auto& nodes = *m_nodeMan.GetNodes();
				const auto i0 = vtxInfo->m_sdef.m_boneIndex[0];
				const auto i1 = vtxInfo->m_sdef.m_boneIndex[1];
				const auto w0 = vtxInfo->m_sdef.m_boneWeight;
				const auto w1 = 1.0f - w0;
				const auto center = vtxInfo->m_sdef.m_sdefC;
				const auto cr0 = vtxInfo->m_sdef.m_sdefR0;
				const auto cr1 = vtxInfo->m_sdef.m_sdefR1;
				const auto q0 = quat_cast(nodes[i0]->GetGlobalTransform());
				const auto q1 = quat_cast(nodes[i1]->GetGlobalTransform());
				const auto m0 = transforms[i0];
				const auto m1 = transforms[i1];

				const auto pos = *position + *morphPos;
				const auto rot_mat = mat3_cast(slerp(q0, q1, w1));

				*updatePosition = glm::mat3(rot_mat) * (pos - center) + glm::vec3(m0 * glm::vec4(cr0, 1)) * w0 + glm::vec3(m1 * glm::vec4(cr1, 1)) * w1;
				*updateNormal = rot_mat * *normal;

				break;
			}
			case SkinningType::DualQuaternion:
			{
				//
				// Skinning with Dual Quaternions
				// https://www.cs.utah.edu/~ladislav/dq/index.html
				//
				glm::dualquat dq[4];
				float w[4] = {};
				for (int bi = 0; bi < 4; bi++)
				{
					if (auto boneID = vtxInfo->m_boneIndex[bi]; boneID != -1)
					{ 
						dq[bi] = dualquat_cast(glm::mat3x4(transpose(transforms[boneID])));
						dq[bi] = normalize(dq[bi]);
						w[bi] = vtxInfo->m_boneWeight[bi];
					}
					else
					{
						w[bi] = 0;
					}
				}
				if (dot(dq[0].real, dq[1].real) < 0) { w[1] *= -1.0f; }
				if (dot(dq[0].real, dq[2].real) < 0) { w[2] *= -1.0f; }
				if (dot(dq[0].real, dq[3].real) < 0) { w[3] *= -1.0f; }
				auto blendDQ = w[0] * dq[0]
					+ w[1] * dq[1]
					+ w[2] * dq[2]
					+ w[3] * dq[3];
				blendDQ = normalize(blendDQ);
				m = transpose(mat3x4_cast(blendDQ));
				break;
			}
			default:
				break;
			}

			if (SkinningType::SDEF != vtxInfo->m_skinningType)
			{
				*updatePosition = glm::vec3(m * glm::vec4(*position + *morphPos, 1));
				*updateNormal = normalize(glm::mat3(m) * *normal);
			}
			*updateUV = *uv + glm::vec2(morphUV->x, morphUV->y);

			vtxInfo++;
			position++;
			normal++;
			uv++;
			updatePosition++;
			updateNormal++;
			updateUV++;
			morphPos++;
			morphUV++;
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
			m_mulMaterialFactors[matIdx].m_diffuse = m_initMaterials[matIdx].m_diffuse;
			m_mulMaterialFactors[matIdx].m_alpha = m_initMaterials[matIdx].m_alpha;
			m_mulMaterialFactors[matIdx].m_specular = m_initMaterials[matIdx].m_specular;
			m_mulMaterialFactors[matIdx].m_specularPower = m_initMaterials[matIdx].m_specularPower;
			m_mulMaterialFactors[matIdx].m_ambient = m_initMaterials[matIdx].m_ambient;

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
