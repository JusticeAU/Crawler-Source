#pragma once
#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "glad.h"
#include "glm.hpp"
// for matrix transforms
#include "ext\matrix_transform.hpp"
#include "ext\matrix_clip_space.hpp"
#include "ext\quaternion_float.hpp"
#include "ext\quaternion_common.hpp"
#include "gtx/quaternion.hpp"

// for imGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

// for ImGuizmo
#include "ImGuizmo.h"