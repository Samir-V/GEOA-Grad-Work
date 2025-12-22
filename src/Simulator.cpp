#include "Simulator.h"
#include "Camera.h"

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
	for (auto& particle : m_LightParticles)
	{
		UpdateParticleRK4(particle, cameraPos, elapsedSec);
	}
}


void Simulator::UpdateParticleRK4(LightParticle& particle, const TriVector& cameraPos, double deltaTime)
{
	if (particle.GetState() == LightState::CAPTURED)
	{
		return;
	}

	// Will be moved from the particle update
	particle.Update(deltaTime, cameraPos);

	double dt = m_UseDeltaTime && deltaTime > 0 ? deltaTime : m_FixedTimeStep;
}

HitResult Simulator::TestRay(const BiVector& ray, const Camera* pCamera) const
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

	for (const auto& particle : m_LightParticles)
	{
		if (particle.GetState() == LightState::CAPTURED) continue;

		if (HitBounds(ray, particle.GetBoundsCenter(), particle.GetBoundsRadius(), camPos))
		{
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
	}

	return result;
}