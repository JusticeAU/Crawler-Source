#include "Primitives.h"

bool CameraFrustum::IsPointInFrustum(glm::vec3 point, CameraFrustum frustum, float forgiveness)
{
	// Check near plane
	float distanceToSurface = glm::dot(point, frustum.nearFace.normal) - frustum.nearFace.distance;
	if (distanceToSurface < -forgiveness) return false;

	// check far plane
	distanceToSurface = glm::dot(point, frustum.farFace.normal) - frustum.farFace.distance;
	if (distanceToSurface < -forgiveness) return false;

	// check left plane
	distanceToSurface = glm::dot(point, frustum.leftFace.normal) - frustum.leftFace.distance;
	if (distanceToSurface < -forgiveness) return false;

	// check right plane
	distanceToSurface = glm::dot(point, frustum.rightFace.normal) - frustum.rightFace.distance;
	if (distanceToSurface < -forgiveness) return false;

	// check top plane
	distanceToSurface = glm::dot(point, frustum.topFace.normal) - frustum.topFace.distance;
	if (distanceToSurface < -forgiveness) return false;

	//// check bottom plane
	//distanceToSurface = glm::dot(point, frustum.bottomFace.normal) - frustum.bottomFace.distance;
	//if (distanceToSurface < -forgiveness) return false;

	return true;
}

CameraFrustum CameraFrustum::GetFrustumFromVPMatrix(glm::mat4 matrix)
{
	CameraFrustum frustum;
	// Near
	frustum.nearFace.normal.x = matrix[0][3] + matrix[0][2];
	frustum.nearFace.normal.y = matrix[1][3] + matrix[1][2];
	frustum.nearFace.normal.z = matrix[2][3] + matrix[2][2];
	frustum.nearFace.distance = -(matrix[3][3] + matrix[3][2]);

	// Far
	frustum.farFace.normal.x = matrix[0][3] - matrix[0][2];
	frustum.farFace.normal.y = matrix[1][3] - matrix[1][2];
	frustum.farFace.normal.z = matrix[2][3] - matrix[2][2];
	frustum.farFace.distance = -(matrix[3][3] - matrix[3][2]);

	// left
	frustum.leftFace.normal.x = matrix[0][3] + matrix[0][0];
	frustum.leftFace.normal.y = matrix[1][3] + matrix[1][0];
	frustum.leftFace.normal.z = matrix[2][3] + matrix[2][0];
	frustum.leftFace.distance = -(matrix[3][3] + matrix[3][0]);

	// right
	frustum.rightFace.normal.x = matrix[0][3] - matrix[0][0];
	frustum.rightFace.normal.y = matrix[1][3] - matrix[1][0];
	frustum.rightFace.normal.z = matrix[2][3] - matrix[2][0];
	frustum.rightFace.distance = -(matrix[3][3] - matrix[3][0]);

	// Top
	frustum.topFace.normal.x = matrix[0][3] - matrix[0][1];
	frustum.topFace.normal.y = matrix[1][3] - matrix[1][1];
	frustum.topFace.normal.z = matrix[2][3] - matrix[2][1];
	frustum.topFace.distance = -(matrix[3][3] - matrix[3][1]);

	// Bottom
	frustum.bottomFace.normal.x = matrix[0][3] + matrix[0][1];
	frustum.bottomFace.normal.y = matrix[1][3] + matrix[1][1];
	frustum.bottomFace.normal.z = matrix[2][3] + matrix[2][1];
	frustum.bottomFace.distance = -(matrix[3][3] + matrix[3][1]);

	return frustum;
}
