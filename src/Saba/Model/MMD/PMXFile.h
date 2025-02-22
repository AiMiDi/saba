//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_PMXFILE_H_
#define SABA_MODEL_PMXFILE_H_

#include "MMDFileString.h"

#include <cstdint>
#include <string>
#include <vector>

#include <glm/gtc/quaternion.hpp>

namespace saba
{
	template <size_t Size>
	using PMXString = MMDFileString<Size>;

	/**
	 * @brief Represents the header of a PMX file.
	 */
	struct PMXHeader
	{
		PMXString<4>	m_magic; ///< Magic string
		float			m_version; ///< Version number

		uint8_t	m_dataSize; ///< Data size

		uint8_t	m_encode; ///< Encoding type (0:UTF16 1:UTF8)
		uint8_t	m_addUVNum; ///< Number of additional UVs

		uint8_t	m_vertexIndexSize; ///< Vertex index size
		uint8_t	m_textureIndexSize; ///< Texture index size
		uint8_t	m_materialIndexSize; ///< Material index size
		uint8_t	m_boneIndexSize; ///< Bone index size
		uint8_t	m_morphIndexSize; ///< Morph index size
		uint8_t	m_rigidbodyIndexSize; ///< Rigidbody index size
	};

	/**
	 * @brief Represents the information of a PMX model.
	 */
	struct PMXInfo
	{
		std::string	m_modelName; ///< Model name
		std::string	m_englishModelName; ///< English model name
		std::string	m_comment; ///< Comment
		std::string	m_englishComment; ///< English comment
	};

	/*
	BDEF1
	m_boneIndices[0]

	BDEF2
	m_boneIndices[0-1]
	m_boneWeights[0]

	BDEF4
	m_boneIndices[0-3]
	m_boneWeights[0-3]

	SDEF
	m_boneIndices[0-1]
	m_boneWeights[0]
	m_sdefC
	m_sdefR0
	m_sdefR1

	QDEF
	m_boneIndices[0-3]
	m_boneWeights[0-3]
	*/
	/**
	 * @brief Represents the weight type of a PMX vertex.
	 */
	enum class PMXVertexWeight : uint8_t
	{
		BDEF1, ///< BDEF1
		BDEF2, ///< BDEF2
		BDEF4, ///< BDEF4
		SDEF, ///< SDEF
		QDEF, ///< QDEF
	};

	/**
	 * @brief Represents a vertex in a PMX file.
	 */
	struct PMXVertex
	{
		glm::vec3	m_position; ///< Position vector
		glm::vec3	m_normal; ///< Normal vector
		glm::vec2	m_uv; ///< UV coordinates

		glm::vec4	m_addUV[4]; ///< Additional UVs

		PMXVertexWeight	m_weightType; ///< Weight type
		int32_t		m_boneIndices[4]; ///< Bone indices
		float		m_boneWeights[4]; ///< Bone weights
		glm::vec3	m_sdefC; ///< SDEF C vector
		glm::vec3	m_sdefR0; ///< SDEF R0 vector
		glm::vec3	m_sdefR1; ///< SDEF R1 vector

		float	m_edgeMag; ///< Edge magnitude
	};

	/**
	 * @brief Represents a face in a PMX file.
	 */
	struct PMXFace
	{
		uint32_t	m_vertices[3]; ///< Vertex indices
	};

	/**
	 * @brief Represents a texture in a PMX file.
	 */
	struct PMXTexture
	{
		std::string m_textureName; ///< Texture name
	};

	/*
	0x01:両面描画
	0x02:地面影
	0x04:セルフシャドウマップへの描画
	0x08:セルフシャドウの描画
	0x10:エッジ描画
	0x20:頂点カラー(※2.1拡張)
	0x40:Point描画(※2.1拡張)
	0x80:Line描画(※2.1拡張)
	*/
	/**
	 * @brief Represents the draw mode flags of a PMX material.
	 */
	enum class PMXDrawModeFlags : uint8_t
	{
		BothFace = 0x01, ///< Both face
		GroundShadow = 0x02, ///< Ground shadow
		CastSelfShadow = 0x04, ///< Cast self shadow
		RecieveSelfShadow = 0x08, ///< Receive self shadow
		DrawEdge = 0x10, ///< Draw edge
		VertexColor = 0x20, ///< Vertex color
		DrawPoint = 0x40, ///< Draw point
		DrawLine = 0x80, ///< Draw line
	};

