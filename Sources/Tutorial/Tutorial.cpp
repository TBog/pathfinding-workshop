#include "pch.h"

#include "resource.h"

#include "Utils\Utils.h"
#include "Utils\FrameTime.h"
#include "Utils\Shader.h"
#include "Utils\Camera.h"
#include "Utils\Input.h"
#include "Utils\Mesh.h"
#include "Utils\Material.h"
#include "Utils\Texture.h"
#include "Utils\Object.h"
#include "Utils\Entity.h"

#include "Render\RenderManager.h"
#include "Render\TexturesManager.h"
#include "Render\ObjectsManager.h"
#include "Render\EntitiesManager.h"
#include "Pathfinding\PathfindingWorkshopManager.h"

#include "Render\PostProcess.h"
#include "Render\Sky.h"
#include "Render\Clouds.h"
#include "Render\Terrain.h"

#include "World\World.h"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
LPCWSTR g_wndClassName = L"Direct3DWindowClass";

HWND g_hWnd = NULL;

int g_resolutionWidth = 1280;
int g_resolutionHeight = 720;

Texture* g_HDRRenderTarget = NULL;
Texture* g_depthBuffer = NULL;

Input* g_input = NULL;

World* g_world = NULL;

DynVec<Entity*> g_visibleEntitiesList(1024, 1024);

ShaderVS g_CubeVS(L"Shaders/Cube_VS.fx");
ShaderPS g_CubePS(L"Shaders/Cube_PS.fx");

Texture* g_quadTexture = NULL;

Mesh* g_cubeMesh = NULL;
Texture* g_cubeTexture_baseColor = NULL;
Texture* g_cubeTexture_normal = NULL;
Texture* g_cubeTexture_roughness = NULL;
Texture* g_cubeTexture_ambientOcclusion = NULL;
D3DXMATRIX g_cubeWorldMatrix;
ID3D11Buffer* g_Cube_VS_CB = NULL;
ID3D11Buffer* g_Cube_PS_CB = NULL;

struct CB_VS_CUBE
{
	D3DXMATRIX  m_World;
};

struct CB_PS_CUBE
{
	D3DXVECTOR4 m_pad1;
};

Object* g_testObj = NULL;

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void InitializeApp();
void DestroyApp();

void Update(float dt);
void Render(float dt);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	InitializeApp();

	// Main message loop:
	bool bGotMsg;
	MSG msg;

	g_renderManager->GetFrameTime().Init();

	do
	{
		// Use PeekMessage() so we can use idle time to render the scene. 
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);

		if (bGotMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			g_renderManager->GetFrameTime().Update();

			float dt = g_renderManager->GetFrameTime().GetDt();
			Update(dt);
			Render(dt);
		}
	} while (WM_QUIT != msg.message);

	DestroyApp();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Processes messages for the main window.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_input)
		g_input->ProcessMessage(message, wParam, lParam);

	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//--------------------------------------------------------------------------------------
// Create camera
//--------------------------------------------------------------------------------------
void InitializeCamera()
{
	D3DXVECTOR3 eyePos(7.f, 1.f, 17.f);
	D3DXVECTOR3 lookAtPos(-2.5f, 10.f, -9.f);
	g_renderManager->GetCamera()->SetViewParams(eyePos, lookAtPos);

	float fov = (float)D3DX_PI / 4;
	float aspectRatio = (float)g_resolutionWidth / (float)g_resolutionHeight;
	float nearPlane = 0.1f;
	float farPlane = 5000.f;

	g_renderManager->GetCamera()->SetProjParams(fov, aspectRatio, nearPlane, farPlane);
}


