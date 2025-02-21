//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "PMDFile.h"

#include <Saba/Base/File.h>
#include <Saba/Base/Log.h>

#include <sstream>
#include <iomanip>

namespace saba
{
	namespace
	{
		template <typename T>
		bool Read(T* data, File& file)
		{
			return file.Read(data);
		}

		template <typename T>
		bool Write(const T* data, File& file)
		{
			return file.Write(data);
		}

		bool ReadHeader(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			auto& [m_magic, m_version, m_modelName, m_comment, m_haveEnglishNameExt, m_englishModelNameExt, m_englishCommentExt] = pmdFile->m_header;
			Read(&m_magic, file);
			Read(&m_version, file);
			Read(&m_modelName, file);
			Read(&m_comment, file);

			m_haveEnglishNameExt = 0;

			if (m_magic.ToString() != "Pmd")
			{
				SABA_ERROR("PMD Header Error.");
				return false;
			}

			if (m_version != 1.0f)
			{
				SABA_ERROR("PMD Version Error.");
				return false;
			}

			return !file.IsBad();
		}

		bool WriteHeader(const PMDFile* pmdFile, File& file)
		{
			const auto& [m_magic, m_version, m_modelName, m_comment, m_haveEnglishNameExt, m_englishModelNameExt, m_englishCommentExt] = pmdFile->m_header;
			Write(&m_magic, file);
			Write(&m_version, file);
			Write(&m_modelName, file);
			Write(&m_comment, file);

			return !file.IsBad();
		}

		bool ReadVertex(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint32_t vertexCount = 0;
			if (!Read(&vertexCount, file))
			{
				return false;
			}

			auto& vertices = pmdFile->m_vertices;
			vertices.resize(vertexCount);
			for (auto& [m_position, m_normal, m_uv, m_bone, m_boneWeight, m_edge] : vertices)
			{
				Read(&m_position, file);
				Read(&m_normal, file);
				Read(&m_uv, file);
				Read(&m_bone[0], file);
				Read(&m_bone[1], file);
				Read(&m_boneWeight, file);
				Read(&m_edge, file);
			}

			return !file.IsBad();
		}

		bool WriteVertex(const PMDFile* pmdFile, File& file)
		{
			uint32_t vertexCount = static_cast<uint32_t>(pmdFile->m_vertices.size());
			Write(&vertexCount, file);

			for (const auto& [m_position, m_normal, m_uv, m_bone, m_boneWeight, m_edge] : pmdFile->m_vertices)
			{
				Write(&m_position, file);
				Write(&m_normal, file);
				Write(&m_uv, file);
				Write(&m_bone[0], file);
				Write(&m_bone[1], file);
				Write(&m_boneWeight, file);
				Write(&m_edge, file);
			}

			return !file.IsBad();
		}

		bool ReadFace(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint32_t faceCount = 0;
			if (!Read(&faceCount, file))
			{
				return false;
			}

			auto& faces = pmdFile->m_faces;
			faces.resize(faceCount / 3);
			for (auto& [m_vertices] : faces)
			{
				Read(&m_vertices[0], file);
				Read(&m_vertices[1], file);
				Read(&m_vertices[2], file);
			}

			return !file.IsBad();
		}

		bool WriteFace(const PMDFile* pmdFile, File& file)
		{
			uint32_t faceCount = static_cast<uint32_t>(pmdFile->m_faces.size() * 3);
			Write(&faceCount, file);

			for (const auto& [m_vertices] : pmdFile->m_faces)
			{
				Write(&m_vertices[0], file);
				Write(&m_vertices[1], file);
				Write(&m_vertices[2], file);
			}

			return !file.IsBad();
		}

		bool ReadMaterial(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint32_t materialCount = 0;
			if (!Read(&materialCount, file))
			{
				return false;
			}

			auto& materials = pmdFile->m_materials;
			materials.resize(materialCount);
			for (auto& [m_diffuse, m_alpha, m_specularPower, m_specular, m_ambient, m_toonIndex, m_edgeFlag, m_faceVertexCount, m_textureName] : materials)
			{
				Read(&m_diffuse, file);
				Read(&m_alpha, file);
				Read(&m_specularPower, file);
				Read(&m_specular, file);
				Read(&m_ambient, file);
				Read(&m_toonIndex, file);
				Read(&m_edgeFlag, file);
				Read(&m_faceVertexCount, file);
				Read(&m_textureName, file);
			}

			return !file.IsBad();
		}

