//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_MMDMODEL_H_
#define SABA_MODEL_MMD_MMDMODEL_H_

#include "MMDNode.h"
#include "MMDIkSolver.h"
#include "MMDMorph.h"

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <memory>

namespace saba
{
	struct MMDMaterial;
	class MMDPhysics;
	class MMDRigidBody;
	class MMDJoint;
	struct VPDFile;

	/**
	 * @brief Manages MMD nodes.
	 */
	class MMDNodeManager
	{
	public:
		virtual ~MMDNodeManager() = default;
		static constexpr size_t NPos = -1;

		/**
		 * @brief Get the number of nodes.
		 * @return The number of nodes.
		 */
		virtual size_t GetNodeCount() = 0;

		/**
		 * @brief Find the index of a node by name.
		 * @param name The name of the node.
		 * @return The index of the node, or NPos if not found.
		 */
		virtual size_t FindNodeIndex(const std::string& name) = 0;

		/**
		 * @brief Get a node by index.
		 * @param idx The index of the node.
		 * @return A pointer to the node.
		 */
		virtual MMDNode* GetMMDNode(size_t idx) = 0;

		/**
		 * @brief Get a node by name.
		 * @param nodeName The name of the node.
		 * @return A pointer to the node, or nullptr if not found.
		 */
		MMDNode* GetMMDNode(const std::string& nodeName)
		{
			const auto findIdx = FindNodeIndex(nodeName);
			if (findIdx == NPos)
			{
				return nullptr;
			}
			return GetMMDNode(findIdx);
		}
	};

	/**
	 * @brief Manages MMD IK solvers.
	 */
	class MMDIKManager
	{
	public:
		virtual ~MMDIKManager() = default;
		static constexpr size_t NPos = -1;

		/**
		 * @brief Get the number of IK solvers.
		 * @return The number of IK solvers.
		 */
		virtual size_t GetIKSolverCount() = 0;

		/**
		 * @brief Find the index of an IK solver by name.
		 * @param name The name of the IK solver.
		 * @return The index of the IK solver, or NPos if not found.
		 */
		virtual size_t FindIKSolverIndex(const std::string& name) = 0;

		/**
		 * @brief Get an IK solver by index.
		 * @param idx The index of the IK solver.
		 * @return A pointer to the IK solver.
		 */
		virtual MMDIkSolver* GetMMDIKSolver(size_t idx) = 0;

		/**
		 * @brief Get an IK solver by name.
		 * @param ikName The name of the IK solver.
		 * @return A pointer to the IK solver, or nullptr if not found.
		 */
		MMDIkSolver* GetMMDIKSolver(const std::string& ikName)
		{
			const auto findIdx = FindIKSolverIndex(ikName);
			if (findIdx == NPos)
			{
				return nullptr;
			}
			return GetMMDIKSolver(findIdx);
		}
	};

	/**
	 * @brief Manages MMD morphs.
	 */
	class MMDMorphManager
	{
	public:
		virtual ~MMDMorphManager() = default;
		static constexpr size_t NPos = -1;

		/**
		 * @brief Get the number of morphs.
		 * @return The number of morphs.
		 */
		virtual size_t GetMorphCount() = 0;

		/**
		 * @brief Find the index of a morph by name.
		 * @param name The name of the morph.
		 * @return The index of the morph, or NPos if not found.
		 */
		virtual size_t FindMorphIndex(const std::string& name) = 0;

		/**
		 * @brief Get a morph by index.
		 * @param idx The index of the morph.
		 * @return A pointer to the morph.
		 */
		virtual MMDMorph* GetMorph(size_t idx) = 0;

		/**
		 * @brief Get a morph by name.
		 * @param name The name of the morph.
		 * @return A pointer to the morph, or nullptr if not found.
		 */
		MMDMorph* GetMorph(const std::string& name)
		{
			const auto findIdx = FindMorphIndex(name);
			if (findIdx == NPos)
			{
				return nullptr;
			}
			return GetMorph(findIdx);
		}
	};

	/**
	 * @brief Manages MMD physics.
	 */
	class MMDPhysicsManager
	{
	public:
		using RigidBodyPtr = std::unique_ptr<MMDRigidBody>;
		using JointPtr = std::unique_ptr<MMDJoint>;

		MMDPhysicsManager();
		~MMDPhysicsManager();

		/**
		 * @brief Create the physics manager.
		 * @return True if successful, false otherwise.
		 */
		bool Create();

