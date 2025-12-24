#pragma once
#include "FlyFish.h"

struct BlackHoleData
{
	TriVector Position;
	double Mass;
	double SchwarzschildRadius;
	double PhotonSphereRadius;
	double CriticalESquared;
};

class BlackHole
{
public:
	BlackHole(TriVector pos, double mass);

	[[nodiscard]] BlackHoleData GetData() const;

	static constexpr double G = 6.67430e-11;
	static constexpr double C = 299792458.0;
	static constexpr double C_SQUARED = C * C;

private:

	TriVector m_Position;
	double m_Mass;
	double m_SchwarzschildRadius;
	double m_PhotonSphereRadius;
	double m_CriticalESquared;
};
