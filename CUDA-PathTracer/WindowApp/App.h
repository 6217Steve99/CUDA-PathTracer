#pragma once

#include "../imgui/imgui_impl_win32.h"
#include "Window.h"
#include "WindowTimer.h"
#include "ImguiManager.h"
#include "Graphics.h"
#include "../Editor/Camera.h"
#include "../tracer/CudaRender.h"
#include "../Editor/PointLight.h"
#include <set>


class App
{
public:
	App();
	// master frame / message loop
	int Go();
	~App();
private:
	void DoFrame();
	void SpawnSimulationWindow() noexcept;
	void SpawnBoxWindowManagerWindow() noexcept;
	void SpawnBoxWindows() noexcept;
private:
	bool enableCudaRenderer=false;
	bool enableEditor = true;
	ImguiManager imgui;
	Window wnd;
	WindowTimer timer;
	CudaRender* render;
	EditorCamera cam;
	PointLight light;
	std::vector<class Box*> boxes;
	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 180;
	float speed_factor = 1.0f;
	std::optional<int> comboBoxIndex;
	std::set<int> boxControlIds;
};