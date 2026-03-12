#include "pch.h"

#include "RenderManager.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Camera.h"
#include "..\Utils\Shader.h"
#include "..\Utils\Texture.h"
#include "..\Utils\Material.h"
#include "..\Utils\Mesh.h"
#include "..\Utils\Object.h"
#include "..\Utils\Entity.h"

#include "PostProcess.h"
#include "Sky.h"
#include "Clouds.h"

#include "..\World\World.h"

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------
ShaderVS g_FullScreenQuadVS(L"Shaders/FullScreenQuad_VS.fx");
ShaderPS g_FullScreenQuadPS(L"Shaders/FullScreenQuad_PS.fx");
ShaderVS g_ObjectVS(L"Shaders/Object_VS.fx");
ShaderPS g_ObjectPS(L"Shaders/Object_PS.fx");

//--------------------------------------------------------------------------------------
// Constant buffers structs
//--------------------------------------------------------------------------------------
struct CB_VS_FRAME
{
	D3DXMATRIX      m_viewProjMatrix;
	D3DXVECTOR3     m_cameraPos;
	float           m_pad1;
	float           g_fogStartZ;
	float           g_invFogSize;
};

struct CB_PS_FRAME
{
	D3DXVECTOR3     m_cameraPos;
	float           m_pad1;
	D3DXVECTOR4     m_projParams;
	D3DXMATRIX      m_invViewMatrix;
	D3DXVECTOR3     m_lightDir;
	float           m_pad2;
	D3DXVECTOR3     m_lightColor;
	float           m_lightPower;
	D3DXVECTOR3     m_ambientColor;
	float           m_ambientPower;
	D3DXVECTOR3     m_fogColor;
	float           m_fogLighPower;

};

struct CB_VS_OBJECT
{
	D3DXMATRIX  m_World;
};

struct CB_PS_OBJECT
{
	D3DXVECTOR4 m_pad1;
};

struct CB_PS_OBJECT_MATERIAL
{
	float       m_diffuseTextureEnabled;
	float       m_normalTextureEnabled;
	float       m_roughnessTextureEnabled;
	float       m_ambientOcclusionTextureEnabled;
};

//-------------------------------------------------------------------

RenderManager* RenderManager::s_Instance = NULL;

RenderManager::RenderManager()
	: m_hWnd(NULL)
	, m_camera(NULL)
	, m_postProcess(NULL)
	, m_sky(NULL)
	, m_clouds(NULL)
	, m_d3dDevice(NULL)
	, m_d3dImmediateContext(NULL)
	, m_d3dSwapChain(NULL)
	, m_backBuffer_RenderTargetView(NULL)
	, m_backBuffer_DepthStencilView(NULL)
	, m_depthWriteState(NULL)
	, m_noDepthWriteState(NULL)
	, m_noDepthTestAndNoWriteState(NULL)
	, m_cullRasterState(NULL)
	, m_blendEnabledState(NULL)
	, m_noBlendState(NULL)
	, m_samWrapLinear(NULL)
	, m_samWrapAniso(NULL)
	, m_samClampLinear(NULL)
	, m_samPoint(NULL)
	, m_defaultObjectVertexLayout(NULL)
	, m_resolutionWidth(0)
	, m_resolutionHeight(0)
	, m_quadMesh(NULL)
{
	m_camera = new Camera();
	m_postProcess = new PostProcess();
	m_sky = new Sky();
	m_clouds = new Clouds();
}

//-------------------------------------------------------------------

RenderManager::~RenderManager()
{
	DestroyDevice();

	SAFE_DELETE(m_camera);
	SAFE_DELETE(m_postProcess);
	SAFE_DELETE(m_sky);
	SAFE_DELETE(m_clouds);
}

//-------------------------------------------------------------------

void RenderManager::Create()
{
	if (s_Instance)
	{
		myAssert(false, L"RenderManager::Create() already called !");
		return;
	}

	s_Instance = new RenderManager();
}

//-------------------------------------------------------------------

void RenderManager::Destroy()
{
	SAFE_DELETE(s_Instance);
}

//-------------------------------------------------------------------

