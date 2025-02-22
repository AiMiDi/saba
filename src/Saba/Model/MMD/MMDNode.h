//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_MMDNODE_H_
#define SABA_MODEL_MMD_MMDNODE_H_

#include <string>
#include <glm/gtc/quaternion.hpp>

namespace saba
{
	/**
	 * @brief Represents a node in an MMD model.
	 */
	class MMDNode
	{
	public:
		MMDNode();
		virtual ~MMDNode() = default;

		/**
		 * @brief Add a child node.
		 * @param child The child node to add.
		 */
		void AddChild(MMDNode* child);

		/**
		 * @brief Called before updating the transform.
		 */
		void BeginUpdateTransform();

		/**
		 * @brief Called after updating the transform.
		 */
		void EndUpdateTransform();

		/**
		 * @brief Update the local transform.
		 */
		void UpdateLocalTransform();

		/**
		 * @brief Update the global transform.
		 */
		void UpdateGlobalTransform();

		/**
		 * @brief Update the transform of child nodes.
		 */
		void UpdateChildTransform() const;

		/**
		 * @brief Set the index of the node.
		 * @param idx The index of the node.
		 */
		void SetIndex(const uint32_t idx) { m_index = idx; }

		/**
		 * @brief Get the index of the node.
		 * @return The index of the node.
		 */
		uint32_t GetIndex() const { return m_index; }

		/**
		 * @brief Set the name of the node.
		 * @param name The name of the node.
		 */
		void SetName(const std::string& name) { m_name = name; }

		/**
		 * @brief Get the name of the node.
		 * @return The name of the node.
		 */
		const std::string& GetName() const { return m_name; }

		/**
		 * @brief Enable or disable IK for the node.
		 * @param enable Whether to enable IK.
		 */
		void EnableIK(const bool enable) { m_enableIK = enable; }

		/**
		 * @brief Check if IK is enabled for the node.
		 * @return True if IK is enabled, false otherwise.
		 */
		bool IsIK() const { return m_enableIK; }

		/**
		 * @brief Set the translation of the node.
		 * @param t The translation vector.
		 */
		void SetTranslate(const glm::vec3& t) { m_translate = t; }

		/**
		 * @brief Get the translation of the node.
		 * @return The translation vector.
		 */
		const glm::vec3& GetTranslate() const { return m_translate; }

		/**
		 * @brief Set the rotation of the node.
		 * @param r The rotation quaternion.
		 */
		void SetRotate(const glm::quat& r) { m_rotate = r; }

		/**
		 * @brief Get the rotation of the node.
		 * @return The rotation quaternion.
		 */
		const glm::quat& GetRotate() const { return m_rotate; }

		/**
		 * @brief Set the scale of the node.
		 * @param s The scale vector.
		 */
		void SetScale(const glm::vec3& s) { m_scale = s; }

		/**
		 * @brief Get the scale of the node.
		 * @return The scale vector.
		 */
		const glm::vec3& GetScale() const { return m_scale; }

		/**
		 * @brief Set the animation translation of the node.
		 * @param t The animation translation vector.
		 */
		void SetAnimationTranslate(const glm::vec3& t) { m_animTranslate = t; }

		/**
		 * @brief Get the animation translation of the node.
		 * @return The animation translation vector.
		 */
		const glm::vec3& GetAnimationTranslate() const { return m_animTranslate; }

		/**
		 * @brief Set the animation rotation of the node.
		 * @param q The animation rotation quaternion.
		 */
		void SetAnimationRotate(const glm::quat& q) { m_animRotate = q; }

		/**
		 * @brief Get the animation rotation of the node.
		 * @return The animation rotation quaternion.
		 */
		const glm::quat& GetAnimationRotate() const { return m_animRotate; }

		/**
		 * @brief Get the combined translation for animation.
		 * @return The combined translation vector.
		 */
		glm::vec3 AnimateTranslate() const { return m_animTranslate + m_translate; }

		/**
		 * @brief Get the combined rotation for animation.
		 * @return The combined rotation quaternion.
		 */
		glm::quat AnimateRotate() const { return m_animRotate * m_rotate; }

		/**
		 * @brief Set the IK rotation of the node.
		 * @param ikr The IK rotation quaternion.
		 */
		void SetIKRotate(const glm::quat& ikr) { m_ikRotate = ikr; }

		/**
		 * @brief Get the IK rotation of the node.
		 * @return The IK rotation quaternion.
		 */
		const glm::quat& GetIKRotate() const { return m_ikRotate; }

		/**
		 * @brief Get the parent node.
		 * @return A pointer to the parent node.
		 */
		MMDNode* GetParent() const { return m_parent; }

		/**
		 * @brief Get the child node.
		 * @return A pointer to the child node.
		 */
		MMDNode* GetChild() const { return m_child; }