		bool WriteMaterial(const PMDFile* pmdFile, File& file)
		{
			uint32_t materialCount = static_cast<uint32_t>(pmdFile->m_materials.size());
			Write(&materialCount, file);

			for (const auto& [m_diffuse, m_alpha, m_specularPower, m_specular, m_ambient, m_toonIndex, m_edgeFlag, m_faceVertexCount, m_textureName] : pmdFile->m_materials)
			{
				Write(&m_diffuse, file);
				Write(&m_alpha, file);
				Write(&m_specularPower, file);
				Write(&m_specular, file);
				Write(&m_ambient, file);
				Write(&m_toonIndex, file);
				Write(&m_edgeFlag, file);
				Write(&m_faceVertexCount, file);
				Write(&m_textureName, file);
			}

			return !file.IsBad();
		}

		bool ReadBone(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint16_t boneCount = 0;
			if (!Read(&boneCount, file))
			{
				return false;
			}

			auto& bones = pmdFile->m_bones;
			bones.resize(boneCount);
			for (auto& [m_boneName, m_parent, m_tail, m_boneType, m_ikParent, m_position, m_englishBoneNameExt] : bones)
			{
				Read(&m_boneName, file);
				Read(&m_parent, file);
				Read(&m_tail, file);
				Read(&m_boneType, file);
				Read(&m_ikParent, file);
				Read(&m_position, file);
			}

			return !file.IsBad();
		}

		bool WriteBone(const PMDFile* pmdFile, File& file)
		{
			uint16_t boneCount = static_cast<uint16_t>(pmdFile->m_bones.size());
			Write(&boneCount, file);

			for (const auto& [m_boneName, m_parent, m_tail, m_boneType, m_ikParent, m_position, m_englishBoneNameExt] : pmdFile->m_bones)
			{
				Write(&m_boneName, file);
				Write(&m_parent, file);
				Write(&m_tail, file);
				Write(&m_boneType, file);
				Write(&m_ikParent, file);
				Write(&m_position, file);
			}

			return !file.IsBad();
		}

		bool ReadIK(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint16_t ikCount = 0;
			if (!Read(&ikCount, file))
			{
				return false;
			}

			auto& iks = pmdFile->m_iks;
			iks.resize(ikCount);
			for (auto& [m_ikNode, m_ikTarget, m_numChain, m_numIteration, m_rotateLimit, m_chanins] : iks)
			{
				Read(&m_ikNode, file);
				Read(&m_ikTarget, file);
				Read(&m_numChain, file);
				Read(&m_numIteration, file);
				Read(&m_rotateLimit, file);

				m_chanins.resize(m_numChain);
				for (auto& ikChain : m_chanins)
				{
					Read(&ikChain, file);
				}
			}

			return !file.IsBad();
		}

		bool WriteIK(const PMDFile* pmdFile, File& file)
		{
			uint16_t ikCount = static_cast<uint16_t>(pmdFile->m_iks.size());
			Write(&ikCount, file);

			for (const auto& [m_ikNode, m_ikTarget, m_numChain, m_numIteration, m_rotateLimit, m_chanins] : pmdFile->m_iks)
			{
				Write(&m_ikNode, file);
				Write(&m_ikTarget, file);
				Write(&m_numChain, file);
				Write(&m_numIteration, file);
				Write(&m_rotateLimit, file);

				for (const auto& ikChain : m_chanins)
				{
					Write(&ikChain, file);
				}
			}

			return !file.IsBad();
		}

		bool ReadBlendShape(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint16_t blendShapeCount = 0;
			if (!Read(&blendShapeCount, file))
			{
				return false;
			}

			auto& morphs = pmdFile->m_morphs;
			morphs.resize(blendShapeCount);
			for (auto& [m_morphName, m_morphType, m_vertices, m_englishShapeNameExt] : morphs)
			{
				Read(&m_morphName, file);

				uint32_t vertexCount = 0;
				Read(&vertexCount, file);
				m_vertices.resize(vertexCount);

				uint8_t morphType;
				Read(&morphType, file);
				m_morphType = static_cast<PMDMorph::MorphType>(morphType);
				for (auto& [m_vertexIndex, m_position] : m_vertices)
				{
					Read(&m_vertexIndex, file);
					Read(&m_position, file);
				}
			}

			return !file.IsBad();
		}