HRESULT RenderManager::CreateDevice(HWND hWnd)
{
	HRESULT hr;

	m_hWnd = hWnd;

	RECT rect;
	GetWindowRect(m_hWnd, &rect);
	m_resolutionWidth = rect.right - rect.left;
	m_resolutionHeight = rect.bottom - rect.top;

	// Create D3D device and swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = m_resolutionWidth;
	sd.BufferDesc.Height = m_resolutionHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed = TRUE;


	D3D_FEATURE_LEVEL  FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT               numFeatureLevelsRequested = 1;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	UINT deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (FAILED(hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		deviceFlags,
		&FeatureLevelsRequested,
		numFeatureLevelsRequested,
		D3D11_SDK_VERSION,
		&sd,
		&m_d3dSwapChain,
		&m_d3dDevice,
		&FeatureLevelsSupported,
		&m_d3dImmediateContext)))
	{
		myAssert(false, L"D3D11CreateDeviceAndSwapChain failed!");
		return hr;
	}



	ID3D11Texture2D* backBuffer = NULL;

	// Get a pointer to the back buffer
	hr = m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	D3D_DEBUG_SET_NAME(backBuffer, "Back Buffer");

	// Create a render-target view
	m_d3dDevice->CreateRenderTargetView(backBuffer, NULL, &m_backBuffer_RenderTargetView);
	D3D_DEBUG_SET_NAME(m_backBuffer_RenderTargetView, "Back Buffer RT View");

	SAFE_RELEASE(backBuffer);

	// Create depth stencil texture
	ID3D11Texture2D* depthStencil = NULL;
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = m_resolutionWidth;
	descDepth.Height = m_resolutionHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_d3dDevice->CreateTexture2D(&descDepth, NULL, &depthStencil);
	myAssert(SUCCEEDED(hr), L"Create depth buffer failed!");
	D3D_DEBUG_SET_NAME(depthStencil, "Depth Buffer");

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.Flags = 0;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_d3dDevice->CreateDepthStencilView(depthStencil, &descDSV, &m_backBuffer_DepthStencilView);
	D3D_DEBUG_SET_NAME(m_backBuffer_DepthStencilView, "Depth Buffer View");

	SAFE_RELEASE(depthStencil);

	// Bind the view
	SetBackBufferRenderTarget();


	Shader::ReloadAll();

	// Create a less than or equal comparison depthstencil state
	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(DSDesc));
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.StencilEnable = FALSE;
	m_d3dDevice->CreateDepthStencilState(&DSDesc, &m_depthWriteState);

	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_d3dDevice->CreateDepthStencilState(&DSDesc, &m_noDepthWriteState);

	DSDesc.DepthEnable = FALSE;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	DSDesc.StencilEnable = FALSE;
	m_d3dDevice->CreateDepthStencilState(&DSDesc, &m_noDepthTestAndNoWriteState);

	D3D11_RASTERIZER_DESC RSDesc;
	RSDesc.AntialiasedLineEnable = FALSE;
	RSDesc.CullMode = D3D11_CULL_BACK;
	RSDesc.DepthBias = 0;
	RSDesc.DepthBiasClamp = 0.0f;
	RSDesc.DepthClipEnable = TRUE;
	RSDesc.FillMode = D3D11_FILL_SOLID;
	RSDesc.FrontCounterClockwise = FALSE;
	RSDesc.MultisampleEnable = FALSE;
	RSDesc.ScissorEnable = FALSE;
	RSDesc.SlopeScaledDepthBias = 0.0f;
	m_d3dDevice->CreateRasterizerState(&RSDesc, &m_cullRasterState);

	D3D11_BLEND_DESC BSDesc;
	ZeroMemory(&BSDesc, sizeof(D3D11_BLEND_DESC));

	BSDesc.RenderTarget[0].BlendEnable = TRUE;
	BSDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BSDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BSDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BSDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BSDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BSDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BSDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

	m_d3dDevice->CreateBlendState(&BSDesc, &m_blendEnabledState);

	BSDesc.RenderTarget[0].BlendEnable = FALSE;
	m_d3dDevice->CreateBlendState(&BSDesc, &m_noBlendState);

	// Create a sampler states
	D3D11_SAMPLER_DESC SamDesc;
	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.MipLODBias = 0.0f;
	SamDesc.MaxAnisotropy = 1;
	SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
	SamDesc.MinLOD = 0;
	SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_d3dDevice->CreateSamplerState(&SamDesc, &m_samWrapLinear);

	SamDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	SamDesc.MaxAnisotropy = 16;
	m_d3dDevice->CreateSamplerState(&SamDesc, &m_samWrapAniso);

	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.MaxAnisotropy = 1;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	m_d3dDevice->CreateSamplerState(&SamDesc, &m_samClampLinear);

	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	m_d3dDevice->CreateSamplerState(&SamDesc, &m_samPoint);

	// default objects input layout
	const D3D11_INPUT_ELEMENT_DESC defaultObjectLayout[] =
	{
		{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0,  48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = g_renderManager->GetDevice()->CreateInputLayout(defaultObjectLayout, ARRAYSIZE(defaultObjectLayout), g_ObjectVS.GetBufferPointer(), g_ObjectVS.GetBufferSize(), &m_defaultObjectVertexLayout);

	_CreateConstantBuffers();

	_CreateQuadMeshResources();

	m_clouds->CreateResources();

	return S_OK;
}

//-------------------------------------------------------------------

void RenderManager::DestroyDevice()
{
	m_clouds->DestroyResources();

	_DeleteQuadMeshResources();

	_DeleteConstantBuffers();

	Shader::UnloadAll();

	SAFE_RELEASE(m_depthWriteState);
	SAFE_RELEASE(m_noDepthWriteState);
	SAFE_RELEASE(m_noDepthTestAndNoWriteState);

	SAFE_RELEASE(m_cullRasterState);

	SAFE_RELEASE(m_blendEnabledState);
	SAFE_RELEASE(m_noBlendState);

	SAFE_RELEASE(m_samWrapLinear);
	SAFE_RELEASE(m_samWrapAniso);
	SAFE_RELEASE(m_samClampLinear);
	SAFE_RELEASE(m_samPoint);

	SAFE_RELEASE(m_defaultObjectVertexLayout);

	SAFE_RELEASE(m_backBuffer_RenderTargetView);
	SAFE_RELEASE(m_backBuffer_DepthStencilView);

	SAFE_RELEASE(m_d3dSwapChain);
	SAFE_RELEASE(m_d3dImmediateContext);
	SAFE_RELEASE(m_d3dDevice);
}

//-------------------------------------------------------------------

void RenderManager::Update(float dt)
{
}

//-------------------------------------------------------------------

void RenderManager::Present()
{
	m_d3dSwapChain->Present(0, 0);
}

//-------------------------------------------------------------------

void RenderManager::ClearBackBufferRenderTarget(const D3DXCOLOR& _ClearColor, float _fClearDepth)
{
	m_d3dImmediateContext->ClearRenderTargetView(m_backBuffer_RenderTargetView, _ClearColor);
	m_d3dImmediateContext->ClearDepthStencilView(m_backBuffer_DepthStencilView, D3D11_CLEAR_DEPTH, _fClearDepth, 0);
}

//-------------------------------------------------------------------

void RenderManager::SetBackBufferRenderTarget()
{
	m_d3dImmediateContext->OMSetRenderTargets(1, &m_backBuffer_RenderTargetView, m_backBuffer_DepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (float)m_resolutionWidth;
	vp.Height = (float)m_resolutionHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_d3dImmediateContext->RSSetViewports(1, &vp);
}

//-------------------------------------------------------------------

void RenderManager::SetRenderTargets(Texture* _textureRT, Texture* _textureDepth)
{
	SetRenderTargets(_textureRT, NULL, NULL, NULL, _textureDepth);
}

//-------------------------------------------------------------------

void RenderManager::SetRenderTargets(Texture* _textureRT0, Texture* _textureRT1, Texture* _textureRT2, Texture* _textureRT3, Texture* _textureDepth)
{
	Texture* texturesRT[4] = { _textureRT0, _textureRT1, _textureRT2, _textureRT3 };
	ID3D11RenderTargetView* RTVs[4] = { NULL, NULL, NULL, NULL };
	ID3D11DepthStencilView* DSV = NULL;

	for (int i = 0; i < 4; i++)
	{
		RTVs[i] = texturesRT[i] ? texturesRT[i]->GetRenderTargetView() : NULL;
	}

	DSV = _textureDepth ? _textureDepth->GetDepthStencilView() : NULL;

	m_d3dImmediateContext->OMSetRenderTargets(4, (ID3D11RenderTargetView**)&RTVs, DSV);


	// setup the viewport to match the render targets set
	int RTresolutionWidth = m_resolutionWidth;
	int RTresolutionHeight = m_resolutionHeight;

	if (_textureDepth)
	{
		RTresolutionWidth = _textureDepth->GetWidth();
		RTresolutionHeight = _textureDepth->GetHeight();
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			if (texturesRT[i])
			{
				RTresolutionWidth = texturesRT[i]->GetWidth();
				RTresolutionHeight = texturesRT[i]->GetHeight();

				break;
			}
		}
	}


	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (float)RTresolutionWidth;
	vp.Height = (float)RTresolutionHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_d3dImmediateContext->RSSetViewports(1, &vp);
}

//-------------------------------------------------------------------

float* RenderManager::LockConstantBuffer(EConstantBufferType _eType)
{
	HRESULT hr = S_OK;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	m_d3dImmediateContext->Map(m_constantBuffers[_eType], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

	return (float*)MappedResource.pData;
}

//-------------------------------------------------------------------

void RenderManager::UnlockConstantBuffer(EConstantBufferType _eType)
{
	m_d3dImmediateContext->Unmap(m_constantBuffers[_eType], 0);

	switch (_eType)
	{
	case eConstantBufferType_PerFrameVS:            m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffers[eConstantBufferType_PerFrameVS]);    break;
	case eConstantBufferType_PerPassVS:             m_d3dImmediateContext->VSSetConstantBuffers(1, 1, &m_constantBuffers[eConstantBufferType_PerPassVS]);    break;
	case eConstantBufferType_PerObjectVS:           m_d3dImmediateContext->VSSetConstantBuffers(2, 1, &m_constantBuffers[eConstantBufferType_PerObjectVS]);   break;
	case eConstantBufferType_PerObjectMaterialVS:   m_d3dImmediateContext->VSSetConstantBuffers(3, 1, &m_constantBuffers[eConstantBufferType_PerObjectMaterialVS]);   break;
	case eConstantBufferType_PerFramePS:            m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_constantBuffers[eConstantBufferType_PerFramePS]);    break;
	case eConstantBufferType_PerPassPS:             m_d3dImmediateContext->PSSetConstantBuffers(1, 1, &m_constantBuffers[eConstantBufferType_PerPassPS]);    break;
	case eConstantBufferType_PerObjectPS:           m_d3dImmediateContext->PSSetConstantBuffers(2, 1, &m_constantBuffers[eConstantBufferType_PerObjectPS]);   break;
	case eConstantBufferType_PerObjectMaterialPS:   m_d3dImmediateContext->PSSetConstantBuffers(3, 1, &m_constantBuffers[eConstantBufferType_PerObjectMaterialPS]);   break;
	}
}