//--------------------------------------------------------------------------------------
// Create resources for a cube mesh
//--------------------------------------------------------------------------------------
void _CreateCubeMesh(ID3D11InputLayout* _cubeVertexLayout)
{
	// vertex buffer & index buffer
	struct CubeVertex
	{
		D3DXVECTOR3 m_position;
		D3DXVECTOR3 m_normal;
		D3DXVECTOR3 m_tangent;
		D3DXVECTOR2 m_uv;
	};

	CubeVertex dataVB[6 * 4];
	int dataVBSizeInBytes = sizeof(dataVB);
	int dataVBStride = sizeof(CubeVertex);

	WORD dataIB[6 * 6];
	int indexDataSizeInBytes = sizeof(dataIB);

	D3DXVECTOR3 faceNormals[6] = { { 0, 0, -1 }, { 1, 0, 0 }, { 0, 0, 1 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 } };
	D3DXVECTOR3 faceTangents[6] = { { 1, 0, 0 }, { 0, 0, 1 }, { -1, 0, 0 }, { 0, 0, -1 }, { 1, 0, 0 }, { 1, 0, 0 } };
	D3DXVECTOR2 quadUVs[4] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
	WORD quadIndices[6] = { 0, 1, 2, 1, 3, 2 };

	for (int faceIdx = 0; faceIdx < 6; faceIdx++)
	{
		int vertexOffset = faceIdx * 4;
		int indexOffset = faceIdx * 6;

		D3DXVECTOR3& normal = faceNormals[faceIdx];
		D3DXVECTOR3& tangent = faceTangents[faceIdx];
		D3DXVECTOR3 binormal;
		D3DXVec3Cross(&binormal, &tangent, &normal);
		D3DXMATRIX tangentSpace;
		D3DXMatrixIdentity(&tangentSpace);
		*(D3DXVECTOR3*)(&tangentSpace._11) = tangent;
		*(D3DXVECTOR3*)(&tangentSpace._21) = normal;
		*(D3DXVECTOR3*)(&tangentSpace._31) = binormal;

		for (int vertexIdx = 0; vertexIdx < 4; vertexIdx++)
		{
			D3DXVECTOR3 localPos = D3DXVECTOR3(quadUVs[vertexIdx].x - 0.5f, 0.5f, 0.5f - quadUVs[vertexIdx].y);

			CubeVertex& crtVertex = dataVB[vertexOffset + vertexIdx];
			D3DXVec3TransformCoord(&crtVertex.m_position, &localPos, &tangentSpace);
			crtVertex.m_normal = normal;
			crtVertex.m_tangent = tangent;
			crtVertex.m_uv = quadUVs[vertexIdx];
		}

		for (int indexIdx = 0; indexIdx < 6; indexIdx++)
		{
			dataIB[indexOffset + indexIdx] = vertexOffset + quadIndices[indexIdx];
		}
	}

	g_cubeMesh = new Mesh((BYTE*)dataVB, dataVBSizeInBytes, dataVBStride, (BYTE*)dataIB, indexDataSizeInBytes, _cubeVertexLayout);
}

void _CreateDebugSphereMesh(ID3D11InputLayout* _cubeVertexLayout)
{
	// vertex buffer & index buffer
	struct CubeVertex
	{
		D3DXVECTOR3 m_position;
		D3DXVECTOR3 m_normal;
		D3DXVECTOR3 m_tangent;
		D3DXVECTOR2 m_uv;
	};

	const float R = 0.5f;
	const int N = 16, M = 32;
	const int vertexCount = 2 + (2 * N - 1) * M;
	CubeVertex dataVB[vertexCount];
	int dataVBSizeInBytes = sizeof(dataVB);
	int dataVBStride = sizeof(CubeVertex);

	const int indexCount = 2 * M * 3 + 2 * (N - 1) * M * 6;
	WORD dataIB[indexCount];
	int indexDataSizeInBytes = sizeof(dataIB);

	// vertex buffer
	int vtxId = 0;
	dataVB[vtxId++].m_position = D3DXVECTOR3(0, R, 0);

	for (int i = 0; i < N - 1; i++)
	{
		float sliceAngle = ((float)D3DX_PI * 0.5f) / N;
		float b = (float)D3DX_PI * 0.5f - sliceAngle * (i + 1);
		float y = R * sinf(b);
		for (int j = 0; j < M; j++)
		{
			float a = j * 2 * (float)D3DX_PI / M;
			float x = R * cosf(a) * cosf(b);
			float z = R * sinf(a) * cosf(b);
			dataVB[vtxId++].m_position = D3DXVECTOR3(x, y, z);
		}
	}

	for (int j = 0; j < M; j++)
	{
		float a = j * 2 * (float)D3DX_PI / M;
		float x = R * cosf(a);
		float z = R * sinf(a);
		dataVB[vtxId++].m_position = D3DXVECTOR3(x, 0, z);
	}

	for (int i = 0; i < N - 1; i++)
	{
		float sliceAngle = ((float)D3DX_PI * 0.5f) / N;
		float b = sliceAngle * (i + 1);
		float y = -R * sinf(b);
		for (int j = 0; j < M; j++)
		{
			float a = j * 2 * (float)D3DX_PI / M;
			float x = R * cosf(a) * cosf(b);
			float z = R * sinf(a) * cosf(b);
			dataVB[vtxId++].m_position = D3DXVECTOR3(x, y, z);
		}
	}

	dataVB[vtxId++].m_position = D3DXVECTOR3(0, -R, 0);

	D3DXVECTOR3 vUp = D3DXVECTOR3(0, 1, 0);
	for (int i = 0; i < vertexCount; i++)
	{
		D3DXVec3Normalize(&dataVB[i].m_normal, &dataVB[i].m_position);
		D3DXVec3Cross(&dataVB[i].m_tangent, &dataVB[i].m_normal, &vUp);
		dataVB[i].m_uv = D3DXVECTOR2(0, 0);
	}

	// index buffer
	int idxId = 0;
	WORD quadIndices[6] = { 0, 1, 2, 1, 3, 2 };
	for (int j = 0; j < M; j++)
	{
		dataIB[idxId++] = 0;
		dataIB[idxId++] = 1 + (j + 1) % M;
		dataIB[idxId++] = 1 + j;
	}

	for (int i = 0; i < 2 * (N - 1); i++)
	{
		for (int j = 0; j < M; j++)
		{
			int vtx[4];
			vtx[0] = 1 + i * M + j;
			vtx[1] = 1 + i * M + (j + 1) % M;
			vtx[2] = 1 + (i + 1) * M + j;
			vtx[3] = 1 + (i + 1) * M + (j + 1) % M;

			for (int k = 0; k < 6; k++)
			{
				dataIB[idxId++] = vtx[quadIndices[k]];
			}
		}
	}

	int baseVtx = 1 + 2 * (N - 1) * M;
	for (int j = 0; j < M; j++)
	{
		dataIB[idxId++] = baseVtx + j;
		dataIB[idxId++] = baseVtx + (j + 1) % M;
		dataIB[idxId++] = vertexCount - 1;
	}

	g_cubeMesh = new Mesh((BYTE*)dataVB, dataVBSizeInBytes, dataVBStride, (BYTE*)dataIB, indexDataSizeInBytes, _cubeVertexLayout);
}

