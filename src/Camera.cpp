#include "Camera.h"
#include "FlyFish.h"
#include <algorithm>

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

float Camera::GetFOVAngle() const
{
	return m_FovAngle;
}

void Camera::Rotate(float deltaYaw, float deltaPitch)
{
	m_Yaw += deltaYaw * MouseSensitivity;
	m_Pitch += deltaPitch * MouseSensitivity;

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

void Camera::Update()
{
}

void Camera::UpdateTransform()
{
	BiVector yawAxis{0, 0, 0, 0, 1, 0};  
	Motor yawRotation = Motor::Rotation(m_Yaw, yawAxis);

	BiVector pitchAxis{0, 0, 0, 1, 0, 0}; 
	Motor pitchRotation = Motor::Rotation(m_Pitch, pitchAxis);

	Motor rotation = pitchRotation * yawRotation;

	float tx = m_Origin.e032();
	float ty = m_Origin.e013();
	float tz = m_Origin.e021();

	Motor translation{1, -tx/2, -ty/2, -tz/2, 0, 0, 0, 0};

	m_Transform = translation * rotation;
}