//-------------------------------------------------------------------

void RenderManager::SetPerFrameConstantBuffers()
{
	ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

	{
		D3DXMATRIX mViewProjection = GetCamera()->GetViewMatrix() * GetCamera()->GetProjMatrix();

		CB_VS_FRAME* frame_VS_CB = (CB_VS_FRAME*)LockConstantBuffer(eConstantBufferType_PerFrameVS);

		D3DXMatrixTranspose(&frame_VS_CB->m_viewProjMatrix, &mViewProjection);
		frame_VS_CB->m_cameraPos = GetCamera()->GetCameraPos();
		frame_VS_CB->g_fogStartZ = GetSky()->GetFogStart();
		frame_VS_CB->g_invFogSize = 1.f / (GetSky()->GetFogEnd() - GetSky()->GetFogStart());

		UnlockConstantBuffer(eConstantBufferType_PerFrameVS);
	}

	{
		D3DXMATRIX mInvView = GetCamera()->GetInvViewMatrix();

		CB_PS_FRAME* frame_PS_CB = (CB_PS_FRAME*)LockConstantBuffer(eConstantBufferType_PerFramePS);

		frame_PS_CB->m_cameraPos = GetCamera()->GetCameraPos();
		frame_PS_CB->m_projParams = GetCamera()->GetProjParams();
		D3DXMatrixTranspose(&frame_PS_CB->m_invViewMatrix, &mInvView);
		frame_PS_CB->m_lightDir = GetSky()->GetLightDir();
		frame_PS_CB->m_lightColor = GetSky()->GetLightColor();
		frame_PS_CB->m_lightPower = GetSky()->GetLightPower();
		frame_PS_CB->m_ambientColor = GetSky()->GetLightAmbientColor();
		frame_PS_CB->m_ambientPower = GetSky()->GetLightAmbientPower();
		frame_PS_CB->m_fogColor = GetSky()->GetFogColor();
		frame_PS_CB->m_fogLighPower = GetSky()->GetFogLightPower();

		UnlockConstantBuffer(eConstantBufferType_PerFramePS);
	}
}

