#include "Direct3dManager.h"

Direct3dManager::Direct3dManager(HWND _hWnd) : hWnd(_hWnd)
{
	HRESULT hr = S_OK;
	RECT rc;
	GetClientRect(hWnd, &rc);
	width = rc.right - rc.left;			// получаем ширину
	height = rc.bottom - rc.top;		// и высоту окна

	UINT createDeviceFlags = 0;
	D3D_DRIVER_TYPE driverTypes[] =
	{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	// Тут мы создаем список поддерживаемых версий DirectX
	D3D_FEATURE_LEVEL featureLevels[] =
	{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	auto sd = CreateFrontBuffer(hWnd);

	// проверка совместимости типов драйверов и создание устройства
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
		if (SUCCEEDED(hr)) // Если устройства созданы успешно, то выходим из цикла
			break;
	}
	if (FAILED(hr))
		return;

	// Создание шейдеров и буфера вершин
	if (FAILED(InitGeometry()))
		return;

	// Инициализация матриц
	if (FAILED(InitMatrixes()))
		return;

	CreateRearBuffer(hr);
	SetupViewport();
}

Direct3dManager::~Direct3dManager()
{
	// Сначала отключим контекст устройства
	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	// Потом удалим объекты
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}

DXGI_SWAP_CHAIN_DESC Direct3dManager::CreateFrontBuffer(HWND hWnd)
{
	// Сейчас мы создадим устройства DirectX. Для начала заполним структуру,
	// которая описывает свойства переднего буфера и привязывает его к нашему окну.
	DXGI_SWAP_CHAIN_DESC sd;									// Структура, описывающая цепь связи (Swap Chain)
	ZeroMemory(&sd, sizeof(sd));							// очищаем ее
	sd.BufferCount = 1;																	// у нас один задний буфер
	sd.BufferDesc.Width = width;												// ширина буфера
	sd.BufferDesc.Height = height;											// высота буфера
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// формат пикселя в буфере
	sd.BufferDesc.RefreshRate.Numerator = 75;						// частота обновления экрана
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// назначение буфера - задний буфер
	sd.OutputWindow = hWnd;		// привязываем к нашему окну
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;	// не полноэкранный режим

	return sd;
}

HRESULT Direct3dManager::CreateRearBuffer(HRESULT hr)		
{
	// Теперь создаем задний буфер. Обратите внимание, в SDK
	// RenderTargetOutput - это передний буфер, а RenderTargetView - задний.
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
	if (FAILED(hr)) return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr)) return hr;

	// Подключаем объект заднего буфера к контексту 
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

	// Переходим к созданию буфера глубин
// Создаем текстуру-описание буфера глубин
	D3D11_TEXTURE2D_DESC descDepth;     // Структура с параметрами
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;            // ширина и
	descDepth.Height = height;    // высота текстуры
	descDepth.MipLevels = 1;            // уровень интерполяции
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // формат (размер пикселя)
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;         // вид - буфер глубин
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	// При помощи заполненной структуры-описания создаем объект текстуры
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr)) return hr;

	// Теперь надо создать сам объект буфера глубин
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;            // Структура с параметрами
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;         // формат как в текстуре
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	// При помощи заполненной структуры-описания и текстуры создаем объект буфера глубин
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr)) return hr;

	// Подключаем объект заднего буфера и объект буфера глубин к контексту устройства

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	return hr;
}

void Direct3dManager::SetupViewport()
{
	// Настройка вьюпорта
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	// Подключаем вьюпорт к контексту устройства
	g_pImmediateContext->RSSetViewports(1, &vp);
}

HRESULT Direct3dManager::InitGeometry()
{
	HRESULT hr = S_OK;

	hr = CreateVertexShader("shader.fx");
	if (FAILED(hr)) return hr;

	// Подключение шаблона вершин
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	hr = CreatePixelShader("shader.fx");
	if (FAILED(hr))return hr;
	
	hr = SetupVertexBuffer();
	if (FAILED(hr))return hr;

	return S_OK;
}

