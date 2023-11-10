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
	enum Faces
	{
		Top,
		Bottom,
		Right,
		Left,
		Near,
		Far
	};

	Plane faces[6];

	static bool IsPointInPlane(glm::vec3 point, Plane plane);
	static bool IsPointInFrustum(glm::vec3 point, CameraFrustum frustum, float forgiveness = 0.0f);
	static bool IsLineInFrustum(glm::vec3 p0, glm::vec3 p1, CameraFrustum frustum);
	static bool IsLineIntersectingPlane(glm::vec3 p0, glm::vec3 p1, Plane plane);
	static CameraFrustum GetFrustumFromVPMatrix(glm::mat4 matrix);
};