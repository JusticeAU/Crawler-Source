#include "Primitives.h"

bool CameraFrustum::IsPointInPlane(glm::vec3 point, Plane plane)
{
	float distanceToSurface = glm::dot(point, plane.normal) - plane.distance;
	return (distanceToSurface > 0);
}

bool CameraFrustum::IsPointInFrustum(glm::vec3 point, CameraFrustum frustum, float forgiveness)
{
	// Check near plane
	float distanceToSurface = glm::dot(point, frustum.faces[Near].normal) - frustum.faces[Near].distance;
	if (distanceToSurface < -forgiveness) return false;

	// check far plane
	distanceToSurface = glm::dot(point, frustum.faces[Far].normal) - frustum.faces[Far].distance;
	if (distanceToSurface < -forgiveness) return false;

	// check left plane
	distanceToSurface = glm::dot(point, frustum.faces[Left].normal) - frustum.faces[Left].distance;
	if (distanceToSurface < -forgiveness) return false;

	// check right plane
	distanceToSurface = glm::dot(point, frustum.faces[Right].normal) - frustum.faces[Right].distance;
	if (distanceToSurface < -forgiveness) return false;

	// check top plane
	distanceToSurface = glm::dot(point, frustum.faces[Top].normal) - frustum.faces[Top].distance;
	if (distanceToSurface < -forgiveness) return false;

	//// check bottom plane
	//distanceToSurface = glm::dot(point, frustum.faces[Bottom].normal) - frustum.faces[Bottom].distance;
	//if (distanceToSurface < -forgiveness) return false;

	return true;
}

bool CameraFrustum::IsLineInFrustum(glm::vec3 p0, glm::vec3 p1, CameraFrustum frustum)
{
	for(int i = 0; i < 6; i ++) if (IsLineIntersectingPlane(p0, p1, frustum.faces[i])) return true;
	return false;
}

bool CameraFrustum::IsLineIntersectingPlane(glm::vec3 p0, glm::vec3 p1, Plane plane)
{
	// returns the point where the line p0-p1 intersects the plane n&d
	glm::vec3 dif;
	dif = p1 - p0;
	float dn = dot(plane.normal, dif);
	float t = (plane.distance + dot(plane.normal, p0)) / dn;
	if (t < 0 || t > 1) return false;
	else return true;

}

CameraFrustum CameraFrustum::GetFrustumFromVPMatrix(glm::mat4 matrix)
{
	CameraFrustum frustum;
	// Near
	frustum.faces[Near].normal.x = matrix[0][3] + matrix[0][2];
	frustum.faces[Near].normal.y = matrix[1][3] + matrix[1][2];
	frustum.faces[Near].normal.z = matrix[2][3] + matrix[2][2];
	frustum.faces[Near].distance = -(matrix[3][3] + matrix[3][2]);

	// Far
	frustum.faces[Far].normal.x = matrix[0][3] - matrix[0][2];
	frustum.faces[Far].normal.y = matrix[1][3] - matrix[1][2];
	frustum.faces[Far].normal.z = matrix[2][3] - matrix[2][2];
	frustum.faces[Far].distance = -(matrix[3][3] - matrix[3][2]);

	// left
	frustum.faces[Left].normal.x = matrix[0][3] + matrix[0][0];
	frustum.faces[Left].normal.y = matrix[1][3] + matrix[1][0];
	frustum.faces[Left].normal.z = matrix[2][3] + matrix[2][0];
	frustum.faces[Left].distance = -(matrix[3][3] + matrix[3][0]);

	// right
	frustum.faces[Right].normal.x = matrix[0][3] - matrix[0][0];
	frustum.faces[Right].normal.y = matrix[1][3] - matrix[1][0];
	frustum.faces[Right].normal.z = matrix[2][3] - matrix[2][0];
	frustum.faces[Right].distance = -(matrix[3][3] - matrix[3][0]);

	// Top
	frustum.faces[Top].normal.x = matrix[0][3] - matrix[0][1];
	frustum.faces[Top].normal.y = matrix[1][3] - matrix[1][1];
	frustum.faces[Top].normal.z = matrix[2][3] - matrix[2][1];
	frustum.faces[Top].distance = -(matrix[3][3] - matrix[3][1]);

	// Bottom
	frustum.faces[Bottom].normal.x = matrix[0][3] + matrix[0][1];
	frustum.faces[Bottom].normal.y = matrix[1][3] + matrix[1][1];
	frustum.faces[Bottom].normal.z = matrix[2][3] + matrix[2][1];
	frustum.faces[Bottom].distance = -(matrix[3][3] + matrix[3][1]);

	return frustum;
}