	/*
	0:無効
	1:乗算
	2:加算
	3:サブテクスチャ(追加UV1のx,yをUV参照して通常テクスチャ描画を行う)
	*/
	/**
	 * @brief Represents the sphere mode of a PMX material.
	 */
	enum class PMXSphereMode : uint8_t
	{
		None, ///< None
		Mul, ///< Multiply
		Add, ///< Add
		SubTexture, ///< Sub texture
	};

	/**
	 * @brief Represents the toon mode of a PMX material.
	 */
	enum class PMXToonMode : uint8_t
	{
		Separate, ///< Separate toon
		Common, ///< Common toon
	};

	/**
	 * @brief Represents a material in a PMX file.
	 */
	struct PMXMaterial
	{
		std::string	m_name; ///< Material name
		std::string	m_englishName; ///< English material name

		glm::vec4	m_diffuse; ///< Diffuse color
		glm::vec3	m_specular; ///< Specular color
		float		m_specularPower; ///< Specular power
		glm::vec3	m_ambient; ///< Ambient color

		PMXDrawModeFlags m_drawMode; ///< Draw mode flags

		glm::vec4	m_edgeColor; ///< Edge color
		float		m_edgeSize; ///< Edge size

		int32_t	m_textureIndex; ///< Texture index
		int32_t	m_sphereTextureIndex; ///< Sphere texture index
		PMXSphereMode m_sphereMode; ///< Sphere mode

		PMXToonMode	m_toonMode; ///< Toon mode
		int32_t		m_toonTextureIndex; ///< Toon texture index

		std::string	m_memo; ///< Memo

		int32_t	m_numFaceVertices; ///< Number of face vertices
	};

	/*
	0x0001  : 接続先(PMD子ボーン指定)表示方法 -> 0:座標オフセットで指定 1:ボーンで指定

	0x0002  : 回転可能
	0x0004  : 移動可能
	0x0008  : 表示
	0x0010  : 操作可

	0x0020  : IK

	0x0080  : ローカル付与 | 付与対象 0:ユーザー変形値／IKリンク／多重付与 1:親のローカル変形量
	0x0100  : 回転付与
	0x0200  : 移動付与

	0x0400  : 軸固定
	0x0800  : ローカル軸

	0x1000  : 物理後変形
	0x2000  : 外部親変形
	*/
	/**
	 * @brief Represents the bone flags of a PMX bone.
	 */
	enum class PMXBoneFlags : uint16_t
	{
		TargetShowMode = 0x0001, ///< Target show mode
		AllowRotate = 0x0002, ///< Allow rotate
		AllowTranslate = 0x0004, ///< Allow translate
		Visible = 0x0008, ///< Visible
		AllowControl = 0x0010, ///< Allow control
		IK = 0x0020, ///< IK
		AppendLocal = 0x0080, ///< Append local
		AppendRotate = 0x0100, ///< Append rotate
		AppendTranslate = 0x0200, ///< Append translate
		FixedAxis = 0x0400, ///< Fixed axis
		LocalAxis = 0x800, ///< Local axis
		DeformAfterPhysics = 0x1000, ///< Deform after physics
		DeformOuterParent = 0x2000, ///< Deform outer parent
	};

	/**
	 * @brief Represents an IK link in a PMX file.
	 */
	struct PMXIKLink
	{
		int32_t			m_ikBoneIndex; ///< IK bone index
		unsigned char	m_enableLimit; ///< Enable limit flag

		glm::vec3	m_limitMin; ///< Minimum limit (in radians)
		glm::vec3	m_limitMax; ///< Maximum limit (in radians)
	};

	/**
	 * @brief Represents a bone in a PMX file.
	 */
	struct PMXBone
	{
		std::string	m_name; ///< Bone name
		std::string	m_englishName; ///< English bone name

		glm::vec3	m_position; ///< Position vector
		int32_t		m_parentBoneIndex; ///< Parent bone index
		int32_t		m_deformDepth; ///< Deform depth

		PMXBoneFlags	m_boneFlag; ///< Bone flags

		glm::vec3	m_positionOffset; ///< Position offset (if target show mode is 0)
		int32_t		m_linkBoneIndex; ///< Link bone index (if target show mode is 1)

