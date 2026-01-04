#include "BlackHole.h"
#include "utils.h"

BlackHole::BlackHole(TriVector pos, double mass) :
	m_Position{std::move(pos)},
	m_Mass{ mass }
{
	m_SchwarzschildRadius = 2.0 * utils::G * m_Mass / utils::C_SQUARED;
}

BlackHoleData BlackHole::GetData() const
{
	return BlackHoleData{
		.Position = m_Position,
		.Mass = m_Mass,
		.SchwarzschildRadius = m_SchwarzschildRadius,
	};
}
