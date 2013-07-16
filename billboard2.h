#pragma once
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"

class billboard2
{
private:
	//cbuffers
	ID3D11Buffer*               g_pcbVSPerObject;
	ID3D11Buffer*               g_pcbPSPerObject;
	ID3D11Buffer*               g_pcbPSPerFrame;

	ID3D11Buffer*				g_pcbPSraytrace;
	//ID3D11Buffer*				g_pcbPSraytrace;

	D3DXMATRIXA16               g_mCenterMesh;
	ID3D11Buffer*				g_pControlPointVB; 
	ID3D11ShaderResourceView*           g_pTextureRV;

	ID3D11RasterizerState*              g_pRasterizerStateSolid;

	ID3D11InputLayout*  g_pVertexLayout11;
	ID3D11InputLayout*  RayTraceVertexLayout11;


	ID3D11Buffer*		g_pVertexBuffer;
	ID3D11Buffer*       g_pIndexBuffer;

	ID3D11VertexShader*                 g_pVertexShader;
	ID3D11HullShader*                   g_pHullShaderInteger;
	ID3D11DomainShader*                 g_pDomainShader;
	ID3D11PixelShader*                  g_pPixelShader;

	ID3D11VertexShader*                 g_pVertexShaderRay;
	ID3D11PixelShader*                  g_pPixelShaderRay;

	//ID3D11VertexShader*  g_pVertexShader;
	//ID3D11PixelShader*   g_pPixelShader;
	CDXUTSDKMesh                g_Mesh11;
	ID3D11RenderTargetView * rtv_render_to_texture;
	ID3D11Texture2D * reflection_texture;
	ID3D11ShaderResourceView* srv_render_to_texture;
	//ID3D11ShaderResourceView * srv_render_to_texture;
	CModelViewerCamera          billboard_camera;  
	ID3D11SamplerState*  g_pSamLinear;
	float rotation;
	int type;

public:
	billboard2(void);
	~billboard2(void);
	void setup_texture();
	void destroy();
	void change_screen();
	void CALLBACK Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 lightpos );
	void CALLBACK RenderTexture( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext,CDXUTDirectionWidget *g_LightControl );

	HRESULT CompileShaderFromFile( WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	HRESULT setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext,D3DXVECTOR3 pos);
	HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
};

