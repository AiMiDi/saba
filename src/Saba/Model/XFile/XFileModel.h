//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//


#ifndef SABA_MODEL_XFILE_OBJMODEL_H_
#define SABA_MODEL_XFILE_OBJMODEL_H_

#include <vector>
#include <memory>
#include <string>
#include <glm/mat4x4.hpp>

namespace saba
{
	class XFileModel
	{
	public:
		/**
		 * @brief Material structure representing material properties.
		 */
		struct Material
		{
			glm::vec4	m_diffuse; ///< Diffuse color.
			glm::vec3	m_specular; ///< Specular color.
			float		m_speculatPower; ///< Specular power.
			glm::vec3	m_emissive; ///< Emissive color.

			std::string	m_texture; ///< Texture file path.

			/**
			 * @brief Enum representing specular texture mode.
			 */
			enum class SpTextureMode
			{
				None, ///< No specular texture.
				Mul, ///< Multiply specular texture.
				Add, ///< Add specular texture.
			};
			SpTextureMode	m_spTextureMode; ///< Specular texture mode.
			std::string	m_spTexture; ///< Specular texture file path.
		};

		/**
		 * @brief Face structure representing a face in the mesh.
		 */
		struct Face
		{
			int	m_position[3]; ///< Indices of the positions.
			int	m_normal[3]; ///< Indices of the normals.
			int	m_uv[3]; ///< Indices of the UV coordinates.
			int	m_material; ///< Index of the material.
		};

		/**
		 * @brief Mesh structure representing a mesh in the model.
		 */
		struct Mesh
		{
			std::string				m_name; ///< Name of the mesh.
			std::vector<glm::vec3>	m_positions; ///< Positions of the vertices.
			std::vector<glm::vec3>	m_normals; ///< Normals of the vertices.
			std::vector<glm::vec2>	m_uvs; ///< UV coordinates of the vertices.
			std::vector<Material>	m_materials; ///< Materials of the mesh.
			std::vector<Face>		m_faces; ///< Faces of the mesh.
		};

		/**
		 * @brief Frame structure representing a frame in the model.
		 */
		struct Frame
		{
			std::string	m_name; ///< Name of the frame.
			glm::mat4	m_local; ///< Local transformation matrix.
			glm::mat4	m_global; ///< Global transformation matrix.
			Mesh*		m_mesh; ///< Pointer to the mesh.

			Frame*		m_parent; ///< Pointer to the parent frame.
			Frame*		m_child; ///< Pointer to the child frame.
			Frame*		m_next; ///< Pointer to the next sibling frame.
		};

		/**
		 * @brief Load the XFile model from a file.
		 * @param filepath Path to the XFile.
		 * @return True if loading is successful, false otherwise.
		 */
		bool Load(const char* filepath);

		/**
		 * @brief Destroy the XFile model.
		 */
		void Destroy();

		/**
		 * @brief Get the number of frames in the model.
		 * @return Number of frames.
		 */
		size_t GetFrameCount() const { return m_frames.size(); }

		/**
		 * @brief Get a frame by index.
		 * @param i Index of the frame.
		 * @return Pointer to the frame.
		 */
		const Frame* GetFrame(const size_t i) const { return m_frames[i].get(); }

		/**
		 * @brief Get the bounding box minimum coordinates.
		 * @return Bounding box minimum coordinates.
		 */
		const glm::vec3& GetBBoxMin() const { return m_bboxMin; }

		/**
		 * @brief Get the bounding box maximum coordinates.
		 * @return Bounding box maximum coordinates.
		 */
		const glm::vec3& GetBBoxMax() const { return m_bboxMax; }

	private:
		static void UpdateGlobalTransform(Frame* frame);

		using MeshPtr = std::unique_ptr<Mesh>;
		using FramePtr = std::unique_ptr<Frame>;

		std::vector<MeshPtr>	m_meshes;
		std::vector<FramePtr>	m_frames;

		glm::vec3		m_bboxMin{};
		glm::vec3		m_bboxMax{};

	};
}

#endif // !SABA_MODEL_XFILE_OBJMODEL_H_
