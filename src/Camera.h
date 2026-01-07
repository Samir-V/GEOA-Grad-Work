#pragma once
#include "FlyFish.h"

class Camera
{
public:
	Camera() = default;

	Camera(TriVector origin, float _fovAngle) :
		m_Origin{std::move(origin)},
		m_FovAngle{ _fovAngle }
	{
		m_Transform = Motor{1, 0, 0, 0, 0, 0, 0, 0};
	}

	const TriVector& GetOrigin() const;
	float GetFOVAngle() const;
	TriVector CameraToWorldPoint(const TriVector& point) const;
	BiVector CameraToWorldLine(const BiVector& line) const;

	bool WorldToScreen(const TriVector& point, int screenWidth, int screenHeight,
		float fov, float aspectRatio, int& outScreenX, int& outScreenY) const;

	// Camera control methods
	void Rotate(float deltaYaw, float deltaPitch);
	void Move(float forward, float right, float up, float elapsedSec);

	float MouseSensitivity{ 0.1f };
	float MoveSpeed{ 5.0f };

private:
	TriVector m_Origin{};
	float m_FovAngle{ 60.f };

	float m_Yaw{ 0.0f };   
	float m_Pitch{ 0.0f };

	Motor m_Transform;


	void UpdateTransform();
};
