#pragma once
#include <vector>

#include "FlyFish.h"

enum class LightState
{
	MOVING,       
	IN_PHOTON_SPHERE, 
	CAPTURED,     
	ESCAPING
};

class LightParticle
{
public:

	LightParticle(ThreeBlade position, TwoBlade velocity, double e);
	[[nodiscard]] const TwoBlade& GetVelocity() const { return m_Velocity; }
	[[nodiscard]] const ThreeBlade& GetPosition() const { return m_Position; }
	[[nodiscard]] LightState GetState() const { return m_State; }

	void SetCaptured();
	void SetState(LightState state) { m_State = state; }

private:

	ThreeBlade m_Position;
	TwoBlade m_Velocity;
	LightState m_State;
	std::vector<ThreeBlade> m_Path;
	double m_E;
};
