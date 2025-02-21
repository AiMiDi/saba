//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "VMDFile.h"

#include <Saba/Base/Log.h>
#include <Saba/Base/File.h>

namespace saba
{
	namespace
	{
		template <typename T>
		bool Read(T* val, File& file)
		{
			return file.Read(val);
		}

		template <typename T>
		bool Write(const T* val, File& file)
		{
			return file.Write(val);
		}

		bool ReadHeader(VMDFile* vmd, File& file)
		{
			Read(&vmd->m_header.m_header, file);
			Read(&vmd->m_header.m_modelName, file);

			if (vmd->m_header.m_header.ToString() != "Vocaloid Motion Data 0002" &&
				vmd->m_header.m_header.ToString() != "Vocaloid Motion Data"
				)
			{
				SABA_WARN("VMD Header error.");
				return false;
			}

			return !file.IsBad();
		}

		bool WriteHeader(const VMDFile* vmd, File& file)
		{
			Write(&vmd->m_header.m_header, file);
			Write(&vmd->m_header.m_modelName, file);
			return !file.IsBad();
		}

		bool ReadMotion(VMDFile* vmd, File& file)
		{
			uint32_t motionCount = 0;
			if (!Read(&motionCount, file))
			{
				return false;
			}

			vmd->m_motions.resize(motionCount);
			for (auto& [m_boneName, m_frame, m_translate, m_quaternion, m_interpolation] : vmd->m_motions)
			{
				Read(&m_boneName, file);
				Read(&m_frame, file);
				Read(&m_translate, file);
				Read(&m_quaternion, file);
				Read(&m_interpolation, file);
			}

			return !file.IsBad();
		}

		bool WriteMotion(const VMDFile* vmd, File& file)
		{
			uint32_t motionCount = static_cast<uint32_t>(vmd->m_motions.size());
			if (!Write(&motionCount, file))
			{
				return false;
			}

			for (const auto& motion : vmd->m_motions)
			{
				Write(&motion.m_boneName, file);
				Write(&motion.m_frame, file);
				Write(&motion.m_translate, file);
				Write(&motion.m_quaternion, file);
				Write(&motion.m_interpolation, file);
			}

			return !file.IsBad();
		}

		bool ReadBlendShape(VMDFile* vmd, File& file)
		{
			uint32_t blendShapeCount = 0;
			if (!Read(&blendShapeCount, file))
			{
				return false;
			}

			vmd->m_morphs.resize(blendShapeCount);
			for (auto& [m_blendShapeName, m_frame, m_weight] : vmd->m_morphs)
			{
				Read(&m_blendShapeName, file);
				Read(&m_frame, file);
				Read(&m_weight, file);
			}

			return !file.IsBad();
		}

		bool WriteBlendShape(const VMDFile* vmd, File& file)
		{
			uint32_t blendShapeCount = static_cast<uint32_t>(vmd->m_morphs.size());
			if (!Write(&blendShapeCount, file))
			{
				return false;
			}

			for (const auto& morph : vmd->m_morphs)
			{
				Write(&morph.m_blendShapeName, file);
				Write(&morph.m_frame, file);
				Write(&morph.m_weight, file);
			}

			return !file.IsBad();
		}

		bool ReadCamera(VMDFile* vmd, File& file)
		{
			uint32_t cameraCount = 0;
			if (!Read(&cameraCount, file))
			{
				return false;
			}

			vmd->m_cameras.resize(cameraCount);
			for (auto& [m_frame, m_distance, m_interest, m_rotate, m_interpolation, m_viewAngle, m_isPerspective] : vmd->m_cameras)
			{
				Read(&m_frame, file);
				Read(&m_distance, file);
				Read(&m_interest, file);
				Read(&m_rotate, file);
				Read(&m_interpolation, file);
				Read(&m_viewAngle, file);
				Read(&m_isPerspective, file);
			}

			return !file.IsBad();
		}

		bool WriteCamera(const VMDFile* vmd, File& file)
		{
			uint32_t cameraCount = static_cast<uint32_t>(vmd->m_cameras.size());
			if (!Write(&cameraCount, file))
			{
				return false;
			}

			for (const auto& camera : vmd->m_cameras)
			{
				Write(&camera.m_frame, file);
				Write(&camera.m_distance, file);
				Write(&camera.m_interest, file);
				Write(&camera.m_rotate, file);
				Write(&camera.m_interpolation, file);
				Write(&camera.m_viewAngle, file);
				Write(&camera.m_isPerspective, file);
			}

			return !file.IsBad();
		}

		bool ReadLight(VMDFile* vmd, File& file)
		{
			uint32_t lightCount = 0;
			if (!Read(&lightCount, file))
			{
				return false;
			}

			vmd->m_lights.resize(lightCount);
			for (auto& [m_frame, m_color, m_position] : vmd->m_lights)
			{
				Read(&m_frame, file);
				Read(&m_color, file);
				Read(&m_position, file);
			}

			return !file.IsBad();
		}

