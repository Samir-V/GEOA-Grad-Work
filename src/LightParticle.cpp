#include "LightParticle.h"
#include <cmath>
#include <algorithm>

LightParticle::LightParticle(TriVector position, BiVector velocity, double e) :
	m_Position{ std::move(position) },
	m_Velocity{ std::move(velocity) },
	m_State{ LightState::MOVING },
	m_E{ e },
	m_BoundsCenter{ m_Position }
{
	m_Path.push_back(m_Position);
	UpdateBounds();
}

void LightParticle::Update(float deltaTime, const TriVector& camPos)
{
	if (m_State != LightState::MOVING) return;

	Motor translator = Motor::Translation(10 * deltaTime, m_Velocity);
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

		UpdateBounds();
	}
}

void LightParticle::UpdateBounds()
{
	if (m_Path.empty())
	{
		m_BoundsCenter = m_Position;
		m_BoundsRadius = 0.25f;
		return;
	}

	float cx = 0, cy = 0, cz = 0;
	for (const auto& p : m_Path)
	{
		cx += p.e032();
		cy += p.e013();
		cz += p.e021();
	}
	cx += m_Position.e032();
	cy += m_Position.e013();
	cz += m_Position.e021();

	float count = static_cast<float>(m_Path.size() + 1);
	cx /= count;
	cy /= count;
	cz /= count;

	m_BoundsCenter = TriVector{cx, cy, cz, 1.0f};

	float maxDistSq = 0.0f;
	for (const auto& p : m_Path)
	{
		float dx = p.e032() - cx;
		float dy = p.e013() - cy;
		float dz = p.e021() - cz;
		float distSq = dx*dx + dy*dy + dz*dz;
		maxDistSq = std::max(maxDistSq, distSq);
	}
	float dx = m_Position.e032() - cx;
	float dy = m_Position.e013() - cy;
	float dz = m_Position.e021() - cz;
	float distSq = dx*dx + dy*dy + dz*dz;
	maxDistSq = std::max(maxDistSq, distSq);

	m_BoundsRadius = std::sqrt(maxDistSq) + 0.25f;
}

void LightParticle::SetCaptured()
{
	m_State = LightState::CAPTURED;
	m_Path.clear();
}