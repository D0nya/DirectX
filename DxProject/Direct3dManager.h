#pragma once
#include "windStd.h"

class Direct3dManager
{
private:
	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*						g_pd3dDevice = NULL;          // ���������� (��� �������� ��������)
	ID3D11DeviceContext*		g_pImmediateContext = NULL;   // �������� ���������� (���������)
	IDXGISwapChain*					g_pSwapChain = NULL;          // ���� ����� (������ � �������)
	ID3D11RenderTargetView* g_pRenderTargetView = NULL;   // ������ ������� ������
	ID3D11VertexShader* g_pVertexShader = NULL;             // ��������� ������

	ID3D11PixelShader* g_pPixelShader = NULL;     // ���������� ������
	ID3D11InputLayout* g_pVertexLayout = NULL;    // �������� ������� ������
	ID3D11Buffer* g_pVertexBuffer = NULL;         // ����� ������
	int width;
	int height;

	DXGI_SWAP_CHAIN_DESC CreateFrontBuffer(HWND hWnd);
	HRESULT CreateRearBuffer(HRESULT hr);
	void SetupViewport();
	HRESULT InitGeometry();    // ������������� ������� ����� � ������ ������
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