void CreateCubeMeshResources()
{
	HRESULT hr = S_OK;

	// input layout
	const D3D11_INPUT_ELEMENT_DESC cubeLayout[] =
	{
		{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0,  36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D11InputLayout* cubeVertexLayout = NULL;
	hr = g_renderManager->GetDevice()->CreateInputLayout(cubeLayout, ARRAYSIZE(cubeLayout), g_CubeVS.GetBufferPointer(), g_CubeVS.GetBufferSize(), &cubeVertexLayout);

	_CreateCubeMesh(cubeVertexLayout);
	// _CreateDebugSphereMesh( cubeVertexLayout );

	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;

	Desc.ByteWidth = sizeof(CB_VS_CUBE);
	g_renderManager->GetDevice()->CreateBuffer(&Desc, NULL, &g_Cube_VS_CB);

	Desc.ByteWidth = sizeof(CB_PS_CUBE);
	g_renderManager->GetDevice()->CreateBuffer(&Desc, NULL, &g_Cube_PS_CB);

	D3DXMatrixIdentity(&g_cubeWorldMatrix);

	g_cubeTexture_baseColor = g_texturesManager->Load(L"Textures/roughrockface4_Base_Color.png");
	g_cubeTexture_normal = g_texturesManager->Load(L"Textures/roughrockface4_Normal.png");
	g_cubeTexture_roughness = g_texturesManager->Load(L"Textures/roughrockface4_Roughness.png");
	g_cubeTexture_ambientOcclusion = g_texturesManager->Load(L"Textures/roughrockface4_Ambient_Occlusion.png");
}

//--------------------------------------------------------------------------------------
// Delete the resources for a cube mesh
//--------------------------------------------------------------------------------------
void DeleteCubeMeshResources()
{
	SAFE_RELEASE(g_cubeTexture_baseColor);
	SAFE_RELEASE(g_cubeTexture_normal);
	SAFE_RELEASE(g_cubeTexture_roughness);
	SAFE_RELEASE(g_cubeTexture_ambientOcclusion);
	SAFE_RELEASE(g_Cube_VS_CB);
	SAFE_RELEASE(g_Cube_PS_CB);
	SAFE_DELETE(g_cubeMesh);
}

//--------------------------------------------------------------------------------------
// Create D3D resources
//--------------------------------------------------------------------------------------
void CreateD3DResources()
{
	g_HDRRenderTarget = new Texture(g_resolutionWidth, g_resolutionHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, false, eTextureType_RenderTarget);
	g_depthBuffer = new Texture(g_resolutionWidth, g_resolutionHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, false, eTextureType_DepthStencil);

	g_world = new World(L"Worlds/world_pathfind.xml");
	//g_world = new World( L"Worlds/world1.xml" );
	//g_world = new World( L"Worlds/world2.xml" );

	//g_quadTexture = g_texturesManager->Load( L"Textures/cloudySky.dds" );

	CreateCubeMeshResources();

	//g_testObj = g_objectsManager->Load( L"Models/Ambulance/Ambulance.obj", 1.f );
	// g_testObj = g_objectsManager->Load( L"Models/Discobolous/CB_Discobolus_LOD0.obj", 0.01f );
	// g_testObj = g_objectsManager->Load( L"Models/Forklift/forklift.obj", 1.f );
	// g_testObj = g_objectsManager->Load( L"Models/Lamborginhi_Aventador/Lamborghini_Aventador.obj", 0.01f );
	// g_testObj = g_objectsManager->Load( L"Models/Rock/rock.obj", 0.01f );
	// g_testObj = g_objectsManager->Load( L"Models/Teapot/teapot_s0.obj", 1.f );
	// g_testObj = g_objectsManager->Load( L"Models/Vintage Suitcase/Vintage_Suitcase_LP.obj", 0.01f );
	// g_testObj = g_objectsManager->Load( L"Models/WoodCabin/WoodenCabinObj.obj", 0.01f );
	// g_testObj = g_objectsManager->Load( L"Models/WoodHouse/house.obj", 1.f );
}

//--------------------------------------------------------------------------------------
// Delete D3D resources
//--------------------------------------------------------------------------------------
void DeleteD3DResources()
{
	SAFE_RELEASE(g_testObj);

	SAFE_RELEASE(g_quadTexture);

	DeleteCubeMeshResources();

	SAFE_DELETE(g_world);

	SAFE_RELEASE(g_HDRRenderTarget);
	SAFE_RELEASE(g_depthBuffer);
}

//--------------------------------------------------------------------------------------
// Create window and D3D device
//--------------------------------------------------------------------------------------
void InitializeApp()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	// Register the windows class
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TUTORIAL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = g_wndClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	ATOM atom = RegisterClassExW(&wcex);
	myAssert(atom, L"RegisterClass failed!");

	// Create the render window
	g_hWnd = CreateWindow(g_wndClassName, L"Tutorial", WS_OVERLAPPEDWINDOW, 0, 0, g_resolutionWidth, g_resolutionHeight, 0, NULL, hInstance, 0);
	myAssert(g_hWnd, L"CreateWindow failed!");

	g_input = new Input();

	RenderManager::Create();
	TexturesManager::Create();
	ObjectsManager::Create();
	EntitiesManager::Create();
	PathfindingWorkshopManager::Create();

	InitializeCamera();

	g_renderManager->CreateDevice(g_hWnd);

	CreateD3DResources();

	ShowWindow(g_hWnd, SW_SHOWNORMAL);
	UpdateWindow(g_hWnd);
}

//--------------------------------------------------------------------------------------
// Destroy window and device
//--------------------------------------------------------------------------------------
void DestroyApp()
{
	DeleteD3DResources();

	g_renderManager->DestroyDevice();

	EntitiesManager::Destroy();
	ObjectsManager::Destroy();
	TexturesManager::Destroy();
	RenderManager::Destroy();
	PathfindingWorkshopManager::Destroy();

	SAFE_DELETE(g_input);

	DestroyWindow(g_hWnd);
	UnregisterClass(g_wndClassName, NULL);
}

//--------------------------------------------------------------------------------------
// Shader Reload logic
//--------------------------------------------------------------------------------------
bool g_isShaderReloadShortcutPressed = false;

void UpdateShaderReloadMonitor()
{
	if (g_input->IsAltPressed() && g_input->IsKeyPressed('R'))
	{
		if (!g_isShaderReloadShortcutPressed)
		{
			g_isShaderReloadShortcutPressed = true;

			Shader::ReloadAll();
		}
	}
	else
	{
		g_isShaderReloadShortcutPressed = false;
	}
}

//--------------------------------------------------------------------------------------
// Render the cube mesh
//--------------------------------------------------------------------------------------
void RenderCubeMesh()
{
	ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

	// Set the blend state and the depth stencil state
	float BlendFactor[4] = { 0, 0, 0, 0 };
	d3dImmediateContext->OMSetBlendState(g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF);
	d3dImmediateContext->OMSetDepthStencilState(g_renderManager->GetDepthWriteState(), 0);
	d3dImmediateContext->RSSetState(g_renderManager->GetCullRasterState());

	// Set the shaders
	g_CubeVS.Set();
	g_CubePS.Set();

	// Set texture SRV
	g_cubeTexture_baseColor->PSSet(0);
	g_cubeTexture_normal->PSSet(1);
	g_cubeTexture_roughness->PSSet(2);
	g_cubeTexture_ambientOcclusion->PSSet(3);

	ID3D11SamplerState* samWrapLinear = g_renderManager->GetSamWrapLinear();
	d3dImmediateContext->PSSetSamplers(0, 1, &samWrapLinear);

	{
		CB_VS_CUBE* cube_VS_CB = (CB_VS_CUBE*)g_renderManager->LockConstantBuffer(eConstantBufferType_PerObjectVS);

		D3DXMatrixTranspose(&cube_VS_CB->m_World, &g_cubeWorldMatrix);

		g_renderManager->UnlockConstantBuffer(eConstantBufferType_PerObjectVS);
	}

	{
		CB_PS_CUBE* cube_PS_CB = (CB_PS_CUBE*)g_renderManager->LockConstantBuffer(eConstantBufferType_PerObjectPS);

		g_renderManager->UnlockConstantBuffer(eConstantBufferType_PerObjectPS);
	}

	g_cubeMesh->Draw();
}

//--------------------------------------------------------------------------------------
// Computes the list of entities that pass the camera frustum test
//--------------------------------------------------------------------------------------
void ComputeVisibleEntities(DynVec<Entity*>& outVisibleEntitiesList, const DynVec<Entity*>& entitiesList)
{
	outVisibleEntitiesList.Clear();

	int entitiesCount = entitiesList.GetSize();
	for (int i = 0; i < entitiesCount; i++)
	{
		Entity* entity = entitiesList[i];

		constexpr bool bIsFullyInFrustum = false;
		if (g_renderManager->GetCamera()->IsBoundingSphereInFrustum(entity->GetBoundingSphere_center(), entity->GetBoundingSphere_radius(), bIsFullyInFrustum) && g_renderManager->GetCamera()->IsAABBInFrustum(entity->GetBoundingAABB_min(), entity->GetBoundingAABB_max(), bIsFullyInFrustum))
		{
			outVisibleEntitiesList.Add(entity);
		}
	}
}

//--------------------------------------------------------------------------------------
// Main update function of the application
//--------------------------------------------------------------------------------------
void Update(float dt)
{
	g_input->Update();

	UpdateShaderReloadMonitor();

	float moveAxisX = g_input->GetGamePadState(0).m_thumbLeftX;
	float moveAxisY = g_input->GetGamePadState(0).m_thumbLeftY;
	float lookAxisX = g_input->GetGamePadState(0).m_thumbRightX;
	float lookAxisY = g_input->GetGamePadState(0).m_thumbRightY;
	float speedScale = 1.f;
	if (g_input->GetGamePadState(0).m_rightTrigger > 0.25f)
		speedScale = 10.f;
	if (g_input->GetGamePadState(0).m_buttonStates & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		speedScale = 100.f;

	g_renderManager->GetCamera()->UpdateBasedOnInput(dt, moveAxisX, moveAxisY, lookAxisX, lookAxisY, speedScale);
	g_pathfindingWorkshopManager->Update(dt);
}

//--------------------------------------------------------------------------------------
// Main render function of the application
//--------------------------------------------------------------------------------------
void Render(float dt)
{
	// set per frame constants
	g_renderManager->SetPerFrameConstantBuffers();

	// clear back buffer render target and depth buffer
	g_renderManager->ClearBackBufferRenderTarget(D3DXCOLOR(1.f, 0.f, 0.f, 1.f), 1.f);

	// pre-render passes
	g_renderManager->GetClouds()->PreRender();

	// set the HDR render target
	g_renderManager->SetRenderTargets(g_HDRRenderTarget, g_depthBuffer);

	g_HDRRenderTarget->ClearRenderTarget(D3DXCOLOR(1.f, 0.f, 0.f, 1.f));
	g_depthBuffer->ClearDepthStencil(1.f);


	// render the sky
	g_renderManager->GetSky()->Render();

	// render our tests
	// g_renderManager->RenderQuadMesh( g_quadTexture );
	// RenderCubeMesh( );
	// g_renderManager->RenderObject( g_testObj, g_cubeWorldMatrix );

	// render the visible entities
	ComputeVisibleEntities(g_visibleEntitiesList, g_world->GetEntitiesList());
	g_renderManager->RenderEntities(g_visibleEntitiesList);

	// render the terrain
	if (g_world->GetTerrain())
		g_world->GetTerrain()->Render();

	// render the clouds
	g_renderManager->GetClouds()->Render(g_HDRRenderTarget, g_depthBuffer);

	// set back the back buffer render target
	g_renderManager->SetBackBufferRenderTarget();

	// apply post process
	g_renderManager->GetPostProcess()->Render(g_HDRRenderTarget, g_depthBuffer);


	// present
	g_renderManager->Present();
}