		bool WriteBlendShape(const PMDFile* pmdFile, File& file)
		{
			uint16_t blendShapeCount = static_cast<uint16_t>(pmdFile->m_morphs.size());
			Write(&blendShapeCount, file);

			for (const auto& [m_morphName, m_morphType, m_vertices, m_englishShapeNameExt] : pmdFile->m_morphs)
			{
				Write(&m_morphName, file);

				uint32_t vertexCount = static_cast<uint32_t>(m_vertices.size());
				Write(&vertexCount, file);

				uint8_t morphType = static_cast<uint8_t>(m_morphType);
				Write(&morphType, file);
				for (const auto& [m_vertexIndex, m_position] : m_vertices)
				{
					Write(&m_vertexIndex, file);
					Write(&m_position, file);
				}
			}

			return !file.IsBad();
		}

		bool ReadBlendShapeDisplayList(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint8_t displayListCount = 0;
			if (!Read(&displayListCount, file))
			{
				return false;
			}

			pmdFile->m_morphDisplayList.m_displayList.resize(displayListCount);
			for (auto& displayList : pmdFile->m_morphDisplayList.m_displayList)
			{
				Read(&displayList, file);
			}

			return !file.IsBad();
		}

		bool WriteBlendShapeDisplayList(const PMDFile* pmdFile, File& file)
		{
			uint8_t displayListCount = static_cast<uint8_t>(pmdFile->m_morphDisplayList.m_displayList.size());
			Write(&displayListCount, file);

			for (const auto& displayList : pmdFile->m_morphDisplayList.m_displayList)
			{
				Write(&displayList, file);
			}

			return !file.IsBad();
		}

		bool ReadBoneDisplayList(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint8_t displayListCount = 0;
			if (!Read(&displayListCount, file))
			{
				return false;
			}

			// ボーンの枠にはデフォルトでセンターが用意されているので、サイズを一つ多くする
			pmdFile->m_boneDisplayLists.resize(static_cast<size_t>(displayListCount) + 1);
			bool first = true;
			for (auto& [m_name, m_displayList, m_englishNameExt] : pmdFile->m_boneDisplayLists)
			{
				if (first)
				{
					first = false;
				}
				else
				{
					Read(&m_name, file);
				}
			}

			uint32_t displayCount = 0;
			if (!Read(&displayCount, file))
			{
				return false;
			}
			for (uint32_t displayIdx = 0; displayIdx < displayCount; displayIdx++)
			{
				uint16_t boneIdx = 0;
				Read(&boneIdx, file);

				uint8_t frameIdx = 0;
				Read(&frameIdx, file);
				pmdFile->m_boneDisplayLists[frameIdx].m_displayList.push_back(boneIdx);
			}

			return !file.IsBad();
		}

		bool WriteBoneDisplayList(const PMDFile* pmdFile, File& file)
		{
			uint8_t displayListCount = static_cast<uint8_t>(pmdFile->m_boneDisplayLists.size() - 1);
			Write(&displayListCount, file);

			bool first = true;
			for (const auto& [m_name, m_displayList, m_englishNameExt] : pmdFile->m_boneDisplayLists)
			{
				if (first)
				{
					first = false;
				}
				else
				{
					Write(&m_name, file);
				}
			}

			uint32_t displayCount = 0;
			for (const auto& displayList : pmdFile->m_boneDisplayLists)
			{
				displayCount += static_cast<uint32_t>(displayList.m_displayList.size());
			}
			Write(&displayCount, file);

			for (uint32_t displayIdx = 0; displayIdx < displayCount; displayIdx++)
			{
				uint16_t boneIdx = 0;
				uint8_t frameIdx = 0;
				for (const auto& displayList : pmdFile->m_boneDisplayLists)
				{
					for (const auto& bone : displayList.m_displayList)
					{
						boneIdx = bone;
						frameIdx = static_cast<uint8_t>(&displayList - &pmdFile->m_boneDisplayLists[0]);
						Write(&boneIdx, file);
						Write(&frameIdx, file);
					}
				}
			}

			return !file.IsBad();
		}

