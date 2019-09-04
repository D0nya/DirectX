#include "Window.h"


int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
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