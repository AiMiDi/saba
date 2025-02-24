//
// Copyright(c) 2016-2019 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "VMDCameraAnimation.h"
#include "VMDAnimationCommon.hpp"

#include <cstdint>
#include <glm/gtc/matrix_transform.hpp>

namespace saba
{
	namespace
	{
		/**
		 * Helper function to set up VMD Bezier curve control points
		 * Converts control points from VMD format (0-127) to normalized space (0-1)
		 */
		void SetVMDBezier(VMDBezier& bezier, const int x0, const int x1, const int y0, const int y1)
		{
			bezier.m_cp1 = glm::vec2(static_cast<float>(x0) / 127.0f, static_cast<float>(y0) / 127.0f);
			bezier.m_cp2 = glm::vec2(static_cast<float>(x1) / 127.0f, static_cast<float>(y1) / 127.0f);
		}
	} // namespace

	/**
	 * Structure to hold camera animation keyframe data
	 * Contains position, rotation, distance, FOV and interpolation curves
	 */
	struct VMDCameraAnimationKey
	{
		int32_t		m_time;         // Keyframe timestamp
		glm::vec3	m_interest;     // Look-at point
		glm::vec3	m_rotate;       // Camera rotation
		float		m_distance;     // Distance from look-at point
		float		m_fov;          // Field of view in radians

		// Bezier interpolation curves for each parameter
		VMDBezier	m_ixBezier;     // Interest point X
		VMDBezier	m_iyBezier;     // Interest point Y
		VMDBezier	m_izBezier;     // Interest point Z
		VMDBezier	m_rotateBezier; // Rotation
		VMDBezier	m_distanceBezier; // Distance
		VMDBezier	m_fovBezier;    // Field of view
	};

	/**
	 * Controller class for camera animation
	 * Handles interpolation and evaluation of camera keyframes
	 */
	class VMDCameraController
	{
	public:
		using KeyType = VMDCameraAnimationKey;

		VMDCameraController(): m_startKeyIndex(0)
		{
		}

		// Core animation functions
		void Evaluate(float t);
		void SortKeys();
		
		// Keyframe management
		void AddKey(const KeyType& key)
		{
			m_keys.push_back(key);
		}
		size_t GetKeyCount() const
		{
			return m_keys.size();
		}
		int32_t GetMaxKeyTime() const;

		// Getters
		const MMDCamera& GetCamera() const
		{
			return m_camera;
		}
		const std::vector<KeyType>& GetKeys() const
		{
			return m_keys;
		}

	private:
		std::vector<VMDCameraAnimationKey>	m_keys;          // Animation keyframes
		MMDCamera							m_camera;        // Current camera state
		size_t								m_startKeyIndex;  // Optimization: last evaluated key index
	};

