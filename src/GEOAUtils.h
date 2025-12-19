#pragma once
#include "Camera.h"
#include "FlyFish.h"
#include "structs.h"

struct Plane
{
	Vector PlaneGenerators;
	Color4f Color;
};

struct Sphere
{
	TriVector Origin;
	float Radius;
	Color4f Color;
};

struct LightRay
{
	
};

inline bool HitPlane(const BiVector& line, const Plane& plane, const Camera* pCamera, float& outDistance)
{
	const auto hit = line ^ plane.PlaneGenerators;

	if (hit[3] < 0.0f)
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

inline bool HitSphere(const BiVector& line, const Sphere& sphere, const Camera* pCamera, float& outShading)
{
    TriVector sphereCenter = sphere.Origin.Normalized();

    Vector closestPlane = line | sphereCenter;
    TriVector closestPoint = (closestPlane ^ line);

    if (std::abs(closestPoint.e123()) < 0.0001f) return false;

    TriVector closestNorm = closestPoint.Normalized();

    TriVector camPos = pCamera->GetOrigin().Normalized();

    BiVector toSphere = sphereCenter & camPos;
    if ((toSphere | line) < 0) return false;  // Sphere is behind camera

    float distToCenter = (closestNorm & sphereCenter).Norm();

    if (distToCenter > sphere.Radius) return false;

	// will be removed, only for testing purposes
    outShading = 1.0f - (distToCenter / sphere.Radius);

    return true;
}

// Think about combining these methods

inline bool HitPoint(const BiVector& line, const TriVector& point, float radius, const TriVector& camPos)
{
    Vector closestPlane = line | point;
    TriVector closestPoint = (closestPlane ^ line);

    if (std::abs(closestPoint.e123()) < 0.0001f) return false;

    TriVector closestNorm = closestPoint.Normalized();

    BiVector toPoint = point & camPos;
    if ((toPoint | line) < 0) return false;

    float dist = (closestNorm & point).Norm();
    return dist <= radius;
}

inline bool HitBounds(const BiVector& line, const TriVector& center, float radius, const TriVector& camPos)
{
    BiVector toCenter = center & camPos;
    if ((toCenter | line) < 0) return false;

    Vector closestPlane = line | center;
    TriVector closestPoint = (closestPlane ^ line);

    if (std::abs(closestPoint.e123()) < 0.0001f) return false;

    TriVector closestNorm = closestPoint.Normalized();
    float dist = (closestNorm & center).Norm();

    return dist <= radius;
}