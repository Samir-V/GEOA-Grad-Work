#pragma once
#include "FlyFish.h"

class Camera
{
public:
	Camera() = default;

	Camera(TriVector origin, float _fovAngle) :
		m_Origin{std::move(origin)},
		m_FovAngle{ _fovAngle }
	{
		m_Transform = Motor{1, 0, 0, 0, 0, 0, 0, 0};
	}

	const TriVector& GetOrigin() const;
	float GetFOVAngle() const;
	TriVector CameraToWorldPoint(const TriVector& point) const;
	BiVector CameraToWorldLine(const BiVector& line) const;

private:
	TriVector m_Origin{};
	float m_FovAngle{ 60.f };

	Motor m_Transform;
};
