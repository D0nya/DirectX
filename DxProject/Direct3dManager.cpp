#include "Direct3dManager.h"

Direct3dManager::Direct3dManager(HWND hWnd)
{
	HRESULT hr = S_OK;
	RECT rc;
	GetClientRect(hWnd, &rc);
	width = rc.right - rc.left;			// �������� ������
	height = rc.bottom - rc.top;		// � ������ ����

	UINT createDeviceFlags = 0;
	D3D_DRIVER_TYPE driverTypes[] =
	{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	// ��� �� ������� ������ �������������� ������ DirectX
	D3D_FEATURE_LEVEL featureLevels[] =
	{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	auto sd = CreateFrontBuffer(hWnd);

	// �������� ������������� ����� ��������� � �������� ����������
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain
		(
			NULL,
			g_driverType,
			NULL,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&sd, &g_pSwapChain,
			&g_pd3dDevice,
			&g_featureLevel,
			&g_pImmediateContext
		);
		if (SUCCEEDED(hr)) // ���� ���������� ������� �������, �� ������� �� �����
			break;
	}
	if (FAILED(hr))
		return;

	// �������� �������� � ������ ������
	if (FAILED(InitGeometry()))
		return;

	CreateRearBuffer(hr);
	SetupViewport();
}

Direct3dManager::~Direct3dManager()
{
	// ������� �������� �������� ����������
	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	// ����� ������ �������
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}

DXGI_SWAP_CHAIN_DESC Direct3dManager::CreateFrontBuffer(HWND hWnd)
{
	// ������ �� �������� ���������� DirectX. ��� ������ �������� ���������,
	// ������� ��������� �������� ��������� ������ � ����������� ��� � ������ ����.
	DXGI_SWAP_CHAIN_DESC sd;									// ���������, ����������� ���� ����� (Swap Chain)
	ZeroMemory(&sd, sizeof(sd));							// ������� ��
	sd.BufferCount = 1;																	// � ��� ���� ������ �����
	sd.BufferDesc.Width = width;												// ������ ������
	sd.BufferDesc.Height = height;											// ������ ������
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// ������ ������� � ������
	sd.BufferDesc.RefreshRate.Numerator = 75;						// ������� ���������� ������
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// ���������� ������ - ������ �����
	sd.OutputWindow = hWnd;		// ����������� � ������ ����
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;	// �� ������������� �����

	return sd;
}

HRESULT Direct3dManager::CreateRearBuffer(HRESULT hr)		
{
	// ������ ������� ������ �����. �������� ��������, � SDK
	// RenderTargetOutput - ��� �������� �����, � RenderTargetView - ������.
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
	if (FAILED(hr)) return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr)) return hr;

	// ���������� ������ ������� ������ � ��������� 
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
	return hr;
}

HRESULT Direct3dManager::InitGeometry()
{
	HRESULT hr = S_OK;
	// ���������� ���������� ������� �� �����
	ID3DBlob* pVSBlob = NULL; // ��������������� ������ - ������ ����� � ����������� ������
	hr = CompileShaderFromFile("shader.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", "������", MB_OK);
		return hr;
	}

	// �������� ���������� �������
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// ����������� ������� ������
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			/* ������������� ���, ������������� ������, ������, �������� ���� (0-15), ����� ������ ������ � ������ ������, ����� ��������� ����� (�� �����), InstanceDataStepRate (�� �����) */
	};
	UINT numElements = ARRAYSIZE(layout);

	// �������� ������� ������
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr)) return hr;
	// ����������� ������� ������
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// ���������� ����������� ������� �� �����
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile("shader.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "���������� �������������� ���� FX. ����������, ��������� ������ ��������� �� �����, ���������� ���� FX.", "������", MB_OK);
		return hr;
	}
	// �������� ����������� �������
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr)) return hr;

	// �������� ������ ������ (��� ������� ������������)
	SimpleVertex vertices[3];
	vertices[0].Pos.x = 0.0f;  vertices[0].Pos.y = 0.5f;  vertices[0].Pos.z = 0.5f;
	vertices[1].Pos.x = 0.5f;  vertices[1].Pos.y = -0.5f;  vertices[1].Pos.z = 0.5f;
	vertices[2].Pos.x = -0.5f;  vertices[2].Pos.y = -0.5f;  vertices[2].Pos.z = 0.5f;
	
	D3D11_BUFFER_DESC bd;  // ���������, ����������� ����������� �����
	ZeroMemory(&bd, sizeof(bd));                    // ������� ��
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 3; // ������ ������ = ������ ����� ������� * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;          // ��� ������ - ����� 
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData; // ���������, ���������� ������ ������
	ZeroMemory(&InitData, sizeof(InitData)); // ������� 
	InitData.pSysMem = vertices;               // ��������� �� ���� 3 �������

	// ����� ������ g_pd3dDevice ������� ������ ������ ������ ID3D11Buffer
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr)) return hr;

	// ��������� ������ ������:
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// ��������� ������� ��������� ������ � ������
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return S_OK;
}

void Direct3dManager::SetupViewport()
{
	// ��������� ��������
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	// ���������� ������� � ��������� ����������
	g_pImmediateContext->RSSetViewports(1, &vp);
}

HRESULT Direct3dManager::CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) 
		pErrorBlob->Release();
	
	return S_OK;
}

void Direct3dManager::Render()
{
	// �������� ������ �����
	float ClearColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f }; // �������, �������, �����, �����-�����
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	// ���������� � ���������� ��������� �������
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	// ���������� ��� �������
	g_pImmediateContext->Draw(3, 0);
	// ������� � �������� ����� (�� �����) ����������, ������������ � ������ ������.
	g_pSwapChain->Present(0, 0);
}
void Direct3dManager::Render(void* procedure)
{
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // �������, �������, �����, �����-�����

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	// ��������� ������ ����� �� �����
	g_pSwapChain->Present(0, 0);
}
