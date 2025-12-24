#include "Simulator.h"
#include "Camera.h"
#include <algorithm>

Simulator::Simulator(BlackHole blackHole, double fixedTimeStep, bool useEOptimization, bool useDeltaTime) :
	m_BlackHole{ std::move(blackHole) },
	m_FixedTimeStep{ fixedTimeStep },
	m_UseEOptimization{ useEOptimization },
	m_UseDeltaTime{ useDeltaTime }
{
	BlackHoleData data = m_BlackHole.GetData();
	m_BlackHoleSphere.Origin = data.Position.Normalized();
	m_BlackHoleSphere.Radius = static_cast<float>(data.SchwarzschildRadius);
	m_BlackHoleSphere.Color = Color4f{0.0f, 0.0f, 0.0f, 1.0f};
}


void Simulator::Update(float elapsedSec, const TriVector& cameraPos)
{
	double physicsDt = elapsedSec * TimeScale;

	for (auto& m_LightParticle : m_LightParticles)
	{
		UpdateParticleRK4(m_LightParticle, cameraPos, elapsedSec, physicsDt);
	}

	// Remove captured particles
	std::erase_if(m_LightParticles, [](const LightParticle& p) { return p.GetState() == LightState::CAPTURED; });
}


void Simulator::UpdateParticleRK4(LightParticle& particle, const TriVector& cameraPos, float deltaTime, float physicsDeltaTime)
{
	if (particle.GetState() == LightState::CAPTURED)
	{
		return;
	}
	
	auto line = particle.GetPosition() & m_BlackHoleSphere.Origin;

	if (line.Norm() < m_BlackHoleSphere.Radius)
	{
		particle.SetCaptured();
		return;
	}

	// Add the functionality of the particle being affected by gravity.
	double dt = m_UseDeltaTime && deltaTime > 0 ? deltaTime : m_FixedTimeStep;

	particle.Update(dt, physicsDeltaTime, cameraPos);
}

void Simulator::SpawnLightParticle(const TriVector& position, const BiVector& direction)
{
	// Add the E calculation later on
	m_LightParticles.emplace_back(position, direction, 0.0);
}

void Simulator::UpdateScreenBounds(const Camera* pCamera, int screenWidth, int screenHeight, float fov, float aspectRatio)
{
	constexpr int PointRadius = 8;

	m_ParticleScreenBounds.resize(m_LightParticles.size());
	for (size_t index = 0; index < m_LightParticles.size(); ++index)
	{
		const auto& particle = m_LightParticles[index];
		ScreenBounds& bounds = m_ParticleScreenBounds[index];
		bounds.visible = false;

		if (particle.GetState() == LightState::CAPTURED) continue;

		bounds.minX = screenWidth;
		bounds.maxX = 0;
		bounds.minY = screenHeight;
		bounds.maxY = 0;

		int screenX, screenY;
		if (pCamera->WorldToScreen(particle.GetPosition(), screenWidth, screenHeight, fov, aspectRatio, screenX, screenY))
		{
			bounds.visible = true;
			bounds.minX = std::min(bounds.minX, screenX - PointRadius);
			bounds.maxX = std::max(bounds.maxX, screenX + PointRadius);
			bounds.minY = std::min(bounds.minY, screenY - PointRadius);
			bounds.maxY = std::max(bounds.maxY, screenY + PointRadius);
		}

		for (const auto& pos : particle.GetPath())
		{
			if (pCamera->WorldToScreen(pos, screenWidth, screenHeight, fov, aspectRatio, screenX, screenY))
			{
				bounds.visible = true;
				bounds.minX = std::min(bounds.minX, screenX - PointRadius);
				bounds.maxX = std::max(bounds.maxX, screenX + PointRadius);
				bounds.minY = std::min(bounds.minY, screenY - PointRadius);
				bounds.maxY = std::max(bounds.maxY, screenY + PointRadius);
			}
		}

		bounds.minX = std::max(0, bounds.minX);
		bounds.maxX = std::min(screenWidth - 1, bounds.maxX);
		bounds.minY = std::max(0, bounds.minY);
		bounds.maxY = std::min(screenHeight - 1, bounds.maxY);
	}
}

HitResult Simulator::TestRayAtPixel(const BiVector& ray, const Camera* pCamera, int px, int py) const
{
	HitResult result;
	TriVector camPos = pCamera->GetOrigin().Normalized();

	float sphereDistSq{};
	if (HitSphere(ray, m_BlackHoleSphere, pCamera, sphereDistSq))
	{
		if (sphereDistSq < result.distanceSq)
		{
			result.hit = true;
			result.distanceSq = sphereDistSq;
			result.color = m_BlackHoleSphere.Color;
		}
	}

	for (size_t index = 0; index < m_LightParticles.size(); ++index)
	{
		const auto& bounds = m_ParticleScreenBounds[index];
		{
			if (!bounds.visible) continue;
		}

		if (px < bounds.minX || px > bounds.maxX || py < bounds.minY || py > bounds.maxY)
		{
			continue;
		}

		const auto& particle = m_LightParticles[index];
		const auto& path = particle.GetPath();

		for (const auto& pos : path)
		{
			float pointDistSq{};
			if (HitPoint(ray, pos, 0.05f, camPos, pointDistSq))
			{
				if (pointDistSq < result.distanceSq)
				{
					result.hit = true;
					result.distanceSq = pointDistSq;
					result.color = Color4f{1.0f, 1.0f, 1.0f, 1.0f};
				}
			}
		}

		float posDistSq{};
		if (HitPoint(ray, particle.GetPosition(), 0.10f, camPos, posDistSq))
		{
			if (posDistSq < result.distanceSq)
			{
				result.hit = true;
				result.distanceSq = posDistSq;
				result.color = Color4f{1.0f, 1.0f, 1.0f, 1.0f};
			}
		}
	}

	return result;
}