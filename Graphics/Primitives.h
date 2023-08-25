#pragma once
#include "glm.hpp"
struct Plane
{
	glm::vec3 normal	= { 0.0f, 1.0f, 0.01f };
	float distance		= 0.0f;
};

class CameraFrustum
{
public:
	Plane topFace;
	Plane bottomFace;

	Plane rightFace;
	Plane leftFace;

	Plane farFace;
	Plane nearFace;

	static bool IsPointInFrustum(glm::vec3 point, CameraFrustum frustum, float forgiveness = 0.0f);
	static CameraFrustum GetFrustumFromVPMatrix(glm::mat4 matrix);
};