	void VMDCameraController::Evaluate(const float t)
	{
		if (m_keys.empty())
		{
			return;
		}

		// Find the appropriate keyframe for the current time
		const auto boundIt = FindBoundKey(m_keys, static_cast<int32_t>(t), m_startKeyIndex);
		
		if (boundIt == std::end(m_keys))
		{
			// Past the last keyframe - use final values
			const auto& lastKey = m_keys[m_keys.size() - 1];
			m_camera.m_interest = lastKey.m_interest;
			m_camera.m_rotate = lastKey.m_rotate;
			m_camera.m_distance = lastKey.m_distance;
			m_camera.m_fov = lastKey.m_fov;
		}
		else
		{
			// Set initial values from current keyframe
			const auto& currentKey = *boundIt;
			m_camera.m_interest = currentKey.m_interest;
			m_camera.m_rotate = currentKey.m_rotate;
			m_camera.m_distance = currentKey.m_distance;
			m_camera.m_fov = currentKey.m_fov;

			// Interpolate between keyframes if not at the start
			if (boundIt != std::begin(m_keys))
			{
				const auto& key0 = *(boundIt - 1);
				const auto& key1 = *boundIt;

				// Check if keyframes are far enough apart for interpolation
				// If frames are 1 frame apart, treat as a camera cut (no interpolation)
				if (key1.m_time - key0.m_time > 1)
				{
					// Calculate interpolation parameters
					const auto timeRange = static_cast<float>(key1.m_time - key0.m_time);
					const auto time = (t - static_cast<float>(key0.m_time)) / timeRange;

					// Find interpolation values using Bezier curves
					const auto ix_x = key0.m_ixBezier.FindBezierX(time);
					const auto iy_x = key0.m_iyBezier.FindBezierX(time);
					const auto iz_x = key0.m_izBezier.FindBezierX(time);
					const auto rotate_x = key0.m_rotateBezier.FindBezierX(time);
					const auto distance_x = key0.m_distanceBezier.FindBezierX(time);
					const auto fov_x = key0.m_fovBezier.FindBezierX(time);

					// Evaluate final interpolation values
					const auto ix_y = key0.m_ixBezier.EvalY(ix_x);
					const auto iy_y = key0.m_iyBezier.EvalY(iy_x);
					const auto iz_y = key0.m_izBezier.EvalY(iz_x);
					const auto rotate_y = key0.m_rotateBezier.EvalY(rotate_x);
					const auto distance_y = key0.m_distanceBezier.EvalY(distance_x);
					const auto fov_y = key0.m_fovBezier.EvalY(fov_x);

					// Interpolate camera parameters
					m_camera.m_interest = mix(key0.m_interest, key1.m_interest, glm::vec3(ix_y, iy_y, iz_y));
					m_camera.m_rotate = mix(key0.m_rotate, key1.m_rotate, rotate_y);
					m_camera.m_distance = glm::mix(key0.m_distance, key1.m_distance, distance_y);
					m_camera.m_fov = glm::mix(key0.m_fov, key1.m_fov, fov_y);
				}
				else
				{
					// Camera cut - use previous keyframe values without interpolation
					m_camera.m_interest = key0.m_interest;
					m_camera.m_rotate = key0.m_rotate;
					m_camera.m_distance = key0.m_distance;
					m_camera.m_fov = key0.m_fov;
				}

				m_startKeyIndex = std::distance(m_keys.cbegin(), boundIt);
			}
		}
	}

	void VMDCameraController::SortKeys()
	{
		std::sort(
			std::begin(m_keys),
			std::end(m_keys),
			[](const KeyType& a, const KeyType& b) { return a.m_time < b.m_time; }
		);
	}

	int32_t VMDCameraController::GetMaxKeyTime() const
	{
		return m_keys.empty() ? 0 : m_keys.back().m_time;
	}

	VMDCameraAnimation::VMDCameraAnimation()
	{
		Destroy();
	}

	bool VMDCameraAnimation::Create(const VMDFile & vmd)
	{
		if (vmd.m_cameras.empty())
		{
			return false;
		}

		m_cameraController = std::make_unique<VMDCameraController>();
		for (const auto& [m_frame, m_distance, m_interest, m_rotate, m_interpolation, m_viewAngle, m_isPerspective] : vmd.m_cameras)
		{
			VMDCameraAnimationKey key{};
			key.m_time = static_cast<int32_t>(m_frame);
			key.m_interest = m_interest * glm::vec3(1, 1, -1);
			key.m_rotate = m_rotate;
			key.m_distance = m_distance;
			key.m_fov = glm::radians(static_cast<float>(m_viewAngle));

			const uint8_t* ip = m_interpolation.data();
			SetVMDBezier(key.m_ixBezier, ip[0], ip[1], ip[2], ip[3]);
			SetVMDBezier(key.m_iyBezier, ip[4], ip[5], ip[6], ip[7]);
			SetVMDBezier(key.m_izBezier, ip[8], ip[9], ip[10], ip[11]);
			SetVMDBezier(key.m_rotateBezier, ip[12], ip[13], ip[14], ip[15]);
			SetVMDBezier(key.m_distanceBezier, ip[16], ip[17], ip[18], ip[19]);
			SetVMDBezier(key.m_fovBezier, ip[20], ip[21], ip[22], ip[23]);

			m_cameraController->AddKey(key);
		}
		m_cameraController->SortKeys();
		return true;
	}

	void VMDCameraAnimation::Destroy()
	{
		m_cameraController.reset();
	}

	void VMDCameraAnimation::Evaluate(const float t)
	{
		m_cameraController->Evaluate(t);
		m_camera = m_cameraController->GetCamera();
	}

	size_t VMDCameraAnimation::GetKeyCount() const
	{ 
		return m_cameraController->GetKeyCount();
	}

	int32_t VMDCameraAnimation::GetMaxKeyTime() const
	{ 
		return m_cameraController->GetMaxKeyTime(); 
	}
}

