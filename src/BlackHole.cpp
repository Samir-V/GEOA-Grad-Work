#include "BlackHole.h"

BlackHole::BlackHole(ThreeBlade pos, double mass) :
	m_Position{std::move(pos)},
	m_Mass{ mass }
{
	// Move values to constants. Think about projecting it on the screen
	m_SchwarzschildRadius = 2.0 * 6.67430e-11 * m_Mass / (299792458.0 * 299792458.0);
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
