//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_PMDFILE_H_
#define SABA_MODEL_MMD_PMDFILE_H_

#include "MMDFileString.h"

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <array>

namespace saba
{
	/**
	 * @brief Template for PMD string with fixed size.
	 * @tparam Size Size of the string.
	 */
	template <size_t Size>
	using PMDString = MMDFileString<Size>;

	/**
	 * @brief Structure representing the header of a PMD file.
	 */
	struct PMDHeader
	{
		PMDString<3>	m_magic;                ///< Magic number.
		float			m_version;              ///< Version number.
		PMDString<20>	m_modelName;            ///< Model name.
		PMDString<256>	m_comment;              ///< Comment.

		uint8_t			m_haveEnglishNameExt;   ///< Flag for English name extension.
		PMDString<20>	m_englishModelNameExt;  ///< English model name extension.
		PMDString<256>	m_englishCommentExt;    ///< English comment extension.
	};

	/**
	 * @brief Structure representing a vertex in a PMD file.
	 */
	struct PMDVertex
	{
		glm::vec3	m_position;   ///< Position of the vertex.
		glm::vec3	m_normal;     ///< Normal of the vertex.
		glm::vec2	m_uv;         ///< UV coordinates of the vertex.
		uint16_t	m_bone[2];    ///< Bone indices.
		uint8_t		m_boneWeight; ///< Bone weight.
		uint8_t		m_edge;       ///< Edge flag.
	};

	/**
	 * @brief Structure representing a face in a PMD file.
	 */
	struct PMDFace
	{
		uint16_t  m_vertices[3]; ///< Indices of the vertices forming the face.
	};

	/**
	 * @brief Structure representing a material in a PMD file.
	 */
	struct PMDMaterial
	{
		glm::vec3		m_diffuse;       ///< Diffuse color.
		float			m_alpha;         ///< Alpha value.
		float			m_specularPower; ///< Specular power.
		glm::vec3		m_specular;      ///< Specular color.
		glm::vec3		m_ambient;       ///< Ambient color.
		uint8_t			m_toonIndex;     ///< Toon index.
		uint8_t			m_edgeFlag;      ///< Edge flag.
		uint32_t		m_faceVertexCount; ///< Number of face vertices.
		PMDString<20>	m_textureName;   ///< Texture name.
	};

	/**
	 * @brief Structure representing a bone in a PMD file.
	 */
	struct PMDBone
	{
		PMDString<20>	m_boneName;           ///< Bone name.
		uint16_t		m_parent;             ///< Parent bone index.
		uint16_t		m_tail;               ///< Tail bone index.
		uint8_t			m_boneType;           ///< Bone type.
		uint16_t		m_ikParent;           ///< IK parent bone index.
		glm::vec3		m_position;           ///< Position of the bone.
		PMDString<20>	m_englishBoneNameExt; ///< English bone name extension.
	};

	/**
	 * @brief Structure representing an IK in a PMD file.
	 */
	struct PMDIk
	{
		using ChainList = std::vector<uint16_t>;

		uint16_t	m_ikNode;       ///< IK node index.
		uint16_t	m_ikTarget;     ///< IK target index.
		uint8_t		m_numChain;     ///< Number of chains.
		uint16_t	m_numIteration; ///< Number of iterations.
		float		m_rotateLimit;  ///< Rotation limit.
		ChainList	m_chanins;      ///< List of chains.
	};

	/**
	 * @brief Structure representing a morph in a PMD file.
	 */
	struct PMDMorph
	{
		/**
		 * @brief Structure representing a vertex in a morph.
		 */
		struct Vertex
		{
			uint32_t	m_vertexIndex; ///< Vertex index.
			glm::vec3	m_position;    ///< Position of the vertex.
		};
		using VertexList = std::vector<Vertex>;

		/**
		 * @brief Enumeration of morph types.
		 */
		enum MorphType : uint8_t
		{
			Base,    ///< Base morph.
			Eyebrow, ///< Eyebrow morph.
			Eye,     ///< Eye morph.
			Rip,     ///< Rip morph.
			Other    ///< Other morph.
		};

		PMDString<20>	m_morphName;           ///< Morph name.
		MorphType		m_morphType;           ///< Morph type.
		VertexList		m_vertices;            ///< List of vertices.
		PMDString<20>	m_englishShapeNameExt; ///< English shape name extension.
	};

	/**
	 * @brief Structure representing a morph display list in a PMD file.
	 */
	struct PMDMorphDisplayList
	{
		typedef std::vector<uint16_t> DisplayList;

		DisplayList	m_displayList; ///< List of morph displays.
	};

	/**
	 * @brief Structure representing a bone display list in a PMD file.
	 */
	struct PMDBoneDisplayList
	{
		typedef std::vector<uint16_t> DisplayList;

