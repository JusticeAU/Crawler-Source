#pragma once

#include "Object.h"
#include <vector>
#include <string>

using std::vector;
using std::to_string;

class Scene
{
public:
	vector<Object*> objects;
	void Update(float deltaTime);
	void DrawObjects();
	void DrawGUI();
	void CleanUp();
};