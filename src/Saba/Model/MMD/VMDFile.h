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

	struct VMDHeader
	{
		MMDFileString<30>	m_header;
		MMDFileString<20>	m_modelName;

		VMDHeader(
			const MMDFileString<30>& header = MMDFileString<30>(),
			const MMDFileString<20>& modelName = MMDFileString<20>()
		) : m_header(header), m_modelName(modelName) {}
	};

	struct VMDMotion
	{
		VMDString<15>	m_boneName;
		uint32_t		m_frame;
		glm::vec3		m_translate;
		glm::quat		m_quaternion;
		std::array<uint8_t, 64>	m_interpolation;

		VMDMotion(
			const VMDString<15>& boneName = VMDString<15>(),
			uint32_t frame = 0,
			const glm::vec3& translate = glm::vec3(0.0f),
			const glm::quat& quaternion = glm::quat(),
			const std::array<uint8_t, 64>& interpolation = std::array<uint8_t, 64>()
		) : m_boneName(boneName), m_frame(frame), m_translate(translate), m_quaternion(quaternion), m_interpolation(interpolation) {}
	};

	struct VMDMorph {
		VMDString<15>	m_blendShapeName;
		uint32_t		m_frame{};
		float			m_weight{};

		VMDMorph(
			const VMDString<15>& blendShapeName = VMDString<15>(),
			uint32_t frame = 0,
			float weight = 0.0f
		) : m_blendShapeName(blendShapeName), m_frame(frame), m_weight(weight) {}
	};

	struct VMDCamera
	{
		uint32_t		m_frame;
		float			m_distance;
		glm::vec3		m_interest;
		glm::vec3		m_rotate;
		std::array<uint8_t, 24>	m_interpolation;
		uint32_t		m_viewAngle;
		uint8_t			m_isPerspective;

		VMDCamera(
			uint32_t frame = 0,
			float distance = 0.0f,
			const glm::vec3& interest = glm::vec3(0.0f),
			const glm::vec3& rotate = glm::vec3(0.0f),
			uint32_t viewAngle = 0,
			uint8_t isPerspective = 0,
			const std::array<uint8_t, 24>& interpolation = std::array<uint8_t, 24>()
		) : m_frame(frame), m_distance(distance), m_interest(interest), m_rotate(rotate), m_interpolation(interpolation), m_viewAngle(viewAngle), m_isPerspective(isPerspective) {}
	};

	struct VMDLight
	{
		uint32_t	m_frame;
		glm::vec3	m_color;
		glm::vec3	m_position;

		VMDLight(
			uint32_t frame = 0,
			const glm::vec3& color = glm::vec3(0.0f),
			const glm::vec3& position = glm::vec3(0.0f)
		) : m_frame(frame), m_color(color), m_position(position) {}
	};

	struct VMDShadow
	{
		uint32_t	m_frame;
		uint8_t		m_shadowType;	// 0:Off 1:mode1 2:mode2
		float		m_distance;

		VMDShadow(
			uint32_t frame = 0,
			uint8_t shadowType = 0,
			float distance = 0.0f
		) : m_frame(frame), m_shadowType(shadowType), m_distance(distance) {}
	};

	struct VMDIkInfo
	{
		VMDString<20>	m_name;
		uint8_t			m_enable{};

		VMDIkInfo(
			const VMDString<20>& name = VMDString<20>(),
			uint8_t enable = 0
		) : m_name(name), m_enable(enable) {}
	};

	struct VMDIk
	{
		uint32_t	m_frame;
		uint8_t		m_show;
		std::vector<VMDIkInfo>	m_ikInfos;

		VMDIk(
			uint32_t frame = 0,
			uint8_t show = 0,
			const std::vector<VMDIkInfo>& ikInfos = std::vector<VMDIkInfo>()
		) : m_frame(frame), m_show(show), m_ikInfos(ikInfos) {}
	};

	struct VMDFile
	{
		VMDHeader					m_header;
		std::vector<VMDMotion>		m_motions;
		std::vector<VMDMorph>		m_morphs;
		std::vector<VMDCamera>		m_cameras;
		std::vector<VMDLight>		m_lights;
		std::vector<VMDShadow>		m_shadows;
		std::vector<VMDIk>			m_iks;

		VMDFile(
			const VMDHeader& header = VMDHeader(),
			const std::vector<VMDMotion>& motions = std::vector<VMDMotion>(),
			const std::vector<VMDMorph>& morphs = std::vector<VMDMorph>(),
			const std::vector<VMDCamera>& cameras = std::vector<VMDCamera>(),
			const std::vector<VMDLight>& lights = std::vector<VMDLight>(),
			const std::vector<VMDShadow>& shadows = std::vector<VMDShadow>(),
			const std::vector<VMDIk>& iks = std::vector<VMDIk>()
		) : m_header(header), m_motions(motions), m_morphs(morphs), m_cameras(cameras), m_lights(lights), m_shadows(shadows), m_iks(iks) {}
	};

	bool ReadVMDFile(VMDFile* vmd, const char* filename);
	bool WriteVMDFile(const VMDFile* vmd, const char* filename);
}

#endif // !SABA_MODEL_MMD_VMDFILE_H_
