#pragma once
#include "BlackHole.h"

class LightParticle;

class Simulator
{
public:
	Simulator(BlackHole blackHole, double fixedTimeStep, bool useEOptimization, bool useDeltaTime);

	void UpdateParticleRK4(LightParticle& particle, double deltaTime);

private:

	BlackHole m_BlackHole;
	double m_FixedTimeStep;
	bool m_UseEOptimization;
	bool m_UseDeltaTime;
};
