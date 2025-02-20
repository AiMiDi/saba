//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "VPDFile.h"

#include <algorithm>

#include <Saba/Base/Log.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>

#include "SjisToUnicode.h"

namespace saba
{
	bool ReadVPDFile(VPDFile * vpd, const char * filename)
	{
		
		TextFileReader file;
		if (!file.Open(filename))
		{
			SABA_INFO("VPD File Open Fail. {}", filename);
			return false;
		}

		std::vector<std::string> lines;
		file.ReadAllLines(lines);

		if (lines.empty() || lines[0] != "Vocaloid Pose Data file")
		{
			SABA_INFO("VPD File Format Error.");
			return false;
		}

		// remove comment
		for (auto it = lines.begin() + 1; it != lines.end(); ++it)
		{
			auto& line = *it;
			auto commentPos = line.find("//");
			line = line.substr(0, commentPos);
		}

		auto lineIt = lines.begin() + 1;
		// parent file name
		lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
			return !line.empty();
		});
		if (lineIt == lines.end())
		{
			SABA_INFO("VPD File Parse Error.[parent fil name]");
			return false;
		}
		++lineIt;

		// num bones
		lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
			return !line.empty();
		});
		if (lineIt == lines.end())
		{
			SABA_INFO("VPD File Parse Error.[num bones]");
			return false;
		}
		int numBones = 0;
		try
		{
			const auto& line = *lineIt;
			auto numStr = line.substr(0, line.find(';'));
			numBones = std::stoi(numStr);
		}
		catch (std::exception& e)
		{
			SABA_INFO("VPD File Parse Error. {}:[{}]", static_cast<size_t>(lineIt - lines.begin()), e.what());
			return false;
		}
		std::vector<VPDBone> bones(numBones);

		++lineIt;

		lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
			return !line.empty();
		});
		int boneCount = 0;
		while (boneCount < numBones && lineIt != lines.end())
		{
			int boneIdx = 0;
			{
				const auto& line = *lineIt;
				auto delimPos1 = line.find("Bone");
				if (delimPos1 == std::string::npos)
				{
					SABA_INFO("VPD File Parse Error. {}:[Not Found Bone]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}
				delimPos1 += 4;

				auto delimPos2 = line.find('{', delimPos1);
				if (delimPos2 == std::string::npos)
				{
					SABA_INFO("VPD File Parse Error. {}:[Not Found Bone]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto numStr = line.substr(delimPos1, delimPos2 - delimPos1);
				try
				{
					boneIdx = std::stoi(numStr);
				}
				catch (std::exception& e)
				{
					SABA_INFO("VPD File Parse Error. {}:[{}]", static_cast<size_t>(lineIt - lines.begin()), e.what());
					return false;
				}
				if (boneIdx >= numBones)
				{
					SABA_INFO("VPD File Parse Error. {}:[Bone Index over]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}
				bones[boneIdx].m_boneName = line.substr(delimPos2 + 1);
			}
			++lineIt;
			lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
				return !line.empty();
			});
			
			{
				const auto& line = *lineIt;
				auto delim1 = line.find_first_not_of(" \t");
				auto delim2 = line.find_first_of(" \t,", delim1);
				if (std::string::npos == delim1 || std::string::npos == delim2)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto delim3 = line.find_first_not_of(" \t,", delim2);
				auto delim4 = line.find_first_of(" \t,", delim3);
				if (std::string::npos == delim3 || std::string::npos == delim4)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto delim5 = line.find_first_not_of(" \t,", delim4);
				auto delim6 = line.find_first_of(" \t;", delim5);
				if (std::string::npos == delim5 || std::string::npos == delim6)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto delim7 = line.find_first_of(';', delim5);
				if (std::string::npos == delim7)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				try
				{
					auto numStr1 = line.substr(delim1, delim2 - delim1);
					auto numStr2 = line.substr(delim3, delim4 - delim3);
					auto numStr3 = line.substr(delim5, delim6 - delim5);
					bones[boneIdx].m_translate.x = std::stof(numStr1);
					bones[boneIdx].m_translate.y = std::stof(numStr2);
					bones[boneIdx].m_translate.z = std::stof(numStr3);
				}
				catch (std::exception& e)
				{
					SABA_INFO("VPD File Parse Error. {}:[{}]", static_cast<size_t>(lineIt - lines.begin()), e.what());
					return false;
				}
			}
			++lineIt;
			lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
				return !line.empty();
			});

			{
				const auto& line = *lineIt;
				auto delim1 = line.find_first_not_of(" \t");
				auto delim2 = line.find_first_of(" \t,", delim1);
				if (std::string::npos == delim1 || std::string::npos == delim2)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto delim3 = line.find_first_not_of(" \t,", delim2);
				auto delim4 = line.find_first_of(" \t,", delim3);
				if (std::string::npos == delim3 || std::string::npos == delim4)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto delim5 = line.find_first_not_of(" \t,", delim4);
				auto delim6 = line.find_first_of(" \t,", delim5);
				if (std::string::npos == delim5 || std::string::npos == delim6)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto delim7 = line.find_first_not_of(" \t,", delim6);
				auto delim8 = line.find_first_of(" \t;", delim7);
				if (std::string::npos == delim7 || std::string::npos == delim8)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				if (std::string::npos == delim7)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				try
				{
					auto numStr1 = line.substr(delim1, delim2 - delim1);
					auto numStr2 = line.substr(delim3, delim4 - delim3);
					auto numStr3 = line.substr(delim5, delim6 - delim5);
					auto numStr4 = line.substr(delim7, delim8 - delim7);
					bones[boneIdx].m_quaternion.x = std::stof(numStr1);
					bones[boneIdx].m_quaternion.y = std::stof(numStr2);
					bones[boneIdx].m_quaternion.z = std::stof(numStr3);
					bones[boneIdx].m_quaternion.w = std::stof(numStr4);
				}
				catch (std::exception& e)
				{
					SABA_INFO("VPD File Parse Error. {}:[{}]", static_cast<size_t>(lineIt - lines.begin()), e.what());
					return false;
				}
			}
			++lineIt;
			lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
				return !line.empty();
			});

			{
				const auto& line = *lineIt;
				if (line.find('}') == std::string::npos)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}
			}
			++lineIt;
			lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
				return !line.empty();
			});
			boneCount++;
		}

		for (auto& bone : bones)
		{
			std::u16string u16Str = ConvertSjisToU16String(bone.m_boneName.c_str());
			std::string u8Str;
			ConvU16ToU8(u16Str, u8Str);
			bone.m_boneName = u8Str;
		}

		vpd->m_bones = std::move(bones);

		std::vector<VPDMorph> morphs;
		while (lineIt != lines.end())
		{
			VPDMorph morph;
			{
				const auto& line = *lineIt;
				auto delimPos1 = line.find("Morph");
				if (delimPos1 == std::string::npos)
				{
					SABA_INFO("VPD File Parse Error. {}:[Not Found Morph]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}
				delimPos1 += sizeof("Morph") - 1;

				auto delimPos2 = line.find('{', delimPos1);
				if (delimPos2 == std::string::npos)
				{
					SABA_INFO("VPD File Parse Error. {}:[Not Found Morph]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				auto numStr = line.substr(delimPos1, delimPos2 - delimPos1);
				try
				{
					std::ignore = std::stoi(numStr);
				}
				catch (std::exception& e)
				{
					SABA_INFO("VPD File Parse Error. {}:[{}]", static_cast<size_t>(lineIt - lines.begin()), e.what());
					return false;
				}
				morph.m_morphName = line.substr(delimPos2 + 1);
			}
			++lineIt;
			lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line) {
				return !line.empty();
			});

			{
				const auto& line = *lineIt;
				auto delim1 = line.find_first_not_of(" \t");
				auto delim2 = line.find_first_of(" \t;", delim1);
				if (std::string::npos == delim1 || std::string::npos == delim2)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				if (std::string::npos == delim2)
				{
					SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
					return false;
				}

				try
				{
					auto numStr1 = line.substr(delim1, delim2 - delim1);
					morph.m_weight = std::stof(numStr1);
				}
				catch (std::exception& e)
				{
					SABA_INFO("VPD File Parse Error. {}:[{}]", static_cast<size_t>(lineIt - lines.begin()), e.what());
					return false;
				}
				++lineIt;
				lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line2) {
					return !line2.empty();
				});

				{
					const auto& line2 = *lineIt;
					if (line2.find('}') == std::string::npos)
					{
						SABA_INFO("VPD File Parse Error. {}:[Split error]", static_cast<size_t>(lineIt - lines.begin()));
						return false;
					}
				}
				++lineIt;
				lineIt = std::find_if(lineIt, lines.end(), [](const std::string& line2) {
					return !line2.empty();
				});
				boneCount++;
			}

			morphs.emplace_back(std::move(morph));
		}

		for (auto& morph : morphs)
		{
			std::u16string u16Str = ConvertSjisToU16String(morph.m_morphName.c_str());
			std::string u8Str;
			ConvU16ToU8(u16Str, u8Str);
			morph.m_morphName = u8Str;
		}

		vpd->m_morphs = std::move(morphs);

		return true;
	}
}
