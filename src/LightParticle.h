#pragma once
#include <vector>

#include "FlyFish.h"

struct BlackHoleData;

enum class LightState
{
	MOVING,       
	CAPTURED
};

struct ParticleState {
	double radialDistance;    
	double radialVelocity;      
	double orbitalAngle;       
	double angularVelocity;
};

struct StateDerivative {
	double radialVelocityChange;       
	double radialAcceleration;          
	double angularVelocityChange;    
	double angularAcceleration;
};

class LightParticle
{
public:

	LightParticle(const TriVector& position, BiVector initialDirection, const BlackHoleData& blackHole);
	[[nodiscard]] TriVector GetPosition() const;
	[[nodiscard]] LightState GetState() const { return m_State; }
	[[nodiscard]] const std::vector<TriVector>& GetPath() const { return m_Path; }
	[[nodiscard]] double GetESquared() const { return m_E * m_E; }

	void UpdateRK4(float deltaTime, float simulationSpeed);
	void SetCaptured();
	void SetState(LightState state) { m_State = state; }
	StateDerivative ComputeDerivatives(const ParticleState& state) const;
	ParticleState GetCurrentState() const;

private:

	// Geodesic data
	double m_DistanceFromBH;
	double m_OrbitAngle;
	double m_RadialSpeed;

	double m_E; // Energy
	double m_L; // Angular momentum 

	// Predicting fate data 
	static double m_CriticalImpactParameter;
	static bool m_CriticalImpactParameterInitialized;

	// Movement Plane data
	TriVector m_BlackHolePos;
	double m_SchwarzschildRadius;

	BiVector m_RadialAxis; // Direction from black hole to particle
	BiVector m_TangentialAxis; // Perpendicular direction in the orbital plane
	BiVector m_OrbitalNormal; // Normal to the orbital plane

	// Rendering data
	LightState m_State;
	std::vector<TriVector> m_Path;
	float m_TimeSinceLastPath{ 0.0f };
	static constexpr size_t MaxPathSize = 4;
	static constexpr float PathUpdateInterval = 0.3f;
};