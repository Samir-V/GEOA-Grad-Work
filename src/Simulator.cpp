#include "Simulator.h"
#include "Camera.h"
#include <algorithm>
#include <random>

Simulator::Simulator(BlackHole blackHole) :
	m_BlackHole{ std::move(blackHole) }
{
	BlackHoleData data = m_BlackHole.GetData();
	m_BlackHoleSphere.Origin = data.Position.Normalized();
	m_BlackHoleSphere.Radius = static_cast<float>(data.SchwarzschildRadius);
	m_BlackHoleSphere.Color = Color4f{0.0f, 0.0f, 0.0f, 1.0f};
}


void Simulator::Update(float elapsedSec, const TriVector& cameraPos)
{

	for (auto& m_LightParticle : m_LightParticles)
	{
		UpdateParticleRK4(m_LightParticle, cameraPos, elapsedSec);
	}

	// Remove captured particles
	std::erase_if(m_LightParticles, [](const LightParticle& p) { return p.GetState() == LightState::CAPTURED; });
}


void Simulator::UpdateParticleRK4(LightParticle& particle, const TriVector& cameraPos, float deltaTime)
{
	if (particle.GetState() == LightState::CAPTURED)
	{
		return;
	}

	auto particlePos = particle.GetPosition();

	float dx = particlePos.e032() - cameraPos.e032();
	float dy = particlePos.e013() - cameraPos.e013();
	float dz = particlePos.e021() - cameraPos.e021();
	float distSq = dx * dx + dy * dy + dz * dz;

	if (distSq > MaxDistanceSq)
	{
		particle.SetCaptured();
		return;
	}

	particle.UpdateRK4(deltaTime, SimulationSpeed);
}

void Simulator::SpawnLightParticle(const TriVector& position, const BiVector& direction)
{
	m_LightParticles.emplace_back(position, direction, m_BlackHole.GetData());
}

void Simulator::UpdateScreenBounds(const Camera* pCamera, int screenWidth, int screenHeight, float fov, float aspectRatio)
{
	constexpr int PointRadius = 30;

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

void Simulator::SpawnParticleGrid(const Camera* pCamera, int gridWidth, int gridHeight,
	float spacing, float distanceFromCamera)
{
	BiVector forward{ 0, 0, 0, 0, 0, 1 };
	BiVector worldForward = pCamera->CameraToWorldLine(forward);

	BiVector right{ 0, 0, 0, 1, 0, 0 };
	BiVector worldRight = pCamera->CameraToWorldLine(right);

	BiVector up{ 0, 0, 0, 0, 1, 0 };
	BiVector worldUp = pCamera->CameraToWorldLine(up);

	BiVector direction{ 0, 0, 0, worldForward.e23(), worldForward.e31(), worldForward.e12() };

	BiVector forwardDisplacement{ worldForward.e23(), worldForward.e31(), worldForward.e12(), 0, 0, 0 };
	BiVector rightDisplacement{ worldRight.e23(), worldRight.e31(), worldRight.e12(), 0, 0, 0 };
	BiVector upDisplacement{ worldUp.e23(), worldUp.e31(), worldUp.e12(), 0, 0, 0};

	float offsetX = -(gridWidth - 1) * spacing / 2.0f;
	float offsetY = -(gridHeight - 1) * spacing / 2.0f;

	for (int row = 0; row < gridHeight; ++row)
	{
		for (int col = 0; col < gridWidth; ++col)
		{
			float xPos = offsetX + col * spacing;
			float yPos = offsetY + row * spacing;

			TriVector currentPos = pCamera->GetOrigin();

			auto forwardMotor = Motor::Translation(distanceFromCamera, forwardDisplacement);
			currentPos = (forwardMotor * currentPos * ~forwardMotor).Grade3().Normalized();

			auto rightMotor = Motor::Translation(xPos, rightDisplacement);
			currentPos = (rightMotor * currentPos * ~rightMotor).Grade3().Normalized();

			auto upMotor = Motor::Translation(yPos, upDisplacement);
			currentPos = (upMotor * currentPos * ~upMotor).Grade3().Normalized();

			SpawnLightParticle(currentPos, direction);
		}
	}
}