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

	LightParticle(TriVector position, BiVector velocity, double e);
	[[nodiscard]] const BiVector& GetVelocity() const { return m_Velocity; }
	[[nodiscard]] const TriVector& GetPosition() const { return m_Position; }
	[[nodiscard]] LightState GetState() const { return m_State; }
	[[nodiscard]] const std::vector<TriVector>& GetPath() const { return m_Path; }
	[[nodiscard]] const TriVector& GetBoundsCenter() const { return m_BoundsCenter; }
	[[nodiscard]] float GetBoundsRadius() const { return m_BoundsRadius; }

	void Update(float deltaTime, const TriVector& camPos);
	void SetCaptured();
	void SetState(LightState state) { m_State = state; }

private:

	static constexpr size_t MaxPathSize = 4;
	static constexpr float PathUpdateInterval = 0.3f;
	static constexpr float MaxDistanceSq = 100.0f * 100.0f;

	TriVector m_Position;
	BiVector m_Velocity;
	LightState m_State;
	std::vector<TriVector> m_Path;
	double m_E;
	float m_TimeSinceLastPath{ 0.0f };

	TriVector m_BoundsCenter;
	float m_BoundsRadius{ 0.0f };

	void UpdateBounds();
};
