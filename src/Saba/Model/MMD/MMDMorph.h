//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_MMDMORPH_H
#define SABA_MODEL_MMD_MMDMORPH_H

#include <string>

namespace saba
{
	/**
	 * @brief Represents an MMD morph.
	 */
	class MMDMorph
	{
	public:
		MMDMorph();

		/**
		 * @brief Set the name of the morph.
		 * @param name The name of the morph.
		 */
		void SetName(const std::string& name) { m_name = name; }

		/**
		 * @brief Get the name of the morph.
		 * @return The name of the morph.
		 */
		const std::string& GetName() const { return m_name; }

		/**
		 * @brief Set the weight of the morph.
		 * @param weight The weight of the morph.
		 */
		void SetWeight(const float weight) { m_weight = weight; }

		/**
		 * @brief Get the weight of the morph.
		 * @return The weight of the morph.
		 */
		float GetWeight() const { return m_weight; }

		/**
		 * @brief Save the base animation state.
		 */
		void SaveBaseAnimation() { m_saveAnimWeight = m_weight; }

		/**
		 * @brief Load the base animation state.
		 */
		void LoadBaseAnimation() { m_weight = m_saveAnimWeight; }

		/**
		 * @brief Clear the base animation state.
		 */
		void ClearBaseAnimation() { m_saveAnimWeight = 0; }

		/**
		 * @brief Get the base animation weight.
		 * @return The base animation weight.
		 */
		float GetBaseAnimationWeight() const { return m_saveAnimWeight; }

	private:
		std::string	m_name;
		float		m_weight;
		float		m_saveAnimWeight;
	};
}

#endif // !SABA_MODEL_MMD_MMDMORPH_H
