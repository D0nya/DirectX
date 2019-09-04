#include "Window.h"

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	try
	{
		Window* window1 = new Window(800, 600, "Window");
		while (true)
		{
			if (const auto ecode = window1->ProcessMessages())
			{
				return *ecode;
			}
		}
	}
	catch (std::exception e)
	{
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
	}
	return 0;
} 