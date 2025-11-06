#pragma once
#include "Camera.h"
#include "FlyFish.h"
#include "structs.h"

struct Plane
{
	OneBlade PlaneGenerators;
	Color4f Color;
};

struct Sphere
{
	
};

struct LightRay
{
	
};

inline bool HitPlane(const TwoBlade& line, const Plane& plane, const Camera* pCamera, float& outDistance)
{
	const auto hit = line ^ plane.PlaneGenerators;
	
	if (hit.IsZero() || hit.Norm() == 0.0f)
	{
		return false;
	}

	auto toHitDistance = (pCamera->GetOrigin() & hit).Norm();

	if (toHitDistance > 0.001f)
	{
		outDistance = toHitDistance;
		return true;
	}

	return false;
}