		/**
		 * @brief Get the MMD physics object.
		 * @return A pointer to the MMD physics object.
		 */
		MMDPhysics* GetMMDPhysics() const;

		/**
		 * @brief Add a rigid body.
		 * @return A pointer to the added rigid body.
		 */
		MMDRigidBody* AddRigidBody();

		/**
		 * @brief Get the list of rigid bodies.
		 * @return A pointer to the vector of rigid bodies.
		 */
		std::vector<RigidBodyPtr>* GetRigidBodys() { return &m_rigidBodys; }

		/**
		 * @brief Add a joint.
		 * @return A pointer to the added joint.
		 */
		MMDJoint* AddJoint();

		/**
		 * @brief Get the list of joints.
		 * @return A pointer to the vector of joints.
		 */
		std::vector<JointPtr>* GetJoints() { return &m_joints; }

	private:
		std::unique_ptr<MMDPhysics>	m_mmdPhysics;
		std::vector<RigidBodyPtr>	m_rigidBodys;
		std::vector<JointPtr>		m_joints;
	};

	/**
	 * @brief Represents a sub-mesh in an MMD model.
	 */
	struct MMDSubMesh
	{
		int	m_beginIndex;
		int	m_vertexCount;
		int	m_materialID;

		/**
		 * @brief Construct a new MMDSubMesh object.
		 * @param beginIndex The beginning index of the sub-mesh.
		 * @param vertexCount The number of vertices in the sub-mesh.
		 * @param materialID The material ID of the sub-mesh.
		 */
		explicit MMDSubMesh(
			const int beginIndex = 0,
			const int vertexCount = 0,
			const int materialID = 0)
		: m_beginIndex(beginIndex),
		  m_vertexCount(vertexCount),
		  m_materialID(materialID)
	{
	}
	};

	class VMDAnimation;

	/**
	 * @brief Represents an MMD model.
	 */
	class MMDModel
	{
	public:
		virtual ~MMDModel() = default;

		/**
		 * @brief Get the node manager.
		 * @return A pointer to the node manager.
		 */
		virtual MMDNodeManager* GetNodeManager() = 0;

		/**
		 * @brief Get the IK manager.
		 * @return A pointer to the IK manager.
		 */
		virtual MMDIKManager* GetIKManager() = 0;

		/**
		 * @brief Get the morph manager.
		 * @return A pointer to the morph manager.
		 */
		virtual MMDMorphManager* GetMorphManager() = 0;

		/**
		 * @brief Get the physics manager.
		 * @return A pointer to the physics manager.
		 */
		virtual MMDPhysicsManager* GetPhysicsManager() = 0;

		/**
		 * @brief Get the number of vertices.
		 * @return The number of vertices.
		 */
		virtual size_t GetVertexCount() const = 0;

		/**
		 * @brief Get the positions of the vertices.
		 * @return A pointer to the positions of the vertices.
		 */
		virtual const glm::vec3* GetPositions() const = 0;

		/**
		 * @brief Get the normals of the vertices.
		 * @return A pointer to the normals of the vertices.
		 */
		virtual const glm::vec3* GetNormals() const = 0;

		/**
		 * @brief Get the UV coordinates of the vertices.
		 * @return A pointer to the UV coordinates of the vertices.
		 */
		virtual const glm::vec2* GetUVs() const = 0;

		/**
		 * @brief Get the updated positions of the vertices.
		 * @return A pointer to the updated positions of the vertices.
		 */
		virtual const glm::vec3* GetUpdatePositions() const = 0;

		/**
		 * @brief Get the updated normals of the vertices.
		 * @return A pointer to the updated normals of the vertices.
		 */
		virtual const glm::vec3* GetUpdateNormals() const = 0;

		/**
		 * @brief Get the updated UV coordinates of the vertices.
		 * @return A pointer to the updated UV coordinates of the vertices.
		 */
		virtual const glm::vec2* GetUpdateUVs() const = 0;

		/**
		 * @brief Get the size of the index elements.
		 * @return The size of the index elements.
		 */
		virtual size_t GetIndexElementSize() const = 0;

		/**
		 * @brief Get the number of indices.
		 * @return The number of indices.
		 */
		virtual size_t GetIndexCount() const = 0;

		/**
		 * @brief Get the indices.
		 * @return A pointer to the indices.
		 */
		virtual const void* GetIndices() const = 0;