HRESULT Direct3dManager::CreateVertexShader(LPCSTR shaderName)
{
	HRESULT hr = S_OK;
	// Компиляция вершинного шейдера из файла
	ID3DBlob* pVSBlob = NULL; // Вспомогательный объект - просто место в оперативной памяти
	hr = CompileShaderFromFile(shaderName, "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Невозможно скомпилировать файл FX. Пожалуйста, запустите данную программу из папки, содержащей файл FX.", "Ошибка", MB_OK);
		return hr;
	}
	// Создание вершинного шейдера
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}
	// Определение шаблона вершин
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			/* семантическое имя, семантический индекс, размер, входящий слот (0-15), адрес начала данных в буфере вершин, класс входящего слота (не важно), InstanceDataStepRate (не важно) */
	};
	UINT numElements = ARRAYSIZE(layout);

	// Создание шаблона вершин
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	return hr;
}
HRESULT Direct3dManager::CreatePixelShader(LPCSTR shaderName)
{
	HRESULT hr = S_OK;

	// Компиляция пиксельного шейдера из файла
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(shaderName, "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Невозможно скомпилировать файл FX. Пожалуйста, запустите данную программу из папки, содержащей файл FX.", "Ошибка", MB_OK);
		return hr;
	}
	// Создание пиксельного шейдера
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	return hr;
}
HRESULT Direct3dManager::SetupVertexBuffer()
{
	HRESULT hr = S_OK;
	// Создание буфера вершин (три вершины треугольника)
	SimpleVertex vertices[]
	{  /* координаты X, Y, Z                          цвет R, G, B, A     */

			{ DirectX::XMFLOAT3(0.0f,  3.0f,  0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f,  0.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(1.0f,  0.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f,  0.0f,  1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(1.0f,  0.0f,  1.0f), DirectX::XMFLOAT4(.0f, 1.0f, 0.0f, 1.0f) },
			
			{ DirectX::XMFLOAT3(0.0f,  -3.0f,  0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd;  // Структура, описывающая создаваемый буфер
	ZeroMemory(&bd, sizeof(bd));                    // очищаем ее
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 6; // размер буфера = размер одной вершины * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;          // тип буфера - буфер 
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData; // Структура, содержащая данные буфера
	ZeroMemory(&InitData, sizeof(InitData)); // очищаем 
	InitData.pSysMem = vertices;               // указатель на вершины

	// Вызов метода g_pd3dDevice создаст объект буфера вершин ID3D11Buffer
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr)) return hr;
	
	hr = CreateIndexBuffer(bd, InitData);
	if (FAILED(hr))
		return hr;

	// Установка буфера вершин:
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Установка буфера индексов
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Установка способа отрисовки вершин в буфере
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	hr = CreateConstantBuffer(bd);
	return hr;
}

HRESULT Direct3dManager::CreateIndexBuffer(D3D11_BUFFER_DESC bd, D3D11_SUBRESOURCE_DATA InitData)
{
	HRESULT hr = S_OK;
	// Создание буфера индексов:
	// Создание массива с данными
	WORD indices[] =
	{
		0, 2, 1,      /* Треугольник 1 = vertices[0], vertices[2], vertices[1] */
		0, 3, 4,      /* Треугольник 2 = vertices[0], vertices[3], vertices[4] */
		0, 1, 3,      /* и т. д. */
		0, 4, 2,

		5, 1, 2,
		5, 4, 3,
		5, 3, 1,
		5, 2, 4

	};
	bd.Usage = D3D11_USAGE_DEFAULT;            // Структура, описывающая создаваемый буфер
	bd.ByteWidth = sizeof(WORD) * 24; // для 6 треугольников необходимо 18 вершин
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; // тип - буфер индексов
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;         // указатель на наш массив индексов

	// Вызов метода g_pd3dDevice создаст объект буфера индексов
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	return hr;
}
HRESULT Direct3dManager::CreateConstantBuffer(D3D11_BUFFER_DESC bd)
{
	HRESULT hr = S_OK;
	// Создание константного буфера
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);            // размер буфера = размеру структуры
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // тип - константный буфер
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
	return hr;
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

HRESULT Direct3dManager::InitMatrixes()
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;           // получаем ширину
	UINT height = rc.bottom - rc.top;   // и высоту окна

	// Инициализация матрицы мира
	g_World = DirectX::XMMatrixIdentity();

	// Инициализация матрицы вида
	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(0.0f, -2.0f, -10.0f, 0.0f);  // Откуда смотрим(x,y,z, angle)
	DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);    // Куда смотрим
	DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);    // Направление верха
	g_View = DirectX::XMMatrixLookAtLH(Eye, At, Up);

	// Инициализация матрицы проекции
	g_Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

	return S_OK;
}
void Direct3dManager::SetMatrixes(float fAngle)
{
	// Обновление переменной-времени
	static float t = 0.0f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)DirectX::XM_PI * 0.0125f;
	}
	else
	{
		static ULONGLONG dwTimeStart = 0;
		ULONGLONG dwTimeCur = GetTickCount64();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 3000.0f;
	}
	// Матрица-орбита: позиция объекта
	DirectX::XMMATRIX mOrbit = DirectX::XMMatrixRotationY(-t + fAngle);
	// Матрица-спин: вращение объекта вокруг своей оси
	DirectX::XMMATRIX mSpin = DirectX::XMMatrixRotationY(t * 2);
	// Матрица-позиция: перемещение на три единицы влево от начала координат
	DirectX::XMMATRIX mTranslate = DirectX::XMMatrixTranslation(-3.0f, 0.0f, 0.0f);
	// Матрица-масштаб: сжатие объекта в 2 раза
	DirectX::XMMATRIX mScale = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
	// Результирующая матрица
	//  --Сначала мы в центре, в масштабе 1:1:1, повернуты по всем осям на 0.0f.
	//  --Сжимаем -> поворачиваем вокруг Y (пока мы еще в центре) -> переносим влево ->
	//  --снова поворачиваем вокруг Y.
	g_World = mScale * mSpin * mTranslate * mOrbit;
	//Собственно, читаем комментарии, которые я сделал макси

	// Обновить константный буфер
	// создаем временную структуру и загружаем в нее матрицы
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	// загружаем временную структуру в константный буфер g_pConstantBuffer
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
}
void Direct3dManager::Render()
{
	// Очистить задний буфер
	float ClearColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f }; // красный, зеленый, синий, альфа-канал
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	// Очистить буфер глубин до 1.0 (максимальное значение)
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Для шести пирамидок

	for (int i = 0; i < 6; i++) 
	{
		// Устанавливаем матрицу, параметр - положение относительно оси Y в радианах
		SetMatrixes(i * (DirectX::XM_PI * 2) / 6);
		// Рисуем i-тую пирамидку
		g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
		g_pImmediateContext->DrawIndexed(24, 0, 0);
	}

	// Вывести в передний буфер (на экран) информацию, нарисованную в заднем буфере.
	g_pSwapChain->Present(0, 0);
}
void Direct3dManager::Render(void* procedure)
{
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // красный, зеленый, синий, альфа-канал

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	// Выбросить задний буфер на экран
	g_pSwapChain->Present(0, 0);
}
