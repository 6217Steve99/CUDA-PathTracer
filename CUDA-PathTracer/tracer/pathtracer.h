#pragma once
#include "common.h"

class Camera;
class Scene;
class BVH;

void Render(Scene& scene, unsigned width, unsigned height, Camera* camera, unsigned iter, bool reset, float3* output);
void BeginRender(Scene& scene, unsigned width, unsigned height, float ep);
void EndRender();