		bool ReadExt(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			Read(&pmdFile->m_header.m_haveEnglishNameExt, file);

			if (pmdFile->m_header.m_haveEnglishNameExt != 0)
			{
				// ModelName
				Read(&pmdFile->m_header.m_englishModelNameExt, file);

				// Comment
				Read(&pmdFile->m_header.m_englishCommentExt, file);

				// BoneName
				for (auto& [m_boneName, m_parent, m_tail, m_boneType, m_ikParent, m_position, m_englishBoneNameExt] : pmdFile->m_bones)
				{
					Read(&m_englishBoneNameExt, file);
				}

				// BlendShape Name
				/*
				BlendShapeの最初はbaseなので、EnglishNameを読まないようにする
				*/
				const size_t numBlensShape = pmdFile->m_morphs.size();
				for (size_t bsIdx = 1; bsIdx < numBlensShape; bsIdx++)
				{
					auto& [m_morphName, m_morphType, m_vertices, m_englishShapeNameExt] = pmdFile->m_morphs[bsIdx];
					Read(&m_englishShapeNameExt, file);
				}

				// BoneDisplayNameLists
				/*
				BoneDisplayNameの最初はセンターとして使用しているので、EnglishNameを読まないようにする
				*/
				const size_t numBoneDisplayName = pmdFile->m_boneDisplayLists.size();
				for (size_t displayIdx = 1; displayIdx < numBoneDisplayName; displayIdx++)
				{
					auto& [m_name, m_displayList, m_englishNameExt] = pmdFile->m_boneDisplayLists[displayIdx];
					Read(&m_englishNameExt, file);
				}
			}

			return !file.IsBad();
		}

		bool WriteExt(const PMDFile* pmdFile, File& file)
		{
			Write(&pmdFile->m_header.m_haveEnglishNameExt, file);

			if (pmdFile->m_header.m_haveEnglishNameExt != 0)
			{
				// ModelName
				Write(&pmdFile->m_header.m_englishModelNameExt, file);

				// Comment
				Write(&pmdFile->m_header.m_englishCommentExt, file);

				// BoneName
				for (const auto& [m_boneName, m_parent, m_tail, m_boneType, m_ikParent, m_position, m_englishBoneNameExt] : pmdFile->m_bones)
				{
					Write(&m_englishBoneNameExt, file);
				}

				// BlendShape Name
				const size_t numBlensShape = pmdFile->m_morphs.size();
				for (size_t bsIdx = 1; bsIdx < numBlensShape; bsIdx++)
				{
					const auto& [m_morphName, m_morphType, m_vertices, m_englishShapeNameExt] = pmdFile->m_morphs[bsIdx];
					Write(&m_englishShapeNameExt, file);
				}

				// BoneDisplayNameLists
				const size_t numBoneDisplayName = pmdFile->m_boneDisplayLists.size();
				for (size_t displayIdx = 1; displayIdx < numBoneDisplayName; displayIdx++)
				{
					const auto& [m_name, m_displayList, m_englishNameExt] = pmdFile->m_boneDisplayLists[displayIdx];
					Write(&m_englishNameExt, file);
				}
			}

			return !file.IsBad();
		}

		bool ReadToonTextureName(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			for (auto& toonTexName : pmdFile->m_toonTextureNames)
			{
				Read(&toonTexName, file);
			}

			return !file.IsBad();
		}

		bool WriteToonTextureName(const PMDFile* pmdFile, File& file)
		{
			for (const auto& toonTexName : pmdFile->m_toonTextureNames)
			{
				Write(&toonTexName, file);
			}

			return !file.IsBad();
		}

		bool ReadRigidBodyExt(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint32_t rigidBodyCount = 0;
			if (!Read(&rigidBodyCount, file))
			{
				return false;
			}

			pmdFile->m_rigidBodies.resize(rigidBodyCount);
			for (auto& [m_rigidBodyName, m_boneIndex, m_groupIndex, m_groupTarget, m_shapeType, m_shapeWidth, m_shapeHeight, m_shapeDepth, m_pos, m_rot, m_rigidBodyWeight, m_rigidBodyPosDimmer, m_rigidBodyRotDimmer, m_rigidBodyRecoil, m_rigidBodyFriction, m_rigidBodyType] : pmdFile->m_rigidBodies)
			{
				Read(&m_rigidBodyName, file);
				Read(&m_boneIndex, file);
				Read(&m_groupIndex, file);
				Read(&m_groupTarget, file);
				Read(&m_shapeType, file);
				Read(&m_shapeWidth, file);
				Read(&m_shapeHeight, file);
				Read(&m_shapeDepth, file);
				Read(&m_pos, file);
				Read(&m_rot, file);
				Read(&m_rigidBodyWeight, file);
				Read(&m_rigidBodyPosDimmer, file);
				Read(&m_rigidBodyRotDimmer, file);
				Read(&m_rigidBodyRecoil, file);
				Read(&m_rigidBodyFriction, file);
				Read(&m_rigidBodyType, file);
			}

			return !file.IsBad();
		}

