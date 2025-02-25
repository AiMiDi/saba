//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_PMDMODEL_H_
#define SABA_MODEL_MMD_PMDMODEL_H_

#include "MMDMaterial.h"
#include "MMDModel.h"

#include <vector>
#include <string>

namespace saba
{
	/**
	 * @brief Represents a PMD model.
	 */
	class PMDModel final : public MMDModel
	{
	public:
		/**
		 * @brief Get the node manager.
		 * @return Pointer to the node manager.
		 */
		MMDNodeManager* GetNodeManager() override { return &m_nodeMan; }

		/**
		 * @brief Get the IK manager.
		 * @return Pointer to the IK manager.
		 */
		MMDIKManager* GetIKManager() override { return &m_ikSolverMan; }

		/**
		 * @brief Get the morph manager.
		 * @return Pointer to the morph manager.
		 */
		MMDMorphManager* GetMorphManager() override { return &m_morphMan; }

		/**
		 * @brief Get the physics manager.
		 * @return Pointer to the physics manager.
		 */
		MMDPhysicsManager* GetPhysicsManager() override { return &m_physicsMan; }

		/**
		 * @brief Get the vertex count.
		 * @return Number of vertices.
		 */
		size_t GetVertexCount() const override { return m_positions.size(); }

		/**
		 * @brief Get the positions of vertices.
		 * @return Pointer to the positions array.
		 */
		const glm::vec3* GetPositions() const override { return &m_positions[0]; }

		/**
		 * @brief Get the normals of vertices.
		 * @return Pointer to the normals array.
		 */
		const glm::vec3* GetNormals() const override { return &m_normals[0]; }

		/**
		 * @brief Get the UV coordinates of vertices.
		 * @return Pointer to the UV coordinates array.
		 */
		const glm::vec2* GetUVs() const override { return &m_uvs[0]; }

		/**
		 * @brief Get the updated positions of vertices.
		 * @return Pointer to the updated positions array.
		 */
		const glm::vec3* GetUpdatePositions() const override { return &m_updatePositions[0]; }

		/**
		 * @brief Get the updated normals of vertices.
		 * @return Pointer to the updated normals array.
		 */
		const glm::vec3* GetUpdateNormals() const override { return &m_updateNormals[0]; }

		/**
		 * @brief Get the updated UV coordinates of vertices.
		 * @return Pointer to the updated UV coordinates array.
		 */
		const glm::vec2* GetUpdateUVs() const override { return &m_uvs[0]; }

		/**
		 * @brief Get the size of index elements.
		 * @return Size of index elements.
		 */
		size_t GetIndexElementSize() const override { return sizeof(uint16_t); }

		/**
		 * @brief Get the index count.
		 * @return Number of indices.
		 */
		size_t GetIndexCount() const override { return m_indices.size(); }

		/**
		 * @brief Get the indices.
		 * @return Pointer to the indices array.
		 */
		const void* GetIndices() const override { return &m_indices[0]; }

		/**
		 * @brief Get the material count.
		 * @return Number of materials.
		 */
		size_t GetMaterialCount() const override { return m_materials.size(); }

		/**
		 * @brief Get the materials.
		 * @return Pointer to the materials array.
		 */
		const MMDMaterial* GetMaterials() const override { return &m_materials[0]; }

		/**
		 * @brief Get the sub-mesh count.
		 * @return Number of sub-meshes.
		 */
		size_t GetSubMeshCount() const override { return m_subMeshes.size(); }

		/**
		 * @brief Get the sub-meshes.
		 * @return Pointer to the sub-meshes array.
		 */
		const MMDSubMesh* GetSubMeshes() const override { return &m_subMeshes[0]; }

		/**
		 * @brief Get the MMD physics.
		 * @return Pointer to the MMD physics.
		 */
		MMDPhysics* GetMMDPhysics() override { return m_physicsMan.GetMMDPhysics(); }

		/**
		 * @brief Initialize the animation.
		 */
		void InitializeAnimation() override;

		/**
		 * @brief Call before animation.
		 */
		void BeginAnimation() override;

		/**
		 * @brief Call after animation.
		 */
		void EndAnimation() override;

		/**
		 * @brief Update morph animation.
		 */
		void UpdateMorphAnimation() override;

		/**
		 * @brief Update node animation.
		 * @param afterPhysicsAnim Whether to update after physics animation.
		 */
		void UpdateNodeAnimation(bool afterPhysicsAnim) override;

		/**
		 * @brief Reset the physics.
		 */
		void ResetPhysics() override;

		/**
		 * @brief Update physics animation.
		 * @param elapsed Elapsed time.
		 */
		void UpdatePhysicsAnimation(float elapsed) override;

		/**
		 * @brief Update vertex data.
		 */
		void Update() override;

		/**
		 * @brief Set parallel update hint.
		 * @param hint Hint value.
		 */
		void SetParallelUpdateHint(uint32_t hint) override {}

		/**
		 * @brief Load a PMD model from the specified file path.
		 * @param filepath The path to the PMD file.
		 * @param mmdDataDir The directory containing MMD data.
		 * @return True if the model was loaded successfully, false otherwise.
		 */
		bool Load(const std::string& filepath, const std::string& mmdDataDir) override;

		/**
		 * @brief Destroy the PMD model.
		 */
		void Destroy();

		/**
		 * @brief Get the minimum bounding box of the model.
		 * @return The minimum bounding box.
		 */
		const glm::vec3& GetBBoxMin() const { return m_bboxMin; }

		/**
		 * @brief Get the maximum bounding box of the model.
		 * @return The maximum bounding box.
		 */
		const glm::vec3& GetBBoxMax() const { return m_bboxMax; }

	private:
		/**
		 * @brief Represents a morph vertex.
		 */
		struct MorphVertex;

		/**
		 * @brief Represents a PMD morph.
		 */
		class PMDMorph : public MMDMorph
		{
		public:
			std::vector<MorphVertex>	m_vertices; ///< List of morph vertices
		};

		std::vector<glm::vec3>	m_positions; ///< List of positions
		std::vector<glm::vec3>	m_normals; ///< List of normals
		std::vector<glm::vec2>	m_uvs; ///< List of UV coordinates
		std::vector<glm::ivec2>	m_bones; ///< List of bones
		std::vector<glm::vec2>	m_boneWeights; ///< List of bone weights
		std::vector<glm::vec3>	m_updatePositions; ///< List of updated positions
		std::vector<glm::vec3>	m_updateNormals; ///< List of updated normals
		std::vector<glm::mat4>	m_transforms; ///< List of transforms

		std::vector<uint16_t> m_indices; ///< List of indices

		PMDMorph					m_baseMorph; ///< Base morph

		glm::vec3		m_bboxMin = glm::vec3(0); ///< Minimum bounding box
		glm::vec3		m_bboxMax = glm::vec3(0); ///< Maximum bounding box

		std::vector<MMDMaterial>	m_materials; ///< List of materials
		std::vector<MMDSubMesh>		m_subMeshes; ///< List of sub-meshes

		MMDNodeManagerT<MMDNode>	m_nodeMan; ///< Node manager
		MMDIKManagerT<MMDIkSolver>	m_ikSolverMan; ///< IK manager
		MMDMorphManagerT<PMDMorph>	m_morphMan; ///< Morph manager
		MMDPhysicsManager			m_physicsMan; ///< Physics manager
	};
}

#endif // !SABA_MODEL_MMD_PMDMODEL_H_