		PMDString<50>	m_name;           ///< Name of the bone display list.
		DisplayList		m_displayList;    ///< List of bone displays.
		PMDString<50>	m_englishNameExt; ///< English name extension.
	};

	/**
	 * @brief Enumeration of rigid body shapes in a PMD file.
	 */
	enum class PMDRigidBodyShape : uint8_t
	{
		Sphere,  ///< 0: Sphere
		Box,     ///< 1: Box
		Capsule, ///< 2: Capsule
	};

	/**
	 * @brief Enumeration of rigid body operations in a PMD file.
	 */
	enum class PMDRigidBodyOperation : uint8_t
	{
		Static,            ///< 0: Follow Bone
		Dynamic,           ///< 1: Physics Simulation
		DynamicAdjustBone, ///< 2: Physics Simulation (Bone Alignment)
	};

	/**
	 * @brief Structure representing a rigid body extension in a PMD file.
	 */
	struct PMDRigidBodyExt
	{
		PMDString<20>	m_rigidBodyName; ///< Rigid body name.
		uint16_t		m_boneIndex;     ///< Bone index.
		uint8_t			m_groupIndex;    ///< Group index.
		uint16_t		m_groupTarget;   ///< Collision group (each bit is a collision flag with the group).
		PMDRigidBodyShape	m_shapeType; ///< Shape type.
		float			m_shapeWidth;   ///< Shape width.
		float			m_shapeHeight;  ///< Shape height.
		float			m_shapeDepth;   ///< Shape depth.
		glm::vec3		m_pos;          ///< Position.
		glm::vec3		m_rot;          ///< Rotation.
		float			m_rigidBodyWeight; ///< Rigid body weight.
		float			m_rigidBodyPosDimmer; ///< Rigid body position dimmer.
		float			m_rigidBodyRotDimmer; ///< Rigid body rotation dimmer.
		float			m_rigidBodyRecoil; ///< Rigid body recoil.
		float			m_rigidBodyFriction; ///< Rigid body friction.
		PMDRigidBodyOperation	m_rigidBodyType; ///< Rigid body type.
	};

	/**
	 * @brief Structure representing a joint extension in a PMD file.
	 */
	struct PMDJointExt
	{
		enum { NumJointName = 20 };

		PMDString<20>	m_jointName;      ///< Joint name.
		uint32_t		m_rigidBodyA;     ///< Rigid body A index.
		uint32_t		m_rigidBodyB;     ///< Rigid body B index.
		glm::vec3		m_jointPos;       ///< Joint position.
		glm::vec3		m_jointRot;       ///< Joint rotation.
		glm::vec3		m_constrainPos1;  ///< Constraint position 1.
		glm::vec3		m_constrainPos2;  ///< Constraint position 2.
		glm::vec3		m_constrainRot1;  ///< Constraint rotation 1.
		glm::vec3		m_constrainRot2;  ///< Constraint rotation 2.
		glm::vec3		m_springPos;      ///< Spring position.
		glm::vec3		m_springRot;      ///< Spring rotation.
	};

	/**
	 * @brief Structure representing a PMD file.
	 */
	struct PMDFile
	{
		PMDHeader						m_header;           ///< Header of the PMD file.
		std::vector<PMDVertex>			m_vertices;         ///< List of vertices.
		std::vector<PMDFace>			m_faces;            ///< List of faces.
		std::vector<PMDMaterial>		m_materials;        ///< List of materials.
		std::vector<PMDBone>			m_bones;            ///< List of bones.
		std::vector<PMDIk>				m_iks;              ///< List of IKs.
		std::vector<PMDMorph>			m_morphs;           ///< List of morphs.
		PMDMorphDisplayList				m_morphDisplayList; ///< Morph display list.
		std::vector<PMDBoneDisplayList>	m_boneDisplayLists; ///< List of bone display lists.
		std::array<PMDString<100>, 10>	m_toonTextureNames; ///< List of toon texture names.
		std::vector<PMDRigidBodyExt>	m_rigidBodies;      ///< List of rigid bodies.
		std::vector<PMDJointExt>		m_joints;           ///< List of joints.
	};

	/**
	 * @brief Read a PMD file.
	 * @param pmdFile Pointer to the PMD file structure.
	 * @param filename Path to the PMD file.
	 * @return True if reading is successful, false otherwise.
	 */
	bool ReadPMDFile(PMDFile* pmdFile, const char* filename);

	/**
	 * @brief Write a PMD file.
	 * @param pmdFile Pointer to the PMD file structure.
	 * @param filename Path to the PMD file.
	 * @return True if writing is successful, false otherwise.
	 */
	bool WritePMDFile(const PMDFile* pmdFile, const char* filename);
}

#endif // !SABA_MODEL_MMD_PMDFILE_H_
