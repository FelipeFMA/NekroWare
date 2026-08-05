#pragma once
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dxgi.lib")
#include <dwmapi.h>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgitype.h>
#include <winerror.h>
#include <tchar.h>
#include <algorithm>
#include <string>
#include <thread>
#include <iostream>
#include <filesystem>
#include <map>
#include <queue>
#include <chrono>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/KeyBind.h"
#include "utils/Header.h"
#include "utils/utils.h"
#include "../rbx/globals/globals.h"
#include "../rbx/globals/options.h"
#include "../features/esp.h"
#include "../features/aimbot.h"
#include "../features/triggerbot.h"
#include "../features/macro.h"
#include "../features/crosshair.h"
#include "utils/W2S.h"

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern bool g_SwapChainOccluded;
extern UINT g_ResizeWidth, g_ResizeHeight;
extern ID3D11RenderTargetView* g_mainRenderTargetView;
extern ImVec4 main_color;
extern ImFont* InterMedium;
extern ImFont* IconFont;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool IsGameOnTop(const std::string& expectedTitle);
void SetTransparency(HWND hwnd, bool boolean);
void DrawNode(RobloxInstance& node);
void ShowImgui();

void RenderKeybindList(ImDrawList* drawList);
void RenderNotifications(ImDrawList* drawList);
void RenderRadar(ImDrawList* drawList);
void RenderFPSCounter(ImDrawList* drawList);
void RenderPerformanceMetrics(ImDrawList* drawList);

// Theme management
void ApplyTheme(int themeId);
void SetDarkTheme();
void SetLightTheme();
void SetPinkTheme();
void SetPurpleTheme();
void SetBlueTheme();
void SetGreenTheme();

// Animation utilities
float EaseInOutQuad(float t);
float EaseInQuad(float t);
float EaseOutQuad(float t);
ImU32 InterpolateColor(ImU32 colA, ImU32 colB, float t);
float GetPulseValue(float speed, float intensity);

// Gradient and effect drawing
void DrawGradientRect(ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_btm_right, ImU32 col_btm_left);
void DrawHorizontalGradient(ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_left, ImU32 col_right);
void DrawVerticalGradient(ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_top, ImU32 col_bottom);
void DrawPulsingGradientRect(ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_primary, ImU32 col_secondary);

// Smooth menu transitions
void StartTabTransition();
float GetTabTransitionAlpha();

// Custom tab/subtab renderers from menu
bool renderer_tab(const char* label, const char* icon, bool selected);
bool renderer_subtab(const char* label, const ImVec2& size_arg, const bool selected);
