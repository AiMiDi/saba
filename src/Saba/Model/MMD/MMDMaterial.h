//
// Copyright(c) 2016-2017 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#ifndef SABA_MODEL_MMD_MMDMATERIAL_H_
#define SABA_MODEL_MMD_MMDMATERIAL_H_

#include <string>
#include <cstdint>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace saba
{
	/**
	 * @brief Represents a material in MMD.
	 */
	struct MMDMaterial
	{
		MMDMaterial();

		/**
		 * @brief Enum for sphere texture mode.
		 */
		enum class SphereTextureMode
		{
			None, ///< No sphere texture
			Mul,  ///< Multiply mode
			Add,  ///< Add mode
		};

		glm::vec3		m_diffuse; ///< Diffuse color
		float			m_alpha; ///< Alpha value
		glm::vec3		m_specular; ///< Specular color
		float			m_specularPower; ///< Specular power
		glm::vec3		m_ambient; ///< Ambient color
		uint8_t			m_edgeFlag; ///< Edge flag
		float			m_edgeSize; ///< Edge size
		glm::vec4		m_edgeColor; ///< Edge color
		std::string		m_texture; ///< Texture file path
		std::string		m_spTexture; ///< Sphere texture file path
		SphereTextureMode	m_spTextureMode; ///< Sphere texture mode
		std::string		m_toonTexture; ///< Toon texture file path
		glm::vec4		m_textureMulFactor; ///< Texture multiply factor
		glm::vec4		m_spTextureMulFactor; ///< Sphere texture multiply factor
		glm::vec4		m_toonTextureMulFactor; ///< Toon texture multiply factor
		glm::vec4		m_textureAddFactor; ///< Texture add factor
		glm::vec4		m_spTextureAddFactor; ///< Sphere texture add factor
		glm::vec4		m_toonTextureAddFactor; ///< Toon texture add factor
		bool			m_bothFace; ///< Both face flag
		bool			m_groundShadow; ///< Ground shadow flag
		bool			m_shadowCaster; ///< Shadow caster flag
		bool			m_shadowReceiver; ///< Shadow receiver flag
	};
}

#endif // !SABA_MODEL_MMD_MMDMATERIAL_H_