		int32_t	m_appendBoneIndex; ///< Append bone index (if append rotate or append translate is enabled)
		float	m_appendWeight; ///< Append weight

		glm::vec3	m_fixedAxis; ///< Fixed axis (if fixed axis is enabled)

		glm::vec3	m_localXAxis; ///< Local X axis (if local axis is enabled)
		glm::vec3	m_localZAxis; ///< Local Z axis (if local axis is enabled)

		int32_t	m_keyValue; ///< Key value (if deform outer parent is enabled)

		int32_t	m_ikTargetBoneIndex; ///< IK target bone index (if IK is enabled)
		int32_t	m_ikIterationCount; ///< IK iteration count
		float	m_ikLimit; ///< IK limit (in radians)

		std::vector<PMXIKLink>	m_ikLinks; ///< IK links
	};


	/*
	0:グループ
	1:頂点
	2:ボーン,
	3:UV,
	4:追加UV1
	5:追加UV2
	6:追加UV3
	7:追加UV4
	8:材質
	9:フリップ(※2.1拡張)
	10:インパルス(※2.1拡張)
	*/
	/**
	 * @brief Represents the morph type of a PMX morph.
	 */
	enum class PMXMorphType : uint8_t
	{
		Group, ///< Group
		Position, ///< Position
		Bone, ///< Bone
		UV, ///< UV
		AddUV1, ///< Additional UV1
		AddUV2, ///< Additional UV2
		AddUV3, ///< Additional UV3
		AddUV4, ///< Additional UV4
		Material, ///< Material
		Flip, ///< Flip
		Impluse, ///< Impulse
	};

	/**
	 * @brief Represents a morph in a PMX file.
	 */
	struct PMXFileMorph
	{
		std::string	m_name; ///< Morph name
		std::string	m_englishName; ///< English morph name

		uint8_t			m_controlPanel; ///< Control panel (1:眉 2:目 3:口 4:その他)
		PMXMorphType	m_morphType; ///< Morph type

		struct PositionMorph
		{
			int32_t		m_vertexIndex; ///< Vertex index
			glm::vec3	m_position; ///< Position vector
		};

		struct UVMorph
		{
			int32_t		m_vertexIndex; ///< Vertex index
			glm::vec4	m_uv; ///< UV vector
		};

		struct BoneMorph
		{
			int32_t		m_boneIndex; ///< Bone index
			glm::vec3	m_position; ///< Position vector
			glm::quat	m_quaternion; ///< Rotation quaternion
		};

		struct MaterialMorph
		{
			enum class OpType : uint8_t
			{
				Mul, ///< Multiply
				Add, ///< Add
			};

			int32_t		m_materialIndex; ///< Material index
			OpType		m_opType; ///< Operation type
			glm::vec4	m_diffuse; ///< Diffuse color
			glm::vec3	m_specular; ///< Specular color
			float		m_specularPower; ///< Specular power
			glm::vec3	m_ambient; ///< Ambient color
			glm::vec4	m_edgeColor; ///< Edge color
			float		m_edgeSize; ///< Edge size
			glm::vec4	m_textureFactor; ///< Texture factor
			glm::vec4	m_sphereTextureFactor; ///< Sphere texture factor
			glm::vec4	m_toonTextureFactor; ///< Toon texture factor
		};

		struct GroupMorph
		{
			int32_t	m_morphIndex; ///< Morph index
			float	m_weight; ///< Weight
		};

		struct FlipMorph
		{
			int32_t	m_morphIndex; ///< Morph index
			float	m_weight; ///< Weight
		};

		struct ImpulseMorph
		{
			int32_t		m_rigidbodyIndex; ///< Rigidbody index
			uint8_t		m_localFlag; ///< Local flag (0:OFF 1:ON)
			glm::vec3	m_translateVelocity; ///< Translate velocity
			glm::vec3	m_rotateTorque; ///< Rotate torque
		};

		std::vector<PositionMorph>	m_positionMorph; ///< Position morphs
		std::vector<UVMorph>		m_uvMorph; ///< UV morphs
		std::vector<BoneMorph>		m_boneMorph; ///< Bone morphs
		std::vector<MaterialMorph>	m_materialMorph; ///< Material morphs
		std::vector<GroupMorph>		m_groupMorph; ///< Group morphs
		std::vector<FlipMorph>		m_flipMorph; ///< Flip morphs
		std::vector<ImpulseMorph>	m_impulseMorph; ///< Impulse morphs
	};