//-------------------------------------------------------------------

void RenderManager::RenderQuadMesh(const Texture* _texture)
{
	ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

	// Set the blend state and the depth stencil state
	float BlendFactor[4] = { 0, 0, 0, 0 };
	d3dImmediateContext->OMSetBlendState(g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF);
	d3dImmediateContext->OMSetDepthStencilState(g_renderManager->GetNoDepthWriteState(), 0);
	d3dImmediateContext->RSSetState(g_renderManager->GetCullRasterState());

	// Set the shaders
	g_FullScreenQuadVS.Set();
	g_FullScreenQuadPS.Set();

	// Set texture SRV
	_texture->PSSet(0);
	ID3D11SamplerState* samClampLinear = g_renderManager->GetSamClampLinear();
	d3dImmediateContext->PSSetSamplers(0, 1, &samClampLinear);
	ID3D11SamplerState* samWrapLinear = g_renderManager->GetSamWrapLinear();
	d3dImmediateContext->PSSetSamplers(1, 1, &samWrapLinear);

	m_quadMesh->Draw();
}

//-------------------------------------------------------------------

void RenderManager::RenderObject(const Object* _object, const D3DXMATRIX& _worldMatrix)
{
	ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

	// Set the blend state and the depth stencil state
	float BlendFactor[4] = { 0, 0, 0, 0 };
	d3dImmediateContext->OMSetBlendState(g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF);
	d3dImmediateContext->OMSetDepthStencilState(g_renderManager->GetDepthWriteState(), 0);
	d3dImmediateContext->RSSetState(g_renderManager->GetCullRasterState());

	// Set the shaders
	g_ObjectVS.Set();
	g_ObjectPS.Set();

	{
		CB_VS_OBJECT* object_VS_CB = (CB_VS_OBJECT*)LockConstantBuffer(eConstantBufferType_PerObjectVS);

		D3DXMatrixTranspose(&object_VS_CB->m_World, &_worldMatrix);

		UnlockConstantBuffer(eConstantBufferType_PerObjectVS);
	}

	{
		CB_PS_OBJECT* object_PS_CB = (CB_PS_OBJECT*)LockConstantBuffer(eConstantBufferType_PerObjectPS);

		UnlockConstantBuffer(eConstantBufferType_PerObjectPS);
	}

	ID3D11SamplerState* samWrapLinear = g_renderManager->GetSamWrapLinear();
	d3dImmediateContext->PSSetSamplers(0, 1, &samWrapLinear);

	for (int meshIdx = 0; meshIdx < _object->GetMeshList().GetSize(); meshIdx++)
	{
		Mesh* mesh = _object->GetMeshList()[meshIdx];
		Material* material = mesh->GetMaterial();

		bool isDiffuseTextureEnabled = false;
		bool isNormalTextureEnabled = false;
		bool isRoughnessTextureEnabled = false;
		bool isAmbientOcclusionTextureEnabled = false;

		// Set texture SRV
		Texture* diffuseTexture = material->GetTexture(eObjectTextureSlotType_Diffuse);
		if (diffuseTexture)
		{
			diffuseTexture->PSSet(0);
			isDiffuseTextureEnabled = true;
		}
		Texture* normalTexture = material->GetTexture(eObjectTextureSlotType_Normal);
		if (normalTexture)
		{
			normalTexture->PSSet(1);
			isNormalTextureEnabled = true;
		}
		Texture* roughnessTexture = material->GetTexture(eObjectTextureSlotType_Roughness);
		if (roughnessTexture)
		{
			roughnessTexture->PSSet(2);
			isRoughnessTextureEnabled = true;
		}
		Texture* ambientOcclusionTexture = material->GetTexture(eObjectTextureSlotType_AmbientOcclusion);
		if (ambientOcclusionTexture)
		{
			ambientOcclusionTexture->PSSet(3);
			isAmbientOcclusionTextureEnabled = true;
		}

		// set shader constants per material
		{
			CB_PS_OBJECT_MATERIAL* object_material_PS_CB = (CB_PS_OBJECT_MATERIAL*)LockConstantBuffer(eConstantBufferType_PerObjectMaterialPS);

			object_material_PS_CB->m_diffuseTextureEnabled = isDiffuseTextureEnabled ? 1.f : 0.f;
			object_material_PS_CB->m_normalTextureEnabled = isNormalTextureEnabled ? 1.f : 0.f;
			object_material_PS_CB->m_roughnessTextureEnabled = isRoughnessTextureEnabled ? 1.f : 0.f;
			object_material_PS_CB->m_ambientOcclusionTextureEnabled = isAmbientOcclusionTextureEnabled ? 1.f : 0.f;

			UnlockConstantBuffer(eConstantBufferType_PerObjectMaterialPS);
		}

		mesh->Draw();
	}
}

