#include "LightParticle.h"

LightParticle::LightParticle(ThreeBlade position, TwoBlade velocity, double e) :
	m_Position{ std::move(position) },
	m_Velocity{ std::move(velocity) },
	m_State{ LightState::MOVING },
	m_E{ e }
{
}

void LightParticle::SetCaptured()
{
	m_State = LightState::CAPTURED;
	m_Path.clear();
}