		/**
		 * @brief Get the number of materials.
		 * @return The number of materials.
		 */
		virtual size_t GetMaterialCount() const = 0;

		/**
		 * @brief Get the materials.
		 * @return A pointer to the materials.
		 */
		virtual const MMDMaterial* GetMaterials() const = 0;

		/**
		 * @brief Get the number of sub-meshes.
		 * @return The number of sub-meshes.
		 */
		virtual size_t GetSubMeshCount() const = 0;

		/**
		 * @brief Get the sub-meshes.
		 * @return A pointer to the sub-meshes.
		 */
		virtual const MMDSubMesh* GetSubMeshes() const = 0;

		/**
		 * @brief Get the MMD physics object.
		 * @return A pointer to the MMD physics object.
		 */
		virtual MMDPhysics* GetMMDPhysics() = 0;

		/**
		 * @brief Initialize the animation.
		 */
		virtual void InitializeAnimation() = 0;

		/**
		 * @brief Save the base animation state.
		 */
		void SaveBaseAnimation();

		/**
		 * @brief Load the base animation state.
		 */
		void LoadBaseAnimation();

		/**
		 * @brief Clear the base animation state.
		 */
		void ClearBaseAnimation();

		/**
		 * @brief Begin the animation.
		 */
		virtual void BeginAnimation() = 0;

		/**
		 * @brief End the animation.
		 */
		virtual void EndAnimation() = 0;

		/**
		 * @brief Update the morph animation.
		 */
		virtual void UpdateMorphAnimation() = 0;

		/**
		 * @brief Update the node animation.
		 * @param afterPhysicsAnim Whether to update after physics animation.
		 */
		virtual void UpdateNodeAnimation(bool afterPhysicsAnim) = 0;

		/**
		 * @brief Reset the physics.
		 */
		virtual void ResetPhysics() = 0;

		/**
		 * @brief Update the physics animation.
		 * @param elapsed The elapsed time.
		 */
		virtual void UpdatePhysicsAnimation(float elapsed) = 0;

		/**
		 * @brief Update the vertices.
		 */
		virtual void Update() = 0;

		/**
		 * @brief Set the parallel update hint.
		 * @param parallelCount The number of parallel updates.
		 */
		virtual void SetParallelUpdateHint(uint32_t parallelCount) = 0;

		/**
		 * @brief Load a PMX/PMD model from the specified file path.
		 * @param filepath The path to the PMX/PMD file.
		 * @param mmdDataDir The directory containing MMD data.
		 * @return True if the model was loaded successfully, false otherwise.
		 */
		virtual bool Load(const std::string& filepath, const std::string& mmdDataDir) = 0;

		/**
		 * @brief Update all animations.
		 * @param vmdAnim The VMD animation.
		 * @param vmdFrame The VMD frame.
		 * @param physicsElapsed The elapsed time for physics.
		 */
		void UpdateAllAnimation(const VMDAnimation* vmdAnim, float vmdFrame, float physicsElapsed);

		/**
		 * @brief Load a pose from a VPD file.
		 * @param vpd The VPD file.
		 * @param frameCount The number of frames to load.
		 */
		void LoadPose(const VPDFile& vpd, int frameCount = 30);

	protected:
		template <typename NodeType>
		class MMDNodeManagerT final : public MMDNodeManager
		{
		public:
			using NodePtr = std::unique_ptr<NodeType>;

			/**
			 * @brief Get the number of nodes.
			 * @return The number of nodes.
			 */
			size_t GetNodeCount() override { return m_nodes.size(); }

			/**
			 * @brief Find the index of a node by name.
			 * @param name The name of the node.
			 * @return The index of the node, or NPos if not found.
			 */
			size_t FindNodeIndex(const std::string& name) override
			{
				auto findIt = std::find_if(
					m_nodes.begin(),
					m_nodes.end(),
					[&name](const NodePtr& node) { return node->GetName() == name; }
				);
				if (findIt == m_nodes.end())
				{
					return NPos;
				}
				return findIt - m_nodes.begin();
			}

			/**
			 * @brief Get a node by index.
			 * @param idx The index of the node.
			 * @return A pointer to the node.
			 */
			MMDNode* GetMMDNode(size_t idx) override
			{
				return m_nodes[idx].get();
			}

			/**
			 * @brief Add a node.
			 * @return A pointer to the added node.
			 */
			NodeType* AddNode()
			{
				auto node = std::make_unique<NodeType>();
				node->SetIndex(static_cast<uint32_t>(m_nodes.size()));
				m_nodes.emplace_back(std::move(node));
				return m_nodes[m_nodes.size() - 1].get();
			}