		bool WriteRigidBodyExt(const PMDFile* pmdFile, File& file)
		{
			uint32_t rigidBodyCount = static_cast<uint32_t>(pmdFile->m_rigidBodies.size());
			Write(&rigidBodyCount, file);

			for (const auto& [m_rigidBodyName, m_boneIndex, m_groupIndex, m_groupTarget, m_shapeType, m_shapeWidth, m_shapeHeight, m_shapeDepth, m_pos, m_rot, m_rigidBodyWeight, m_rigidBodyPosDimmer, m_rigidBodyRotDimmer, m_rigidBodyRecoil, m_rigidBodyFriction, m_rigidBodyType] : pmdFile->m_rigidBodies)
			{
				Write(&m_rigidBodyName, file);
				Write(&m_boneIndex, file);
				Write(&m_groupIndex, file);
				Write(&m_groupTarget, file);
				Write(&m_shapeType, file);
				Write(&m_shapeWidth, file);
				Write(&m_shapeHeight, file);
				Write(&m_shapeDepth, file);
				Write(&m_pos, file);
				Write(&m_rot, file);
				Write(&m_rigidBodyWeight, file);
				Write(&m_rigidBodyPosDimmer, file);
				Write(&m_rigidBodyRotDimmer, file);
				Write(&m_rigidBodyRecoil, file);
				Write(&m_rigidBodyFriction, file);
				Write(&m_rigidBodyType, file);
			}

			return !file.IsBad();
		}

		bool ReadJointExt(PMDFile* pmdFile, File& file)
		{
			if (file.IsBad())
			{
				return false;
			}

			uint32_t jointCount = 0;
			if (!Read(&jointCount, file))
			{
				return false;
			}

			pmdFile->m_joints.resize(jointCount);
			for (auto& [m_jointName, m_rigidBodyA, m_rigidBodyB, m_jointPos, m_jointRot, m_constrainPos1, m_constrainPos2, m_constrainRot1, m_constrainRot2, m_springPos, m_springRot] : pmdFile->m_joints)
			{
				Read(&m_jointName, file);
				Read(&m_rigidBodyA, file);
				Read(&m_rigidBodyB, file);
				Read(&m_jointPos, file);
				Read(&m_jointRot, file);
				Read(&m_constrainPos1, file);
				Read(&m_constrainPos2, file);
				Read(&m_constrainRot1, file);
				Read(&m_constrainRot2, file);
				Read(&m_springPos, file);
				Read(&m_springRot, file);
			}

			return !file.IsBad();
		}

		bool WriteJointExt(const PMDFile* pmdFile, File& file)
		{
			uint32_t jointCount = static_cast<uint32_t>(pmdFile->m_joints.size());
			Write(&jointCount, file);

			for (const auto& [m_jointName, m_rigidBodyA, m_rigidBodyB, m_jointPos, m_jointRot, m_constrainPos1, m_constrainPos2, m_constrainRot1, m_constrainRot2, m_springPos, m_springRot] : pmdFile->m_joints)
			{
				Write(&m_jointName, file);
				Write(&m_rigidBodyA, file);
				Write(&m_rigidBodyB, file);
				Write(&m_jointPos, file);
				Write(&m_jointRot, file);
				Write(&m_constrainPos1, file);
				Write(&m_constrainPos2, file);
				Write(&m_constrainRot1, file);
				Write(&m_constrainRot2, file);
				Write(&m_springPos, file);
				Write(&m_springRot, file);
			}

			return !file.IsBad();
		}

