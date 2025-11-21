#include "Simulator.h"
#include "LightParticle.h"

Simulator::Simulator(BlackHole blackHole, double fixedTimeStep, bool useEOptimization, bool useDeltaTime) :
	m_BlackHole{ std::move(blackHole) },
	m_FixedTimeStep{ fixedTimeStep },
	m_UseEOptimization{ useEOptimization },
	m_UseDeltaTime{ useDeltaTime }
{
}

void Simulator::UpdateParticleRK4(LightParticle& particle, double deltaTime)
{
	if (particle.GetState() == LightState::CAPTURED)
	{
		return;
	}

	double dt = m_UseDeltaTime && deltaTime > 0 ? deltaTime : m_FixedTimeStep;
}