#pragma once
#include "Graphics.h"
#include <vector>

using std::vector;
class BillboardList
{
public:
	BillboardList();
	~BillboardList();

	void Render();

	vector<glm::vec3> positions;
	GLuint buffer = -1;
};