		bool ReadPMDFile(PMDFile* pmdFile, File& file)
		{
			if (!ReadHeader(pmdFile, file))
			{
				SABA_ERROR("ReadHeader Fail.");
				return false;
			}

			if (!ReadVertex(pmdFile, file))
			{
				SABA_ERROR("ReadVertex Fail.");
				return false;
			}

			if (!ReadFace(pmdFile, file))
			{
				SABA_ERROR("ReadFace Fail.");
				return false;
			}

			if (!ReadMaterial(pmdFile, file))
			{
				SABA_ERROR("ReadMaterial Fail.");
				return false;
			}

			if (!ReadBone(pmdFile, file))
			{
				SABA_ERROR("ReadBone Fail.");
				return false;
			}

			if (!ReadIK(pmdFile, file))
			{
				SABA_ERROR("ReadIK Fail.");
				return false;
			}

			if (!ReadBlendShape(pmdFile, file))
			{
				SABA_ERROR("ReadBlendShape Fail.");
				return false;
			}

			if (!ReadBlendShapeDisplayList(pmdFile, file))
			{
				SABA_ERROR("ReadBlendShapeDisplayList Fail.");
				return false;
			}

			if (!ReadBoneDisplayList(pmdFile, file))
			{
				SABA_ERROR("ReadBoneDisplayList Fail.");
				return false;
			}

			size_t toonTexIdx = 1;
			for (auto& toonTexName : pmdFile->m_toonTextureNames)
			{
				std::stringstream ss;
				ss << "toon" << std::setfill('0') << std::setw(2) << toonTexIdx << ".bmp";
				std::string name = ss.str();
				toonTexName.Set(name.c_str());
				toonTexIdx++;
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadExt(pmdFile, file))
				{
					SABA_ERROR("ReadExt Fail.");
					return false;
				}
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadToonTextureName(pmdFile, file))
				{
					SABA_ERROR("ReadToonTextureName Fail.");
					return false;
				}
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadRigidBodyExt(pmdFile, file))
				{
					SABA_ERROR("ReadRigidBodyExt Fail.");
					return false;
				}
			}

			if (file.Tell() < file.GetSize())
			{
				if (!ReadJointExt(pmdFile, file))
				{
					SABA_ERROR("ReadJointExt Fail.");
					return false;
				}
			}

			return true;
		}

		bool WritePMDFile(const PMDFile* pmdFile, File& file)
		{
			if (!WriteHeader(pmdFile, file))
			{
				SABA_ERROR("WriteHeader Fail.");
				return false;
			}

			if (!WriteVertex(pmdFile, file))
			{
				SABA_ERROR("WriteVertex Fail.");
				return false;
			}

			if (!WriteFace(pmdFile, file))
			{
				SABA_ERROR("WriteFace Fail.");
				return false;
			}

			if (!WriteMaterial(pmdFile, file))
			{
				SABA_ERROR("WriteMaterial Fail.");
				return false;
			}

			if (!WriteBone(pmdFile, file))
			{
				SABA_ERROR("WriteBone Fail.");
				return false;
			}

			if (!WriteIK(pmdFile, file))
			{
				SABA_ERROR("WriteIK Fail.");
				return false;
			}

			if (!WriteBlendShape(pmdFile, file))
			{
				SABA_ERROR("WriteBlendShape Fail.");
				return false;
			}

			if (!WriteBlendShapeDisplayList(pmdFile, file))
			{
				SABA_ERROR("WriteBlendShapeDisplayList Fail.");
				return false;
			}

			if (!WriteBoneDisplayList(pmdFile, file))
			{
				SABA_ERROR("WriteBoneDisplayList Fail.");
				return false;
			}

			if (!WriteExt(pmdFile, file))
			{
				SABA_ERROR("WriteExt Fail.");
				return false;
			}

			if (!WriteToonTextureName(pmdFile, file))
			{
				SABA_ERROR("WriteToonTextureName Fail.");
				return false;
			}

			if (!WriteRigidBodyExt(pmdFile, file))
			{
				SABA_ERROR("WriteRigidBodyExt Fail.");
				return false;
			}

			if (!WriteJointExt(pmdFile, file))
			{
				SABA_ERROR("WriteJointExt Fail.");
				return false;
			}

			return true;
		}
	}

	bool ReadPMDFile(PMDFile* pmdFile, const char * filename)
	{
		SABA_INFO("PMD File Open. {}", filename);

		File file;
		if (!file.Open(filename))
		{
			SABA_INFO("PMD File Open Fail. {}", filename);
			return false;
		}

		if (!ReadPMDFile(pmdFile, file))
		{
			SABA_INFO("PMD File Read Fail. {}", filename);
			return false;
		}
		SABA_INFO("PMD File Read Successed. {}", filename);

		return true;
	}

	bool WritePMDFile(const PMDFile* pmdFile, const char* filename)
	{
		SABA_INFO("PMD File Open. {}", filename);

		File file;
		if (!file.Create(filename))
		{
			SABA_INFO("PMD File Open Fail. {}", filename);
			return false;
		}

		if (!WritePMDFile(pmdFile, file))
		{
			SABA_INFO("PMD File Write Fail. {}", filename);
			return false;
		}
		SABA_INFO("PMD File Write Successed. {}", filename);

		return true;
	}

}
