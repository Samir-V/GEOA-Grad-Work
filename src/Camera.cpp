#include "Camera.h"

#include <algorithm>
#include "FlyFish.h"

const TriVector& Camera::GetOrigin() const
{
	return m_Origin;
}


TriVector Camera::CameraToWorldPoint(const TriVector& point) const
{
	return (m_Transform * point * ~m_Transform).Grade3();
}

BiVector Camera::CameraToWorldLine(const BiVector& line) const
{
	return (m_Transform * line * ~m_Transform).Grade2();
}

bool Camera::WorldToScreen(const TriVector& point, int screenWidth, int screenHeight,
	float fov, float aspectRatio, int& outScreenX, int& outScreenY) const
{
	TriVector screenPoint = (~m_Transform * point * m_Transform).Grade3();

	const float camX = screenPoint.e032();
	const float camY = screenPoint.e013();
	const float camZ = screenPoint.e021();

	if (camZ <= 0.01f) return false;

	outScreenX = static_cast<int>((camX / camZ / (aspectRatio * fov) + 1.0f) * 0.5f * screenWidth);
	outScreenY = static_cast<int>((1.0f - camY / camZ / fov) * 0.5f * screenHeight);

	return true;
}

float Camera::GetFOVAngle() const
{
	return m_FovAngle;
}

void Camera::Rotate(float deltaYaw, float deltaPitch)
{
	m_Yaw += deltaYaw * MouseSensitivity;
	m_Pitch += deltaPitch * MouseSensitivity;

	m_Pitch = std::min(m_Pitch, 89.0f);
	m_Pitch = std::max(m_Pitch, -89.0f);

	UpdateTransform();
}

void Camera::Move(float forward, float right, float up, float elapsedSec)
{
	BiVector moveDir{ right, up, forward, 0, 0, 0 };

	BiVector worldMove = CameraToWorldLine(moveDir);
	Motor translator = Motor::Translation(MoveSpeed * elapsedSec, worldMove);

	m_Origin = (translator * m_Origin * ~translator).Grade3();

	UpdateTransform();
}

void Camera::UpdateTransform()
{
	BiVector yawAxis{0, 0, 0, 0, 1, 0};
	Motor yawRotation = Motor::Rotation(m_Yaw, yawAxis);

	BiVector pitchAxisLocal{0, 0, 0, 1, 0, 0};
	BiVector pitchAxisRotated = (yawRotation * pitchAxisLocal * ~yawRotation).Grade2();
	Motor pitchRotation = Motor::Rotation(m_Pitch, pitchAxisRotated);

	Motor rotation = pitchRotation * yawRotation;

	BiVector direction{ m_Origin.e032(), m_Origin.e013(), m_Origin.e021(), 0, 0, 0 };

	if (direction.VNorm() == 0.0f)
	{
		m_Transform = rotation;
		return;
	}

	Motor translation = Motor::Translation(direction.VNorm(), direction);
	m_Transform = translation * rotation;
}
