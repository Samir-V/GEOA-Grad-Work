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

struct ScreenBounds
{
	int minX{ 0 };
	int maxX{ 0 };
	int minY{ 0 };
	int maxY{ 0 };
	bool visible{ false };
};

class Simulator
{
public:
	Simulator(BlackHole blackHole);

	void UpdateParticleRK4(LightParticle& particle, const TriVector& cameraPos, float deltaTime);
	void Update(float elapsedSec, const TriVector& cameraPos);
	void SpawnLightParticle(const TriVector& position, const BiVector& direction);
	void UpdateScreenBounds(const Camera* pCamera, int screenWidth, int screenHeight, float fov, float aspectRatio);
	HitResult TestRayAtPixel(const BiVector& ray, const Camera* pCamera, int px, int py) const;

	static constexpr float SimulationSpeed{ 20 };
	static constexpr float MaxDistanceSq = 50.0f * 50.0f;

private:

	BlackHole m_BlackHole;
	Sphere m_BlackHoleSphere;
	std::vector<LightParticle> m_LightParticles;

	std::vector<ScreenBounds> m_ParticleScreenBounds;
};
