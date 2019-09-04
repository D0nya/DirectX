#pragma once
#include "windStd.h"
#include <d3d11.h>
#include <d3dx11.h>

class Direct3dManager
{
private:
	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*						g_pd3dDevice = NULL;          // Устройство (для создания объектов)
	ID3D11DeviceContext*		g_pImmediateContext = NULL;   // Контекст устройства (рисование)
	IDXGISwapChain*					g_pSwapChain = NULL;          // Цепь связи (буфера с экраном)
	ID3D11RenderTargetView* g_pRenderTargetView = NULL;   // Объект заднего буфера
	int width;
	int height;


	DXGI_SWAP_CHAIN_DESC CreateFrontBuffer(HWND hWnd);
	HRESULT CreateRearBuffer(HRESULT hr);
	void SetupViewport();

public:
	Direct3dManager(HWND hWnd);
	Direct3dManager(const Direct3dManager&) = delete;
	Direct3dManager& operator=(const Direct3dManager&) = delete;
	~Direct3dManager();

	void Render();
	void Render(void* procedure);
};