#pragma once
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"



class testing
{
private:
	//cbuffers
	ID3D11Buffer*               g_pcbVSPerObject;
	ID3D11Buffer*               g_pcbPSPerObject;
	ID3D11Buffer*               g_pcbPSPerFrame;
	D3DXMATRIXA16               g_mCenterMesh;
	CDXUTSDKMesh                g_Mesh11;
	ID3D11RasterizerState*              g_pRasterizerStateSolid;
	ID3D11InputLayout*  g_pVertexLayout11;
	ID3D11Buffer*		g_pVertexBuffer;
	ID3D11Buffer*       g_pIndexBuffer;
	ID3D11VertexShader*  g_pVertexShader;
	ID3D11PixelShader*   g_pPixelShader;
	ID3D11SamplerState*  g_pSamLinear;
public:
	void destroy();
	testing(void);
	~testing(void);
	void CALLBACK Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 lightpos );
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	HRESULT setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext);
};

