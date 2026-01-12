#pragma once
#include "FlyFish.h"

struct BlackHoleData
{
	TriVector Position;
	double Mass;
	double SchwarzschildRadius;
};

class BlackHole
{
public:
	BlackHole(TriVector pos, double mass);

	[[nodiscard]] BlackHoleData GetData() const;

private:

	TriVector m_Position;
	double m_Mass;
	double m_SchwarzschildRadius;
};