//-------------------------------------------------------------------

void RenderManager::RenderEntities(const DynVec<Entity*>& _entitiesList)
{
	int entitiesCount = _entitiesList.GetSize();
	for (int i = 0; i < entitiesCount; i++)
	{
		RenderObject(_entitiesList[i]->GetObject(), _entitiesList[i]->GetWorldMatrix());
	}
}

//-------------------------------------------------------------------

void RenderManager::_CreateConstantBuffers()
{
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;

	Desc.ByteWidth = 256 * 4 * sizeof(float);

	for (int i = 0; i < eConstantBufferType_Count; i++)
	{
		m_d3dDevice->CreateBuffer(&Desc, NULL, &m_constantBuffers[i]);
	}

	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffers[eConstantBufferType_PerFrameVS]);
	m_d3dImmediateContext->VSSetConstantBuffers(1, 1, &m_constantBuffers[eConstantBufferType_PerPassVS]);
	m_d3dImmediateContext->VSSetConstantBuffers(2, 1, &m_constantBuffers[eConstantBufferType_PerObjectVS]);
	m_d3dImmediateContext->VSSetConstantBuffers(3, 1, &m_constantBuffers[eConstantBufferType_PerObjectMaterialVS]);
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_constantBuffers[eConstantBufferType_PerFramePS]);
	m_d3dImmediateContext->PSSetConstantBuffers(1, 1, &m_constantBuffers[eConstantBufferType_PerPassPS]);
	m_d3dImmediateContext->PSSetConstantBuffers(2, 1, &m_constantBuffers[eConstantBufferType_PerObjectPS]);
	m_d3dImmediateContext->PSSetConstantBuffers(3, 1, &m_constantBuffers[eConstantBufferType_PerObjectMaterialPS]);
}