		bool WriteLight(const VMDFile* vmd, File& file)
		{
			uint32_t lightCount = static_cast<uint32_t>(vmd->m_lights.size());
			if (!Write(&lightCount, file))
			{
				return false;
			}

			for (const auto& light : vmd->m_lights)
			{
				Write(&light.m_frame, file);
				Write(&light.m_color, file);
				Write(&light.m_position, file);
			}

			return !file.IsBad();
		}

		bool ReadShadow(VMDFile* vmd, File& file)
		{
			uint32_t shadowCount = 0;
			if (!Read(&shadowCount, file))
			{
				return false;
			}

			vmd->m_shadows.resize(shadowCount);
			for (auto& [m_frame, m_shadowType, m_distance] : vmd->m_shadows)
			{
				Read(&m_frame, file);
				Read(&m_shadowType, file);
				Read(&m_distance, file);
			}

			return !file.IsBad();
		}

		bool WriteShadow(const VMDFile* vmd, File& file)
		{
			uint32_t shadowCount = static_cast<uint32_t>(vmd->m_shadows.size());
			if (!Write(&shadowCount, file))
			{
				return false;
			}

			for (const auto& shadow : vmd->m_shadows)
			{
				Write(&shadow.m_frame, file);
				Write(&shadow.m_shadowType, file);
				Write(&shadow.m_distance, file);
			}

			return !file.IsBad();
		}

		bool ReadIK(VMDFile* vmd, File& file)
		{
			uint32_t ikCount = 0;
			if (!Read(&ikCount, file))
			{
				return false;
			}

			vmd->m_iks.resize(ikCount);
			for (auto& [m_frame, m_show, m_ikInfos] : vmd->m_iks)
			{
				Read(&m_frame, file);
				Read(&m_show, file);
				uint32_t ikInfoCount = 0;
				if (!Read(&ikInfoCount, file))
				{
					return false;
				}
				m_ikInfos.resize(ikInfoCount);
				for (auto& [m_name, m_enable] : m_ikInfos)
				{
					Read(&m_name, file);
					Read(&m_enable, file);
				}
			}

			return !file.IsBad();
		}

		bool WriteIK(const VMDFile* vmd, File& file)
		{
			uint32_t ikCount = static_cast<uint32_t>(vmd->m_iks.size());
			if (!Write(&ikCount, file))
			{
				return false;
			}

			for (const auto& ik : vmd->m_iks)
			{
				Write(&ik.m_frame, file);
				Write(&ik.m_show, file);
				uint32_t ikInfoCount = static_cast<uint32_t>(ik.m_ikInfos.size());
				if (!Write(&ikInfoCount, file))
				{
					return false;
				}
				for (const auto& ikInfo : ik.m_ikInfos)
				{
					Write(&ikInfo.m_name, file);
					Write(&ikInfo.m_enable, file);
				}
			}

			return !file.IsBad();
		}

		bool ReadVMDFile(VMDFile* vmd, File& file)
		{
			if (!ReadHeader(vmd, file))
			{
				SABA_WARN("ReadHeader Fail.");
				return false;
			}

			if (!ReadMotion(vmd, file))
			{
				SABA_WARN("ReadMotion Fail.");
				return false;
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadBlendShape(vmd, file))
				{
					SABA_WARN("ReadBlednShape Fail.");
					return false;
				}
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadCamera(vmd, file))
				{
					SABA_WARN("ReadCamera Fail.");
					return false;
				}
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadLight(vmd, file))
				{
					SABA_WARN("ReadLight Fail.");
					return false;
				}
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadShadow(vmd, file))
				{
					SABA_WARN("ReadShadow Fail.");
					return false;
				}
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadIK(vmd, file))
				{
					SABA_WARN("ReadIK Fail.");
					return false;
				}
			}

			return true;
		}

		bool WriteVMDFile(const VMDFile* vmd, File& file)
		{
			if (!WriteHeader(vmd, file))
			{
				SABA_WARN("WriteHeader Fail.");
				return false;
			}

			if (!WriteMotion(vmd, file))
			{
				SABA_WARN("WriteMotion Fail.");
				return false;
			}

			if (!WriteBlendShape(vmd, file))
			{
				SABA_WARN("WriteBlendShape Fail.");
				return false;
			}

			if (!WriteCamera(vmd, file))
			{
				SABA_WARN("WriteCamera Fail.");
				return false;
			}

			if (!WriteLight(vmd, file))
			{
				SABA_WARN("WriteLight Fail.");
				return false;
			}

			if (!WriteShadow(vmd, file))
			{
				SABA_WARN("WriteShadow Fail.");
				return false;
			}

			if (!WriteIK(vmd, file))
			{
				SABA_WARN("WriteIK Fail.");
				return false;
			}

			return true;
		}
	}

	bool ReadVMDFile(VMDFile * vmd, const char * filename)
	{
		File file;
		if (!file.Open(filename))
		{
			SABA_WARN("VMD File Open Fail. {}", filename);
			return false;
		}

		return ReadVMDFile(vmd, file);
	}

	bool WriteVMDFile(const VMDFile* vmd, const char* filename)
	{
		File file;
		if (!file.Create(filename))
		{
			SABA_WARN("VMD File Open Fail. {}", filename);
			return false;
		}

		return WriteVMDFile(vmd, file);
	}

}
