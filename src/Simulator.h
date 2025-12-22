#pragma once
#include "BlackHole.h"
#include "GEOAUtils.h"
#include "LightParticle.h"
#include <vector>
#include <limits>

class Camera;

struct HitResult
{
	bool hit{ false };
	float distanceSq{ std::numeric_limits<float>::max() };
	Color4f color{ 0.3f, 0.3f, 0.3f, 1.0f };
};

class Simulator
{
public:
	Simulator(BlackHole blackHole, double fixedTimeStep, bool useEOptimization, bool useDeltaTime);

	void UpdateParticleRK4(LightParticle& particle, const TriVector& cameraPos, double deltaTime);
	void Update(float elapsedSec, const TriVector& cameraPos);
	void SpawnLightParticle(const TriVector& position, const TriVector& direction);

	HitResult TestRay(const BiVector& ray, const Camera* pCamera) const;

private:

	BlackHole m_BlackHole;
	Sphere m_BlackHoleSphere;
	std::vector<LightParticle> m_LightParticles;
	double m_FixedTimeStep;
	bool m_UseEOptimization;
	bool m_UseDeltaTime;
};
