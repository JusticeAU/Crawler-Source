#pragma once
#include "glm.hpp"
struct Plane
{
	glm::vec3 normal	= { 0.0f, 1.0f, 0.01f };
	float distance		= 0.0f;
};

struct CameraFrustum
{
	Plane topFace;
	Plane bottomFace;

	Plane rightFace;
	Plane leftFace;

	Plane farFace;
	Plane nearFace;
};