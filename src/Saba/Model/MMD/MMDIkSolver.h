//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_MMDIKSOLVER_H_
#define	SABA_MODEL_MMD_MMDIKSOLVER_H_

#include "MMDNode.h"

#include <vector>
#include <string>

namespace saba
{
	/**
	 * @brief Represents an IK solver in MMD.
	 */
	class MMDIkSolver final
	{
	public:
		MMDIkSolver();

		/**
		 * @brief Set the IK node.
		 * @param node Pointer to the IK node.
		 */
		void SetIKNode(MMDNode* node) { m_ikNode = node; }

		/**
		 * @brief Set the target node.
		 * @param node Pointer to the target node.
		 */
		void SetTargetNode(MMDNode* node) { m_ikTarget = node; }

		/**
		 * @brief Get the IK node.
		 * @return Pointer to the IK node.
		 */
		MMDNode* GetIKNode() const { return m_ikNode; }

		/**
		 * @brief Get the target node.
		 * @return Pointer to the target node.
		 */
		MMDNode* GetTargetNode() const { return m_ikTarget; }

		/**
		 * @brief Get the name of the IK node.
		 * @return Name of the IK node.
		 */
		std::string GetName() const
		{
			if (m_ikNode != nullptr)
			{
				return m_ikNode->GetName();
			}
			return "";
		}

		/**
		 * @brief Set the iterate count.
		 * @param count Iterate count.
		 */
		void SetIterateCount(const uint32_t count) { m_iterateCount = count; }

		/**
		 * @brief Set the limit angle.
		 * @param angle Limit angle.
		 */
		void SetLimitAngle(const float angle) { m_limitAngle = angle; }

		/**
		 * @brief Enable or disable the IK solver.
		 * @param enable Enable flag.
		 */
		void Enable(const bool enable) { m_enable = enable; }

		/**
		 * @brief Check if the IK solver is enabled.
		 * @return True if enabled, false otherwise.
		 */
		bool Enabled() const { return m_enable; }

		/**
		 * @brief Add an IK chain.
		 * @param node Pointer to the node.
		 * @param isKnee Whether the node is a knee.
		 */
		void AddIKChain(MMDNode* node, bool isKnee = false);

		/**
		 * @brief Add an IK chain with axis limit.
		 * @param node Pointer to the node.
		 * @param axisLimit Whether to enable axis limit.
		 * @param limitMin Minimum limit.
		 * @param limitMax Maximum limit.
		 */
		void AddIKChain(
			MMDNode* node,
			bool axisLimit,
			const glm::vec3& limitMin,
			const glm::vec3& limitMax
		);

		/**
		 * @brief Solve the IK.
		 */
		void Solve();

		/**
		 * @brief Save the base animation state.
		 */
		void SaveBaseAnimation() { m_baseAnimEnable = m_enable; }

		/**
		 * @brief Load the base animation state.
		 */
		void LoadBaseAnimation() { m_enable = m_baseAnimEnable; }

		/**
		 * @brief Clear the base animation state.
		 */
		void ClearBaseAnimation() { m_baseAnimEnable = true; }

		/**
		 * @brief Check if the base animation is enabled.
		 * @return True if enabled, false otherwise.
		 */
		bool GetBaseAnimationEnabled() const { return m_baseAnimEnable; }

	private:
		struct IKChain
		{
			MMDNode*	m_node; ///< Pointer to the node
			bool		m_enableAxisLimit; ///< Axis limit flag
			glm::vec3	m_limitMax; ///< Maximum limit
			glm::vec3	m_limitMin; ///< Minimum limit
			glm::vec3	m_prevAngle; ///< Previous angle
			glm::quat	m_saveIKRot; ///< Saved IK rotation
			float		m_planeModeAngle; ///< Plane mode angle

			IKChain(
				MMDNode* node = nullptr,
				const bool enableAxisLimit = false,
				const glm::vec3& limitMax = glm::vec3(0.0f),
				const glm::vec3& limitMin = glm::vec3(0.0f),
				const glm::quat& saveIKRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
				const glm::vec3& prevAngle = glm::vec3(0.0f),
				const float planeModeAngle = 0.0f
			)
				: m_node(node),
				  m_enableAxisLimit(enableAxisLimit),
				  m_limitMax(limitMax),
				  m_limitMin(limitMin),
				  m_prevAngle(prevAngle),
				  m_saveIKRot(saveIKRot),
				  m_planeModeAngle(planeModeAngle)
			{
			}
		};

		void SolveCore(uint32_t iteration);

		enum class SolveAxis {
			X, ///< X axis
			Y, ///< Y axis
			Z, ///< Z axis
		};

		void SolvePlane(uint32_t iteration, size_t chainIdx, SolveAxis solveAxis);

		std::vector<IKChain>	m_chains; ///< List of IK chains
		MMDNode*	m_ikNode; ///< Pointer to the IK node
		MMDNode*	m_ikTarget; ///< Pointer to the target node
		uint32_t	m_iterateCount; ///< Iterate count
		float		m_limitAngle; ///< Limit angle
		bool		m_enable; ///< Enable flag
		bool		m_baseAnimEnable; ///< Base animation enable flag
	};
}

#endif // !SABA_MODEL_MMD_MMDIKSOLVER_H_
