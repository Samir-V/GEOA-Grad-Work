#include "LightParticle.h"
#include "BlackHole.h"
#include <iostream>
#include "utils.h"

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

	// COMMUTATOR product of the gep between two lines gives the common normal for these two lines
	// velocity of the particle is in the plane by definition
	// radial axis is created by two points that are in the plane, hence also is in the plane
	// hence the commutator product produces a normal to the plane they lie in
	m_OrbitalNormal = (velocityDir * m_RadialAxis).Grade2().Normalized();

	// Tangential axis must lie in the plane and be perpendicular to the radial axis
	// that means it is also perpendicular to the orbital normal by definition
	// commutator product produces a line perpendicular to them both
	m_TangentialAxis = (m_OrbitalNormal * m_RadialAxis).Grade2().Normalized();

	double alignment = -(velocityDir | m_RadialAxis);
	double tangentProjection = -(velocityDir | m_TangentialAxis);

	if (tangentProjection < 0)
	{
		m_TangentialAxis = m_TangentialAxis * -1.0f;
		tangentProjection = -tangentProjection;
	}

	double tangentMag = tangentProjection;

	if (tangentMag < 1e-10)
	{
		m_L = 0.0;
	}
	else
	{
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
	float angleDegrees = static_cast<float>(m_OrbitAngle * 180.0 / utils::g_Pi);
	Motor rotator = Motor::Rotation(angleDegrees, m_OrbitalNormal);

	BiVector rotatedRadial = (rotator * m_RadialAxis * ~rotator).Grade2().Normalized();

	BiVector translationDir{ rotatedRadial.e23(), rotatedRadial.e31(), rotatedRadial.e12(), 0, 0, 0 };
	Motor translator = Motor::Translation(static_cast<float>(m_DistanceFromBH), translationDir);

	return (translator * m_BlackHolePos * ~translator).Grade3().Normalized();


	/*double cosAngle = std::cos(m_OrbitAngle);
	double sinAngle = std::sin(m_OrbitAngle);

	BiVector direction = m_RadialAxis * cosAngle + m_TangentialAxis * sinAngle;
	BiVector translationDir{ direction.e23(), direction.e31(), direction.e12(), 0, 0, 0 };

	Motor translator = Motor::Translation(static_cast<float>(m_DistanceFromBH), translationDir);

	return (translator * m_BlackHolePos * ~translator).Grade3().Normalized();*/
}