#pragma once
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

inline bool HitPlane(const TwoBlade& ray, const Plane& plane)
{
	const auto hit = ray ^ plane.PlaneGenerators;
	
	if (hit.IsZero() || hit.Norm() == 0.0f)
	{
		return false;
	}

	return true;
}