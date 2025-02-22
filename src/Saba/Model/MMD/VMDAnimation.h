//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_VMDANIMATION_H_
#define SABA_MODEL_MMD_VMDANIMATION_H_

#include "MMDModel.h"
#include "MMDNode.h"
#include "VMDFile.h"
#include "MMDIkSolver.h"

#include <vector>
#include <algorithm>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace saba
{
	/**
	 * @brief Represents a Bezier curve used in VMD animation.
	 */
	struct VMDBezier
	{
		/**
		 * @brief Evaluate the X component of the Bezier curve at time t.
		 * @param t The time parameter.
		 * @return The evaluated X component.
		 */
		float EvalX(float t) const;

		/**
		 * @brief Evaluate the Y component of the Bezier curve at time t.
		 * @param t The time parameter.
		 * @return The evaluated Y component.
		 */
		float EvalY(float t) const;

		/**
		 * @brief Evaluate both X and Y components of the Bezier curve at time t.
		 * @param t The time parameter.
		 * @return The evaluated X and Y components as a glm::vec2.
		 */
		glm::vec2 Eval(float t) const;

		/**
		 * @brief Find the X component of the Bezier curve at a given time.
		 * @param time The time parameter.
		 * @return The X component at the given time.
		 */
		float FindBezierX(float time) const;

		glm::vec2	m_cp1; ///< Control point 1
		glm::vec2	m_cp2; ///< Control point 2
	};

	class VMDNodeController;
	class VMDMorphController;
	class VMDIKController;

	/**
	 * @brief Manages VMD animation for an MMD model.
	 */
	class VMDAnimation
	{
	public:
		VMDAnimation();

		/**
		 * @brief Create the VMD animation with the given MMD model.
		 * @param model The MMD model.
		 * @return True if creation is successful, false otherwise.
		 */
		bool Create(const std::shared_ptr<MMDModel>& model);

		/**
		 * @brief Add a VMD file to the animation.
		 * @param vmd The VMD file.
		 * @return True if addition is successful, false otherwise.
		 */
		bool Add(const VMDFile& vmd);

		/**
		 * @brief Destroy the VMD animation.
		 */
		void Destroy();

		/**
		 * @brief Evaluate the animation at time t with the given weight.
		 * @param t The time parameter.
		 * @param weight The weight parameter.
		 */
		void Evaluate(float t, float weight = 1.0f) const;

		/**
		 * @brief Synchronize physics with the animation.
		 * @param t The time parameter.
		 * @param frameCount The number of frames to synchronize over.
		 */
		void SyncPhysics(float t, int frameCount = 30) const;

		/**
		 * @brief Get the maximum key time of the animation.
		 * @return The maximum key time.
		 */
		int32_t GetMaxKeyTime() const { return static_cast<int32_t>(m_maxKeyTime); }
	private:
		int32_t CalculateMaxKeyTime() const;

		using NodeControllerPtr = std::unique_ptr<VMDNodeController>;
		using IKControllerPtr = std::unique_ptr<VMDIKController>;
		using MorphControllerPtr = std::unique_ptr<VMDMorphController>;

		std::shared_ptr<MMDModel>			m_model; ///< The MMD model
		std::vector<NodeControllerPtr>		m_nodeControllers; ///< Node controllers
		std::vector<IKControllerPtr>		m_ikControllers; ///< IK controllers
		std::vector<MorphControllerPtr>		m_morphControllers; ///< Morph controllers
		uint32_t	m_maxKeyTime; ///< Maximum key time
	};

}

#endif // !SABA_MODEL_MMD_VMDANIMATION_H_
