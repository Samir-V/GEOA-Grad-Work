#include "Camera.h"
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

float Camera::GetFOVAngle() const
{
	return m_FovAngle;
}
