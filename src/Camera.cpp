#include "Camera.h"

const ThreeBlade& Camera::GetOrigin() const
{
	return m_Origin; 
}


ThreeBlade Camera::CameraToWorldPoint(const ThreeBlade& point) const
{
	return (m_FinalTransform * point * ~m_FinalTransform).Grade3();
}

TwoBlade Camera::CameraToWorldLine(const TwoBlade& line) const
{
	return (m_FinalTransform * line * ~m_FinalTransform).Grade2();
}

float Camera::GetFOVAngle() const
{
	return m_FovAngle;
}
