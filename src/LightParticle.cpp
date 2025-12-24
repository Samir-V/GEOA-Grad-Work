#include "LightParticle.h"

LightParticle::LightParticle(TriVector position, BiVector velocity, double e) :
	m_Position{ std::move(position) },
	m_Velocity{ std::move(velocity) },
	m_State{ LightState::MOVING },
	m_E{ e }
{
	m_Path.push_back(m_Position);
}

void LightParticle::Update(float deltaTime, float physicsDeltaTime, const TriVector& camPos)
{
	if (m_State != LightState::MOVING) return;

	Motor translator = Motor::Translation(C * physicsDeltaTime, m_Velocity);
	m_Position = (translator * m_Position * ~translator).Grade3().Normalize();

	float dx = m_Position.e032() - camPos.e032();
	float dy = m_Position.e013() - camPos.e013();
	float dz = m_Position.e021() - camPos.e021();
	float distSq = dx*dx + dy*dy + dz*dz;

	if (distSq > MaxDistanceSq)
	{
		SetCaptured();
		return;
	}

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