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

inline bool HitSphere(const BiVector& line, const Sphere& sphere, const Camera* pCamera, float& outDistanceSq)
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

    float dx = sphereCenter.e032() - camPos.e032();
    float dy = sphereCenter.e013() - camPos.e013();
    float dz = sphereCenter.e021() - camPos.e021();
    outDistanceSq = dx*dx + dy*dy + dz*dz;

    return true;
}

// Think about combining these methods

inline bool HitPoint(const BiVector& line, const TriVector& point, float radius, const TriVector& camPos, float& outDistanceSq)
{
    TriVector pointNorm = point.Normalized();

    Vector closestPlane = line | pointNorm;
    TriVector closestPoint = (closestPlane ^ line);

    if (std::abs(closestPoint.e123()) < 0.0001f) return false;

    TriVector closestNorm = closestPoint.Normalized();

    BiVector toPoint = pointNorm & camPos;
    if ((toPoint | line) < 0) return false;

    float dist = (closestNorm & pointNorm).Norm();
    if (dist > radius) return false;

    float dx = pointNorm.e032() - camPos.e032();
    float dy = pointNorm.e013() - camPos.e013();
    float dz = pointNorm.e021() - camPos.e021();
    outDistanceSq = dx*dx + dy*dy + dz*dz;

    return true;
}

// This should be removed in favor of making a proper AABB check

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