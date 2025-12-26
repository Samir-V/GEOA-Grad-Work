#include "LightParticle.h"

#include "utils.h"

LightParticle::LightParticle(TriVector position, BiVector velocity, double e) :
	m_Position{ std::move(position) },
	m_Velocity{ std::move(velocity) },
	m_State{ LightState::MOVING },
	m_ESquared{ e }
{
	m_Path.push_back(m_Position);
}

void LightParticle::Update(float deltaTime, float physicsDeltaTime, const BiVector& bendingAccel)
{
	if (m_State != LightState::MOVING) return;

	BiVector newVelocity = m_Velocity + bendingAccel * physicsDeltaTime;
	m_Velocity = newVelocity / newVelocity.VNorm();

	Motor translator = Motor::Translation(utils::C * physicsDeltaTime, m_Velocity);
	m_Position = (translator * m_Position * ~translator).Grade3().Normalize();

	m_TimeSinceLastPath += deltaTime;
	if (m_TimeSinceLastPath >= PathUpdateInterval)
	{
		m_TimeSinceLastPath = 0.0f;

		m_Path.push_back(m_Position);

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