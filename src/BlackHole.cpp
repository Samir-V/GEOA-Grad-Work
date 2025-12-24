#include "BlackHole.h"

BlackHole::BlackHole(TriVector pos, double mass) :
	m_Position{std::move(pos)},
	m_Mass{ mass }
{
	m_SchwarzschildRadius = 2.0 * G * m_Mass / C_SQUARED;
	m_PhotonSphereRadius = 1.5 * m_SchwarzschildRadius;
	m_CriticalESquared = 4.0 / 27.0;
}

BlackHoleData BlackHole::GetData() const
{
	return BlackHoleData{
		.Position = m_Position,
		.Mass = m_Mass,
		.SchwarzschildRadius = m_SchwarzschildRadius,
		.PhotonSphereRadius = m_PhotonSphereRadius,
		.CriticalESquared = m_CriticalESquared
	};
}