		/**
		 * @brief Get the next sibling node.
		 * @return A pointer to the next sibling node.
		 */
		MMDNode* GetNext() const { return m_next; }

		/**
		 * @brief Get the previous sibling node.
		 * @return A pointer to the previous sibling node.
		 */
		MMDNode* GetPrev() const { return m_prev; }

		/**
		 * @brief Set the local transform matrix.
		 * @param m The local transform matrix.
		 */
		void SetLocalTransform(const glm::mat4& m) { m_local = m; }

		/**
		 * @brief Get the local transform matrix.
		 * @return The local transform matrix.
		 */
		const glm::mat4& GetLocalTransform() const { return m_local; }

		/**
		 * @brief Set the global transform matrix.
		 * @param m The global transform matrix.
		 */
		void SetGlobalTransform(const glm::mat4& m) { m_global = m; }

		/**
		 * @brief Get the global transform matrix.
		 * @return The global transform matrix.
		 */
		const glm::mat4& GetGlobalTransform() const { return m_global; }

		/**
		 * @brief Calculate the inverse of the initial transform.
		 */
		void CalculateInverseInitTransform();
		
		/**
		 * @brief Get the inverse of the initial transform.
		 * @return The inverse of the initial transform matrix.
		 */
		const glm::mat4& GetInverseInitTransform() const { return m_inverseInit; }

		/**
		 * @brief Save the initial translation, rotation, and scale.
		 */
		void SaveInitialTRS()
		{
			m_initTranslate = m_translate;
			m_initRotate = m_rotate;
			m_initScale = m_scale;
		}

		/**
		 * @brief Load the initial translation, rotation, and scale.
		 */
		void LoadInitialTRS()
		{
			m_translate = m_initTranslate;
			m_rotate = m_initRotate;
			m_scale = m_initScale;
			//m_animTranslate = glm::vec3(0);
			//m_animRotate = glm::quat(1, 0, 0, 0);
		}

		/**
		 * @brief Get the initial translation.
		 * @return The initial translation vector.
		 */
		const glm::vec3& GetInitialTranslate() const { return m_initTranslate; }

		/**
		 * @brief Get the initial rotation.
		 * @return The initial rotation quaternion.
		 */
		const glm::quat& GetInitialRotate() const { return m_initRotate; }

		/**
		 * @brief Get the initial scale.
		 * @return The initial scale vector.
		 */
		const glm::vec3& GetInitialScale() const { return m_initScale; }

		/**
		 * @brief Save the base animation state.
		 */
		void SaveBaseAnimation()
		{
			m_baseAnimTranslate = m_animTranslate;
			m_baseAnimRotate = m_animRotate;
		}

		/**
		 * @brief Load the base animation state.
		 */
		void LoadBaseAnimation()
		{
			m_animTranslate = m_baseAnimTranslate;
			m_animRotate = m_baseAnimRotate;
		}

		/**
		 * @brief Clear the base animation state.
		 */
		void ClearBaseAnimation()
		{
			m_baseAnimTranslate = glm::vec3(0);
			m_baseAnimRotate = glm::quat(1, 0, 0, 0);
		}

		/**
		 * @brief Get the base animation translation.
		 * @return The base animation translation vector.
		 */
		const glm::vec3& GetBaseAnimationTranslate() const { return m_baseAnimTranslate; }

		/**
		 * @brief Get the base animation rotation.
		 * @return The base animation rotation quaternion.
		 */
		const glm::quat& GetBaseAnimationRotate() const { return m_baseAnimRotate; }

	protected:
		/**
		 * @brief Called before updating the transform.
		 */
		virtual void OnBeginUpdateTransform();

		/**
		 * @brief Called after updating the transform.
		 */
		virtual void OnEndUpdateTransfrom();

		/**
		 * @brief Called to update the local transform.
		 */
		virtual void OnUpdateLocalTransform();

		uint32_t		m_index;
		std::string		m_name;
		bool			m_enableIK;

		MMDNode*		m_parent;
		MMDNode*		m_child;
		MMDNode*		m_next;
		MMDNode*		m_prev;

		glm::vec3	m_translate;
		glm::quat	m_rotate;
		glm::vec3	m_scale;

		glm::vec3	m_animTranslate;
		glm::quat	m_animRotate;

		glm::vec3	m_baseAnimTranslate;
		glm::quat	m_baseAnimRotate;

		glm::quat	m_ikRotate;

		glm::mat4		m_local;
		glm::mat4		m_global;
		glm::mat4		m_inverseInit;

		glm::vec3	m_initTranslate;
		glm::quat	m_initRotate;
		glm::vec3	m_initScale;
	};
}

#endif // !SABA_MODEL_MMD_MMDNODE_H_

