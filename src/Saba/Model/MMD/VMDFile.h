//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_VMDFILE_H_
#define SABA_MODEL_MMD_VMDFILE_H_

#include "MMDFileString.h"

#include <cstdint>
#include <vector>
#include <array>

#include <glm/gtc/quaternion.hpp>

namespace saba
{
	template <size_t Size>
	using VMDString = MMDFileString<Size>;

	/**
	 * @brief Represents the header of a VMD file.
	 */
	struct VMDHeader
	{
		MMDFileString<30>	m_header; ///< Header string
		MMDFileString<20>	m_modelName; ///< Model name string

		explicit VMDHeader(
			const MMDFileString<30>& header = MMDFileString<30>(),
			const MMDFileString<20>& modelName = MMDFileString<20>()
		) : m_header(header), m_modelName(modelName) {}
	};

	/**
	 * @brief Represents a motion in a VMD file.
	 */
	struct VMDMotion
	{
		VMDString<15>	m_boneName; ///< Bone name
		uint32_t		m_frame; ///< Frame number
		glm::vec3		m_translate; ///< Translation vector
		glm::quat		m_quaternion; ///< Rotation quaternion
		std::array<uint8_t, 64>	m_interpolation; ///< Interpolation data

		explicit VMDMotion(
			const VMDString<15>& boneName = VMDString<15>(),
			const uint32_t frame = 0,
			const glm::vec3& translate = glm::vec3(0.0f),
			const glm::quat& quaternion = glm::quat(),
			const std::array<uint8_t, 64>& interpolation = std::array<uint8_t, 64>()
		) : m_boneName(boneName), m_frame(frame), m_translate(translate), m_quaternion(quaternion), m_interpolation(interpolation) {}
	};

	/**
	 * @brief Represents a morph in a VMD file.
	 */
	struct VMDMorph {
		VMDString<15>	m_blendShapeName; ///< Blend shape name
		uint32_t		m_frame{}; ///< Frame number
		float			m_weight{}; ///< Weight value

		explicit VMDMorph(
			const VMDString<15>& blendShapeName = VMDString<15>(),
			const uint32_t frame = 0,
			const float weight = 0.0f
		) : m_blendShapeName(blendShapeName), m_frame(frame), m_weight(weight) {}
	};

	/**
	 * @brief Represents a camera in a VMD file.
	 */
	struct VMDCamera
	{
		uint32_t		m_frame; ///< Frame number
		float			m_distance; ///< Distance value
		glm::vec3		m_interest; ///< Interest point
		glm::vec3		m_rotate; ///< Rotation vector
		std::array<uint8_t, 24>	m_interpolation; ///< Interpolation data
		uint32_t		m_viewAngle; ///< View angle
		uint8_t			m_isPerspective; ///< Perspective flag

		explicit VMDCamera(
			const uint32_t frame = 0,
			const float distance = 0.0f,
			const glm::vec3& interest = glm::vec3(0.0f),
			const glm::vec3& rotate = glm::vec3(0.0f),
			const uint32_t viewAngle = 0,
			const uint8_t isPerspective = 0,
			const std::array<uint8_t, 24>& interpolation = std::array<uint8_t, 24>()
		) : m_frame(frame), m_distance(distance), m_interest(interest), m_rotate(rotate), m_interpolation(interpolation), m_viewAngle(viewAngle), m_isPerspective(isPerspective) {}
	};

	/**
	 * @brief Represents a light in a VMD file.
	 */
	struct VMDLight
	{
		uint32_t	m_frame; ///< Frame number
		glm::vec3	m_color; ///< Color vector
		glm::vec3	m_position; ///< Position vector

		explicit VMDLight(
			const uint32_t frame = 0,
			const glm::vec3& color = glm::vec3(0.0f),
			const glm::vec3& position = glm::vec3(0.0f)
		) : m_frame(frame), m_color(color), m_position(position) {}
	};

	/**
	 * @brief Represents a shadow in a VMD file.
	 */
	struct VMDShadow
	{
		uint32_t	m_frame; ///< Frame number
		uint8_t		m_shadowType; ///< Shadow type (0:Off 1:mode1 2:mode2)
		float		m_distance; ///< Distance value

		explicit VMDShadow(
			const uint32_t frame = 0,
			const uint8_t shadowType = 0,
			const float distance = 0.0f
		) : m_frame(frame), m_shadowType(shadowType), m_distance(distance) {}
	};

	/**
	 * @brief Represents IK information in a VMD file.
	 */
	struct VMDIkInfo
	{
		VMDString<20>	m_name; ///< IK name
		uint8_t			m_enable{}; ///< Enable flag

		explicit VMDIkInfo(
			const VMDString<20>& name = VMDString<20>(),
			const uint8_t enable = 0
		) : m_name(name), m_enable(enable) {}
	};

	/**
	 * @brief Represents IK data in a VMD file.
	 */
	struct VMDIk
	{
		uint32_t	m_frame; ///< Frame number
		uint8_t		m_show; ///< Show flag
		std::vector<VMDIkInfo>	m_ikInfos; ///< IK information list

		explicit VMDIk(
			const uint32_t frame = 0,
			const uint8_t show = 0,
			const std::vector<VMDIkInfo>& ikInfos = std::vector<VMDIkInfo>()
		) : m_frame(frame), m_show(show), m_ikInfos(ikInfos) {}
	};

	/**
	 * @brief Represents a VMD file.
	 */
	struct VMDFile
	{
		VMDHeader					m_header; ///< VMD header
		std::vector<VMDMotion>		m_motions; ///< Motion list
		std::vector<VMDMorph>		m_morphs; ///< Morph list
		std::vector<VMDCamera>		m_cameras; ///< Camera list
		std::vector<VMDLight>		m_lights; ///< Light list
		std::vector<VMDShadow>		m_shadows; ///< Shadow list
		std::vector<VMDIk>			m_iks; ///< IK list

		explicit VMDFile(
			const VMDHeader& header = VMDHeader(),
			const std::vector<VMDMotion>& motions = std::vector<VMDMotion>(),
			const std::vector<VMDMorph>& morphs = std::vector<VMDMorph>(),
			const std::vector<VMDCamera>& cameras = std::vector<VMDCamera>(),
			const std::vector<VMDLight>& lights = std::vector<VMDLight>(),
			const std::vector<VMDShadow>& shadows = std::vector<VMDShadow>(),
			const std::vector<VMDIk>& iks = std::vector<VMDIk>()
		) : m_header(header), m_motions(motions), m_morphs(morphs), m_cameras(cameras), m_lights(lights), m_shadows(shadows), m_iks(iks) {}
	};

	/**
	 * @brief Read a VMD file from the specified filename.
	 * @param vmd Pointer to the VMDFile structure to store the read data.
	 * @param filename The name of the file to read.
	 * @return True if the file is read successfully, false otherwise.
	 */
	bool ReadVMDFile(VMDFile* vmd, const char* filename);

	/**
	 * @brief Write a VMD file to the specified filename.
	 * @param vmd Pointer to the VMDFile structure containing the data to write.
	 * @param filename The name of the file to write.
	 * @return True if the file is written successfully, false otherwise.
	 */
	bool WriteVMDFile(const VMDFile* vmd, const char* filename);
}

#endif // !SABA_MODEL_MMD_VMDFILE_H_
