#pragma once
#include "FlyFish.h"

struct BlackHoleData
{
	ThreeBlade Position;
	double Mass;
	double SchwarzschildRadius;
	double PhotonSphereRadius;
	double CriticalESquared;
};

class BlackHole
{
public:
	BlackHole(ThreeBlade pos, double mass);

	[[nodiscard]] BlackHoleData GetData() const;

private:

	ThreeBlade m_Position;
	double m_Mass;
	double m_SchwarzschildRadius;
	double m_PhotonSphereRadius;
	double m_CriticalESquared;
};