	/**
	 * @brief Represents a display frame in a PMX file.
	 */
	struct PMXDisplayFrame
	{
		std::string	m_name; ///< Frame name
		std::string	m_englishName; ///< English frame name

		/**
		 * @brief Represents the target type of a display frame.
		 */
		enum class TargetType : uint8_t
		{
			BoneIndex, ///< Bone index
			MorphIndex, ///< Morph index
		};
		struct Target
		{
			TargetType	m_type; ///< Target type
			int32_t		m_index; ///< Target index
		};

		/**
		 * @brief Represents the frame type of a display frame.
		 */
		enum class FrameType : uint8_t
		{
			DefaultFrame, ///< Default frame
			SpecialFrame, ///< Special frame
		};

		FrameType			m_flag; ///< Frame type
		std::vector<Target>	m_targets; ///< Targets
	};

	/**
	 * @brief Represents a rigidbody in a PMX file.
	 */
	struct PMXRigidbody
	{
		std::string	m_name; ///< Rigidbody name
		std::string	m_englishName; ///< English rigidbody name

		int32_t		m_boneIndex; ///< Bone index
		uint8_t		m_group; ///< Group
		uint16_t	m_collisionGroup; ///< Collision group

		/**
		 * @brief Represents the shape of a rigidbody.
		 */
		enum class Shape : uint8_t
		{
			Sphere, ///< Sphere
			Box, ///< Box
			Capsule, ///< Capsule
		};
		Shape		m_shape; ///< Shape
		glm::vec3	m_shapeSize; ///< Shape size

		glm::vec3	m_translate; ///< Translation vector
		glm::vec3	m_rotate; ///< Rotation vector (in radians)

		float	m_mass; ///< Mass
		float	m_translateDimmer; ///< Translation dimmer
		float	m_rotateDimmer; ///< Rotation dimmer
		float	m_repulsion; ///< Repulsion
		float	m_friction; ///< Friction

		/**
		 * @brief Represents the operation type of a rigidbody.
		 */
		enum class Operation : uint8_t
		{
			Static, ///< Static
			Dynamic, ///< Dynamic
			DynamicAndBoneMerge ///< Dynamic and bone merge
		};
		Operation	m_op; ///< Operation type
	};

	/**
	 * @brief Represents a joint in a PMX file.
	 */
	struct PMXJoint
	{
		std::string	m_name; ///< Joint name
		std::string	m_englishName; ///< English joint name

		/**
		 * @brief Represents the joint type.
		 */
		enum class JointType : uint8_t
		{
			SpringDOF6, ///< Spring DOF6
			DOF6, ///< DOF6
			P2P, ///< P2P
			ConeTwist, ///< Cone twist
			Slider, ///< Slider
			Hinge, ///< Hinge
		};
		JointType	m_type; ///< Joint type
		int32_t		m_rigidbodyAIndex; ///< Rigidbody A index
		int32_t		m_rigidbodyBIndex; ///< Rigidbody B index

		glm::vec3	m_translate; ///< Translation vector
		glm::vec3	m_rotate; ///< Rotation vector

		glm::vec3	m_translateLowerLimit; ///< Translation lower limit
		glm::vec3	m_translateUpperLimit; ///< Translation upper limit
		glm::vec3	m_rotateLowerLimit; ///< Rotation lower limit
		glm::vec3	m_rotateUpperLimit; ///< Rotation upper limit

		glm::vec3	m_springTranslateFactor; ///< Spring translation factor
		glm::vec3	m_springRotateFactor; ///< Spring rotation factor
	};

	/**
	 * @brief Represents a softbody in a PMX file.
	 */
	struct PMXSoftbody
	{
		std::string	m_name; ///< Softbody name
		std::string	m_englishName; ///< English softbody name

		/**
		 * @brief Represents the softbody type.
		 */
		enum class SoftbodyType : uint8_t
		{
			TriMesh, ///< TriMesh
			Rope, ///< Rope
		};
		SoftbodyType	m_type; ///< Softbody type

		int32_t			m_materialIndex; ///< Material index

		uint8_t		m_group; ///< Group
		uint16_t	m_collisionGroup; ///< Collision group

