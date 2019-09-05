#pragma once
#include "windStd.h"
#include "Direct3dManager.h"


class Window
{
private:
	int width;
	int	height;
	HWND hWnd; // Идентификатор окна
	Direct3dManager* manager;	// Класс для работы с DirectX

	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept; 
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wPAram, LPARAM lParam) noexcept;
	
	class WindowClass
	{
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;
		static constexpr const char* wndClassName = "Direct3D Engine Window";
		static WindowClass wndClass;
		HINSTANCE hInst; 
	};
public:
	Window(int width, int height, const char* name) noexcept;
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	~Window();
	
	std::optional<int> ProcessMessages() noexcept;	
};