//-------------------------------------------------------------------

void RenderManager::_DeleteConstantBuffers()
{
	for (int i = 0; i < eConstantBufferType_Count; i++)
	{
		SAFE_RELEASE(m_constantBuffers[i]);
	}
}

//-------------------------------------------------------------------

void RenderManager::_CreateQuadMeshResources()
{
	HRESULT hr = S_OK;

	// input layout
	const D3D11_INPUT_ELEMENT_DESC quadLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D11InputLayout* quadVertexLayout = NULL;
	hr = g_renderManager->GetDevice()->CreateInputLayout(quadLayout, ARRAYSIZE(quadLayout), g_FullScreenQuadVS.GetBufferPointer(), g_FullScreenQuadVS.GetBufferSize(), &quadVertexLayout);

	// vertex buffer
	float dataVB[] = { 0.f, 0.f,       1.f, 0.f,
		0.f, 1.f,       1.f, 1.f };
	int dataVBSizeInBytes = sizeof(dataVB);
	int dataVBStride = 2 * sizeof(float);

	// index buffer
	WORD dataIB[] = { 0, 1, 2, 1, 3, 2 };
	int indexDataSizeInBytes = sizeof(dataIB);

	m_quadMesh = new Mesh((BYTE*)dataVB, dataVBSizeInBytes, dataVBStride, (BYTE*)dataIB, indexDataSizeInBytes, quadVertexLayout);
}

//-------------------------------------------------------------------

void RenderManager::_DeleteQuadMeshResources()
{
	SAFE_DELETE(m_quadMesh);
}

//-------------------------------------------------------------------
