#include "LightParticle.h"
#include "BlackHole.h"
#include <iostream>

double LightParticle::m_CriticalImpactParameter = 0.0;
bool LightParticle::m_CriticalImpactParameterInitialized = false;

LightParticle::LightParticle(TriVector position, BiVector initialDirection, const BlackHoleData& blackHole) :
	m_BlackHolePos(blackHole.Position),
	m_SchwarzschildRadius(blackHole.SchwarzschildRadius),
	m_State(LightState::MOVING)
{
	if (!m_CriticalImpactParameterInitialized)
	{
		m_CriticalImpactParameter = 2.598 * blackHole.SchwarzschildRadius;
		m_CriticalImpactParameterInitialized = true;
		std::cout << "Critical impact parameter: " << m_CriticalImpactParameter << std::endl;
	}

	BiVector toParticle = blackHole.Position & position;
	m_DistanceFromBH = toParticle.Norm();
	m_RadialAxis = toParticle.Normalized();

	BiVector velocityDir = BiVector{ 0, 0, 0, initialDirection.e23(), initialDirection.e31(), initialDirection.e12() }.Normalized();

	double alignment = -(velocityDir | m_RadialAxis);
	BiVector tangent = velocityDir - m_RadialAxis * alignment;
	double tangentMag = tangent.Norm();

	if (tangentMag < 1e-10)
	{
		BiVector arbitrary = (std::abs(m_RadialAxis.e23()) < 0.9f)
			? BiVector{ 0, 0, 0, 1, 0, 0 }
		: BiVector{ 0, 0, 0, 0, 1, 0 };

		tangent = arbitrary - m_RadialAxis * (arbitrary | m_RadialAxis);
		m_TangentialAxis = tangent.Normalized();
		m_L = 0.0;
	}
	else
	{
		m_TangentialAxis = tangent.Normalized();
		m_L = m_DistanceFromBH * tangentMag;
	}

	m_OrbitAngle = 0.0;
	m_RadialSpeed = alignment;

	double metric = 1.0 - m_SchwarzschildRadius / m_DistanceFromBH;
	double V_eff = metric * (m_L * m_L) / (m_DistanceFromBH * m_DistanceFromBH);
	m_E = std::sqrt(m_RadialSpeed * m_RadialSpeed + V_eff);

	m_Path.push_back(position);

	// DEBUG OUTPUTS

	double impactParameter = m_L / m_E;

	if (impactParameter < m_CriticalImpactParameter)
	{
		std::cout << "Particle WILL be captured. Impact parameter: " << impactParameter << std::endl;
	}
	else
	{
		std::cout << "Particle will ESCAPE. Impact parameter: " << impactParameter << std::endl;
	}

	/*std::cout << "Distance from BH: " << m_DistanceFromBH << std::endl;
	std::cout << "Alignment: " << alignment << std::endl;
	std::cout << "Tangent magnitude: " << tangentMag << std::endl;
	std::cout << "Angular momentum L: " << m_L << std::endl;
	std::cout << "Radial speed: " << m_RadialSpeed << std::endl;
	std::cout << "RadialAxis: (" << m_RadialAxis.e23() << ", " << m_RadialAxis.e31() << ", " << m_RadialAxis.e12() << ")" << std::endl;
	std::cout << "VelocityDir: (" << velocityDir.e23() << ", " << velocityDir.e31() << ", " << velocityDir.e12() << ")" << std::endl;*/
}

void LightParticle::Update(float deltaTime, float simulationSpeed)
{
	if (m_State == LightState::CAPTURED || m_State == LightState::ESCAPING) 
	{
		return;
	}

	if (m_DistanceFromBH <= m_SchwarzschildRadius)
	{
		SetCaptured();
		return;
	}

	double r = m_DistanceFromBH;
	double r4 = r * r * r * r;

	double dV_dr = (m_L * m_L) * (3.0 * m_SchwarzschildRadius - 2.0 * r) / r4;
	double radialAcceleration = -0.5 * dV_dr;

	auto actualStep = deltaTime * simulationSpeed;

	m_RadialSpeed += radialAcceleration * actualStep;
	m_DistanceFromBH += m_RadialSpeed * actualStep;

	if (m_DistanceFromBH <= m_SchwarzschildRadius)
	{
		m_State = LightState::CAPTURED;
		return;
	}

	double angularSpeed = m_L / (m_DistanceFromBH * m_DistanceFromBH);
	m_OrbitAngle += angularSpeed * actualStep;

	m_TimeSinceLastPath += deltaTime;
	if (m_TimeSinceLastPath >= PathUpdateInterval)
	{
		m_TimeSinceLastPath = 0.0f;

		m_Path.push_back(GetPosition());

		if (m_Path.size() > MaxPathSize)
		{
			m_Path.erase(m_Path.begin());
		}
	}
}

void LightParticle::SetCaptured()
{
	m_State = LightState::CAPTURED;
	m_Path.clear();
}

TriVector LightParticle::GetPosition() const
{
	double cosAngle = std::cos(m_OrbitAngle);
	double sinAngle = std::sin(m_OrbitAngle);

	BiVector direction = m_RadialAxis * cosAngle + m_TangentialAxis * sinAngle;

	BiVector translationDir{ direction.e23(), direction.e31(), direction.e12(), 0, 0, 0 };

	Motor translator = Motor::Translation(static_cast<float>(m_DistanceFromBH), translationDir);
	return (translator * m_BlackHolePos * ~translator).Grade3().Normalized();
}