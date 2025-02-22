//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_VPDFILE_H_
#define SABA_MODEL_MMD_VPDFILE_H_

#include <vector>
#include <string>

#include <glm/gtc/quaternion.hpp>

namespace saba
{
	/**
	 * @brief Represents a bone in a VPD file.
	 */
	struct VPDBone
	{
		std::string	m_boneName; ///< Bone name
		glm::vec3	m_translate; ///< Translation vector
		glm::quat	m_quaternion; ///< Rotation quaternion
	};

	/**
	 * @brief Represents a morph in a VPD file.
	 */
	struct VPDMorph
	{
		std::string	m_morphName; ///< Morph name
		float		m_weight; ///< Weight value
	};

	/**
	 * @brief Represents a VPD file.
	 */
	struct VPDFile
	{
		std::vector<VPDBone>	m_bones; ///< List of bones
		std::vector<VPDMorph>	m_morphs; ///< List of morphs
	};

	/**
	 * @brief Reads a VPD file from the specified filename.
	 * @param vpd Pointer to the VPDFile structure to store the read data.
	 * @param filename The name of the file to read.
	 * @return True if the file was read successfully, false otherwise.
	 */
	bool ReadVPDFile(VPDFile* vpd, const char* filename);
}

#endif // !SABA_MODEL_MMD_VPDFILE_H_
