#include "Camera.h"

const ThreeBlade& Camera::GetOrigin() const
{
	return m_Origin; 
}


ThreeBlade Camera::CameraToWorldPoint(const ThreeBlade& point) const
{
	return (m_Transform * point * ~m_Transform).Grade3();
}

TwoBlade Camera::CameraToWorldLine(const TwoBlade& line) const
{
	return (m_Transform * line * ~m_Transform).Grade2();
}

float Camera::GetFOVAngle() const
{
	return m_FovAngle;
}
