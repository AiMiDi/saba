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
		void SetVMDBezier(VMDBezier& bezier, const int x0, const int x1, const int y0, const int y1)
		{
			bezier.m_cp1 = glm::vec2(static_cast<float>(x0) / 127.0f, static_cast<float>(y0) / 127.0f);
			bezier.m_cp2 = glm::vec2(static_cast<float>(x1) / 127.0f, static_cast<float>(y1) / 127.0f);
		}
	} // namespace

	struct VMDCameraAnimationKey
	{
		int32_t		m_time;
		glm::vec3	m_interest;
		glm::vec3	m_rotate;
		float		m_distance;
		float		m_fov;

		VMDBezier	m_ixBezier;
		VMDBezier	m_iyBezier;
		VMDBezier	m_izBezier;
		VMDBezier	m_rotateBezier;
		VMDBezier	m_distanceBezier;
		VMDBezier	m_fovBezier;
	};

	class VMDCameraController
	{
	public:
		using KeyType = VMDCameraAnimationKey;

		VMDCameraController(): m_startKeyIndex(0)
		{
		}

		void Evaluate(float t);
		const MMDCamera& GetCamera() const { return m_camera; }
		const std::vector<KeyType>& GetKeys() const { return m_keys; }
		size_t GetKeyCount() const { return m_keys.size(); }
		int32_t GetMaxKeyTime() const;

		void AddKey(const KeyType& key)
		{
			m_keys.push_back(key);
		}
		void SortKeys();

	private:
		std::vector<VMDCameraAnimationKey>	m_keys;
		MMDCamera							m_camera;
		size_t								m_startKeyIndex;
	};

	void VMDCameraController::Evaluate(const float t)
	{
		if (m_keys.empty())
		{
			return;
		}

		if (const auto boundIt = FindBoundKey(m_keys, static_cast<int32_t>(t), m_startKeyIndex); boundIt == std::end(m_keys))
		{
			const auto& [m_time, m_interest, m_rotate, m_distance, m_fov, m_ixBezier, m_iyBezier, m_izBezier, m_rotateBezier, m_distanceBezier, m_fovBezier] = m_keys[m_keys.size() - 1];
			m_camera.m_interest = m_interest;
			m_camera.m_rotate = m_rotate;
			m_camera.m_distance = m_distance;
			m_camera.m_fov = m_fov;
		}
		else
		{
			const auto& [m_time, m_interest, m_rotate, m_distance, m_fov, m_ixBezier, m_iyBezier, m_izBezier, m_rotateBezier, m_distanceBezier, m_fovBezier] = *boundIt;
			m_camera.m_interest = m_interest;
			m_camera.m_rotate = m_rotate;
			m_camera.m_distance = m_distance;
			m_camera.m_fov = m_fov;
			if (boundIt != std::begin(m_keys))
			{
				const auto& key0 = *(boundIt - 1);
				const auto& key1 = *boundIt;

				if (key1.m_time - key0.m_time > 1)
				{
					const auto timeRange = static_cast<float>(key1.m_time - key0.m_time);
					const auto time = (t - static_cast<float>(key0.m_time)) / timeRange;
					const auto ix_x = key0.m_ixBezier.FindBezierX(time);
					const auto iy_x = key0.m_iyBezier.FindBezierX(time);
					const auto iz_x = key0.m_izBezier.FindBezierX(time);
					const auto rotate_x = key0.m_rotateBezier.FindBezierX(time);
					const auto distance_x = key0.m_distanceBezier.FindBezierX(time);
					const auto fov_x = key0.m_fovBezier.FindBezierX(time);

					const auto ix_y = key0.m_ixBezier.EvalY(ix_x);
					const auto iy_y = key0.m_iyBezier.EvalY(iy_x);
					const auto iz_y = key0.m_izBezier.EvalY(iz_x);
					const auto rotate_y = key0.m_rotateBezier.EvalY(rotate_x);
					const auto distance_y = key0.m_distanceBezier.EvalY(distance_x);
					const auto fov_y = key0.m_fovBezier.EvalY(fov_x);

					m_camera.m_interest = mix(key0.m_interest, key1.m_interest, glm::vec3(ix_y, iy_y, iz_y));
					m_camera.m_rotate = mix(key0.m_rotate, key1.m_rotate, rotate_y);
					m_camera.m_distance = glm::mix(key0.m_distance, key1.m_distance, distance_y);
					m_camera.m_fov = glm::mix(key0.m_fov, key1.m_fov, fov_y);
				}
				else
				{
					/*
					カメラアニメーションでキーが1フレーム間隔で打たれている場合、
					カメラの切り替えと判定し補間を行わないようにする（key0を使用する）
					*/
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

	VMDCameraAnimation::VMDCameraAnimation()
	{
		Destroy();
	}

	bool VMDCameraAnimation::Create(const VMDFile & vmd)
	{
		if (!vmd.m_cameras.empty())
		{
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
		}
		else
		{
			return false;
		}
		return true;
	}

	int32_t VMDCameraController::GetMaxKeyTime() const
	{
		if (m_keys.empty())
		{
			return 0;
		}
		return m_keys.back().m_time;
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

