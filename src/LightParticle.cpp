	#include "LightParticle.h"
	#include "BlackHole.h"
	#include <iostream>
	#include "utils.h"

	double LightParticle::m_CriticalImpactParameter = 0.0;
	bool LightParticle::m_CriticalImpactParameterInitialized = false;

	LightParticle::LightParticle(const TriVector& position, BiVector initialDirection, const BlackHoleData& blackHole) :
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

		m_OrbitalNormal = (velocityDir * m_RadialAxis).Grade2().Normalized();
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
	}

	void LightParticle::UpdateRK4(float deltaTime, float simulationSpeed)
	{
		if (m_State == LightState::CAPTURED) 
		{
			return;
		}

		if (m_DistanceFromBH <= m_SchwarzschildRadius)
		{
			SetCaptured();
			return;
		}

		double dt = deltaTime * simulationSpeed;

		ParticleState state = GetCurrentState();

		StateDerivative k1 = ComputeDerivatives(state);

		ParticleState state2;
		state2.radialDistance = state.radialDistance + 0.5 * dt * k1.radialVelocityChange;
		state2.radialVelocity = state.radialVelocity + 0.5 * dt * k1.radialAcceleration;
		state2.orbitalAngle = state.orbitalAngle + 0.5 * dt * k1.angularVelocityChange;
		state2.angularVelocity = state.angularVelocity + 0.5 * dt * k1.angularAcceleration;
		StateDerivative k2 = ComputeDerivatives(state2);

		ParticleState state3;
		state3.radialDistance = state.radialDistance + 0.5 * dt * k2.radialVelocityChange;
		state3.radialVelocity = state.radialVelocity + 0.5 * dt * k2.radialAcceleration;
		state3.orbitalAngle = state.orbitalAngle + 0.5 * dt * k2.angularVelocityChange;
		state3.angularVelocity = state.angularVelocity + 0.5 * dt * k2.angularAcceleration;
		StateDerivative k3 = ComputeDerivatives(state3);

		ParticleState state4;
		state4.radialDistance = state.radialDistance + dt * k3.radialVelocityChange;
		state4.radialVelocity = state.radialVelocity + dt * k3.radialAcceleration;
		state4.orbitalAngle = state.orbitalAngle + dt * k3.angularVelocityChange;
		state4.angularVelocity = state.angularVelocity + dt * k3.angularAcceleration;
		StateDerivative k4 = ComputeDerivatives(state4);

		m_DistanceFromBH += (dt / 6.0) * (k1.radialVelocityChange + 2 * k2.radialVelocityChange + 2 * k3.radialVelocityChange + k4.radialVelocityChange);
		m_RadialSpeed += (dt / 6.0) * (k1.radialAcceleration + 2 * k2.radialAcceleration + 2 * k3.radialAcceleration + k4.radialAcceleration);
		m_OrbitAngle += (dt / 6.0) * (k1.angularVelocityChange + 2 * k2.angularVelocityChange + 2 * k3.angularVelocityChange + k4.angularVelocityChange);

		if (m_DistanceFromBH <= m_SchwarzschildRadius)
		{
			m_State = LightState::CAPTURED;
			return;
		}

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
	}

	StateDerivative LightParticle::ComputeDerivatives(const ParticleState& state) const
	{
		StateDerivative deriv;

		double r = state.radialDistance;
		double r4 = r * r * r * r;

		deriv.radialVelocityChange = state.radialVelocity;

		double dV_dr = m_L * m_L * (3.0 * m_SchwarzschildRadius - 2.0 * r) / r4;
		deriv.radialAcceleration = -0.5 * dV_dr;

		deriv.angularVelocityChange = m_L / (r * r);

		deriv.angularAcceleration = 0.0;

		return deriv;
	}

	ParticleState LightParticle::GetCurrentState() const
	{
		ParticleState state;
		state.radialDistance = m_DistanceFromBH;
		state.radialVelocity = m_RadialSpeed;
		state.orbitalAngle = m_OrbitAngle;
		state.angularVelocity = m_L / (m_DistanceFromBH * m_DistanceFromBH);
		return state;
	}