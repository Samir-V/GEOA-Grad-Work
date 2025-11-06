#pragma once
#include "FlyFish.h"

class Camera
{
public:
	Camera() = default;

	Camera(ThreeBlade origin, float _fovAngle) :
		m_Origin{std::move(origin)},
		m_FovAngle{ _fovAngle }
	{
		m_Transform = Motor{1, 0, 0, 0, 0, 0, 0, 1};
	}

	const ThreeBlade& GetOrigin() const;
	float GetFOVAngle() const;
	ThreeBlade CameraToWorldPoint(const ThreeBlade& point) const;
	TwoBlade CameraToWorldLine(const TwoBlade& line) const;

private:
	ThreeBlade m_Origin{};
	float m_FovAngle{ 60.f };

	Motor m_Transform;
};
