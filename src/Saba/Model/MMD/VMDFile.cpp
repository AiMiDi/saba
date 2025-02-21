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

}
