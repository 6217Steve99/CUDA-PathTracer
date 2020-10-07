#pragma once

#include "../imgui/imgui_impl_win32.h"
#include "Window.h"
#include "WindowTimer.h"
#include "ImguiManager.h"
#include "Graphics.h"
#include "../tracer/CudaRender.h"


class App
{
public:
	App();
	// master frame / message loop
	int Go();
	~App();
private:
	void DoFrame();
private:
	ImguiManager imgui;
	Window wnd;
	WindowTimer timer;
	float speed_factor = 1.0f;
	CudaRender* render;

	static constexpr size_t nDrawables = 180;
};