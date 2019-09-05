#pragma once
#include "windStd.h"
#include "VertexCreator.h"

class Direct3dManager
{
private:
	HWND hWnd;
	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*						g_pd3dDevice = NULL;          // Устройство (для создания объектов)
	ID3D11DeviceContext*		g_pImmediateContext = NULL;   // Контекст устройства (рисование)
	IDXGISwapChain*					g_pSwapChain = NULL;          // Цепь связи (буфера с экраном)
	ID3D11RenderTargetView* g_pRenderTargetView = NULL;   // Объект заднего буфера
	ID3D11VertexShader*			g_pVertexShader = NULL;       // Вершинный шейдер
	ID3D11Buffer*						g_pConstantBuffer = NULL;			// Константный буфер
	ID3D11Buffer*						g_pIndexBuffer = NULL;        // Буфер индексов вершин

	DirectX::XMMATRIX				g_World;                      // Матрица мира
	DirectX::XMMATRIX				g_View;                       // Матрица вида
	DirectX::XMMATRIX				g_Projection;                 // Матрица проекции

	ID3D11Texture2D* g_pDepthStencil = NULL;             // Текстура буфера глубин
	ID3D11DepthStencilView* g_pDepthStencilView = NULL;          // Объект вида, буфер глубин

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
	HRESULT CreateVertexShader(LPCSTR shaderName);
	HRESULT CreatePixelShader(LPCSTR shaderName);
	HRESULT SetupVertexBuffer();
	HRESULT CreateIndexBuffer(D3D11_BUFFER_DESC bd, D3D11_SUBRESOURCE_DATA InitData);
	HRESULT CreateConstantBuffer(D3D11_BUFFER_DESC bd);

	HRESULT InitMatrixes();    // Инициализация матриц
public:
	struct SimpleVertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT4 Color;
	};
	struct ConstantBuffer
	{
		DirectX::XMMATRIX mWorld;       // Матрица мира
		DirectX::XMMATRIX mView;        // Матрица вида
		DirectX::XMMATRIX mProjection;  // Матрица проекции
	};

	Direct3dManager(HWND hWnd);
	Direct3dManager(const Direct3dManager&) = delete;
	Direct3dManager& operator=(const Direct3dManager&) = delete;
	~Direct3dManager();

	void Render();
	void Render(void* procedure);

	void SetMatrixes();        // Обновление матрицы мира
};