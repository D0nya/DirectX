#pragma once
#include "windStd.h"

class Direct3dManager
{
private:
	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*						g_pd3dDevice = NULL;          // Устройство (для создания объектов)
	ID3D11DeviceContext*		g_pImmediateContext = NULL;   // Контекст устройства (рисование)
	IDXGISwapChain*					g_pSwapChain = NULL;          // Цепь связи (буфера с экраном)
	ID3D11RenderTargetView* g_pRenderTargetView = NULL;   // Объект заднего буфера
	ID3D11VertexShader* g_pVertexShader = NULL;             // Вершинный шейдер

	ID3D11PixelShader* g_pPixelShader = NULL;     // Пиксельный шейдер
	ID3D11InputLayout* g_pVertexLayout = NULL;    // Описание формата вершин
	ID3D11Buffer* g_pVertexBuffer = NULL;         // Буфер вершин
	int width;
	int height;

	DXGI_SWAP_CHAIN_DESC CreateFrontBuffer(HWND hWnd);
	HRESULT CreateRearBuffer(HRESULT hr);
	void SetupViewport();
	HRESULT InitGeometry();    // Инициализация шаблона ввода и буфера вершин
	HRESULT CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
public:
	struct SimpleVertex
	{
		DirectX::XMFLOAT3 Pos;
	};

	Direct3dManager(HWND hWnd);
	Direct3dManager(const Direct3dManager&) = delete;
	Direct3dManager& operator=(const Direct3dManager&) = delete;
	~Direct3dManager();

	void Render();
	void Render(void* procedure);
};