			/**
			 * @brief Get a node by index.
			 * @param i The index of the node.
			 * @return A pointer to the node.
			 */
			NodeType* GetNode(size_t i)
			{
				return m_nodes[i].get();
			}

			/**
			 * @brief Get the list of nodes.
			 * @return A pointer to the vector of nodes.
			 */
			std::vector<NodePtr>* GetNodes()
			{
				return &m_nodes;
			}

		private:
			std::vector<NodePtr>	m_nodes;
		};

		template <typename IKSolverType>
		class MMDIKManagerT final : public MMDIKManager
		{
		public:
			using IKSolverPtr = std::unique_ptr<IKSolverType>;

			/**
			 * @brief Get the number of IK solvers.
			 * @return The number of IK solvers.
			 */
			size_t GetIKSolverCount() override { return m_ikSolvers.size(); }

			/**
			 * @brief Find the index of an IK solver by name.
			 * @param name The name of the IK solver.
			 * @return The index of the IK solver, or NPos if not found.
			 */
			size_t FindIKSolverIndex(const std::string& name) override
			{
				auto findIt = std::find_if(
					m_ikSolvers.begin(),
					m_ikSolvers.end(),
					[&name](const IKSolverPtr& ikSolver) { return ikSolver->GetName() == name; }
				);
				if (findIt == m_ikSolvers.end())
				{
					return NPos;
				}
				return findIt - m_ikSolvers.begin();
			}

			/**
			 * @brief Get an IK solver by index.
			 * @param idx The index of the IK solver.
			 * @return A pointer to the IK solver.
			 */
			MMDIkSolver* GetMMDIKSolver(size_t idx) override
			{
				return m_ikSolvers[idx].get();
			}

			/**
			 * @brief Add an IK solver.
			 * @return A pointer to the added IK solver.
			 */
			IKSolverType* AddIKSolver()
			{
				m_ikSolvers.emplace_back(std::make_unique<IKSolverType>());
				return m_ikSolvers[m_ikSolvers.size() - 1].get();
			}

			/**
			 * @brief Get an IK solver by index.
			 * @param i The index of the IK solver.
			 * @return A pointer to the IK solver.
			 */
			IKSolverType* GetIKSolver(size_t i)
			{
				return m_ikSolvers[i].get();
			}

			/**
			 * @brief Get the list of IK solvers.
			 * @return A pointer to the vector of IK solvers.
			 */
			std::vector<IKSolverPtr>* GetIKSolvers()
			{
				return &m_ikSolvers;
			}

		private:
			std::vector<IKSolverPtr>	m_ikSolvers;
		};

		template <typename MorphType>
		class MMDMorphManagerT final : public MMDMorphManager
		{
		public:
			using MorphPtr = std::unique_ptr<MorphType>;

			/**
			 * @brief Get the number of morphs.
			 * @return The number of morphs.
			 */
			size_t GetMorphCount() override { return m_morphs.size(); }

			/**
			 * @brief Find the index of a morph by name.
			 * @param name The name of the morph.
			 * @return The index of the morph, or NPos if not found.
			 */
			size_t FindMorphIndex(const std::string& name) override
			{
				auto findIt = std::find_if(
					m_morphs.begin(),
					m_morphs.end(),
					[&name](const MorphPtr& morph) { return morph->GetName() == name; }
				);
				if (findIt == m_morphs.end())
				{
					return NPos;
				}
				return findIt - m_morphs.begin();
			}

			/**
			 * @brief Get a morph by index.
			 * @param idx The index of the morph.
			 * @return A pointer to the morph.
			 */
			MMDMorph* GetMorph(size_t idx) override
			{
				return m_morphs[idx].get();
			}

			/**
			 * @brief Add a morph.
			 * @return A pointer to the added morph.
			 */
			MorphType* AddMorph()
			{
				m_morphs.emplace_back(std::make_unique<MorphType>());
				return m_morphs[m_morphs.size() - 1].get();
			}

			/**
			 * @brief Get the list of morphs.
			 * @return A pointer to the vector of morphs.
			 */
			std::vector<MorphPtr>* GetMorphs()
			{
				return &m_morphs;
			}

		private:
			std::vector<MorphPtr>	m_morphs;
		};
	};
}

#endif // !SABA_MODEL_MMD_MMDMODEL_H_
