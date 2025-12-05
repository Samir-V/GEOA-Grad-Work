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

	if (hit.IsZero() || hit[3] < 0.0f)
	{
		return false;
	}

	auto lineToHit = pCamera->GetOrigin() & hit;
	float toHitDistance = lineToHit.Norm() / hit[3];

	if (toHitDistance > 0.001f && toHitDistance < 1000.0f)
	{
		outDistance = toHitDistance;
		return true;
	}

	return false;
}