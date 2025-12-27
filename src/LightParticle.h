#pragma once
#include <vector>

#include "FlyFish.h"

struct BlackHoleData;

enum class LightState
{
	MOVING,       
	CAPTURED,     
	ESCAPING
};

class LightParticle
{
public:

	LightParticle(TriVector position, BiVector initialDirection, const BlackHoleData& blackHole);
	[[nodiscard]] TriVector GetPosition() const;
	[[nodiscard]] LightState GetState() const { return m_State; }
	[[nodiscard]] const std::vector<TriVector>& GetPath() const { return m_Path; }
	[[nodiscard]] double GetESquared() const { return m_E * m_E; }

	void Update(float deltaTime, float simulationSpeed);
	void SetCaptured();
	void SetState(LightState state) { m_State = state; }

private:

	// Geodesic data
	double m_DistanceFromBH;
	double m_OrbitAngle;
	double m_RadialSpeed;

	double m_E; // Energy
	double m_L; // Angular momentum 

	// Predicting fate data (shared across all particles)
	static double m_CriticalImpactParameter;
	static bool m_CriticalImpactParameterInitialized;

	// Movement Plane data
	TriVector m_BlackHolePos;
	double m_SchwarzschildRadius;

	BiVector m_RadialAxis; // The direction FROM the black hole to the particle
	BiVector m_TangentialAxis; // The perpendicular direction

	// Rendering data
	LightState m_State;
	std::vector<TriVector> m_Path;
	float m_TimeSinceLastPath{ 0.0f };
	static constexpr size_t MaxPathSize = 4;
	static constexpr float PathUpdateInterval = 0.3f;
};