		/**
		 * @brief Represents the softbody mask.
		 */
		enum class SoftbodyMask : uint8_t
		{
			BLink = 0x01, ///< B-Link
			Cluster = 0x02, ///< Cluster
			HybridLink = 0x04, ///< Hybrid link
		};
		SoftbodyMask	m_flag; ///< Softbody mask

		int32_t	m_BLinkLength; ///< B-Link length
		int32_t	m_numClusters; ///< Number of clusters

		float	m_totalMass; ///< Total mass
		float	m_collisionMargin; ///< Collision margin

		/**
		 * @brief Represents the aero model.
		 */
		enum class AeroModel : int32_t
		{
			kAeroModelV_TwoSided, ///< V_TwoSided
			kAeroModelV_OneSided, ///< V_OneSided
			kAeroModelF_TwoSided, ///< F_TwoSided
			kAeroModelF_OneSided, ///< F_OneSided
		};
		int32_t		m_aeroModel; ///< Aero model

		//config
		float	m_VCF; ///< VCF
		float	m_DP; ///< DP
		float	m_DG; ///< DG
		float	m_LF; ///< LF
		float	m_PR; ///< PR
		float	m_VC; ///< VC
		float	m_DF; ///< DF
		float	m_MT; ///< MT
		float	m_CHR; ///< CHR
		float	m_KHR; ///< KHR
		float	m_SHR; ///< SHR
		float	m_AHR; ///< AHR

		//cluster
		float	m_SRHR_CL; ///< SRHR_CL
		float	m_SKHR_CL; ///< SKHR_CL
		float	m_SSHR_CL; ///< SSHR_CL
		float	m_SR_SPLT_CL; ///< SR_SPLT_CL
		float	m_SK_SPLT_CL; ///< SK_SPLT_CL
		float	m_SS_SPLT_CL; ///< SS_SPLT_CL

		//interation
		int32_t	m_V_IT; ///< V_IT
		int32_t	m_P_IT; ///< P_IT
		int32_t	m_D_IT; ///< D_IT
		int32_t	m_C_IT; ///< C_IT

		//material
		float	m_LST; ///< LST
		float	m_AST; ///< AST
		float	m_VST; ///< VST

		struct AnchorRigidbody
		{
			int32_t		m_rigidBodyIndex; ///< Rigidbody index
			int32_t		m_vertexIndex; ///< Vertex index
			uint8_t	m_nearMode; ///< Near mode (0:FF 1:ON)
		};
		std::vector<AnchorRigidbody>	m_anchorRigidbodies; ///< Anchor rigidbodies

		std::vector<int32_t>	m_pinVertexIndices; ///< Pin vertex indices
	};

	/**
	 * @brief Represents a PMX file.
	 */
	struct PMXFile
	{
		PMXHeader	m_header; ///< Header
		PMXInfo		m_info; ///< Information

		std::vector<PMXVertex>		m_vertices; ///< Vertices
		std::vector<PMXFace>		m_faces; ///< Faces
		std::vector<PMXTexture>		m_textures; ///< Textures
		std::vector<PMXMaterial>	m_materials; ///< Materials
		std::vector<PMXBone>		m_bones; ///< Bones
		std::vector<PMXFileMorph>		m_morphs; ///< Morphs
		std::vector<PMXDisplayFrame>	m_displayFrames; ///< Display frames
		std::vector<PMXRigidbody>	m_rigidbodies; ///< Rigidbodies
		std::vector<PMXJoint>		m_joints; ///< Joints
		std::vector<PMXSoftbody>	m_softbodies; ///< Softbodies
	};

	/**
	 * @brief Reads a PMX file.
	 * @param pmxFile Pointer to the PMXFile structure to store the read data.
	 * @param filename The name of the file to read.
	 * @return True if the file was read successfully, false otherwise.
	 */
	bool ReadPMXFile(PMXFile* pmxFile, const char* filename);

	/**
	 * @brief Writes a PMX file.
	 * @param pmxFile Pointer to the PMXFile structure containing the data to write.
	 * @param filename The name of the file to write.
	 * @return True if the file was written successfully, false otherwise.
	 */
	bool WritePMXFile(const PMXFile* pmxFile, const char* filename);
}

#endif // !SABA_MODEL_PMXFILE_H_
