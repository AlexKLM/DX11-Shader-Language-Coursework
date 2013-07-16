#pragma once
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"


class spiked_alien
{
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



	ID3D11InputLayout*  part_in_layout;

	ID3D11VertexShader*  g_pVertexShader;
	ID3D11GeometryShader* g_pGeoShader;
	ID3D11GeometryShader* g_pGeoexplode;
	ID3D11PixelShader*   g_pPixelShader;
	ID3D11SamplerState*  g_pSamLinear;

	ID3D11Buffer*       g_pStreamBuffer;
	ID3D11Buffer*       g_pRenderBuffer;

	ID3D11VertexShader*  VS_part_pass;
	ID3D11VertexShader*  VS_part_calc;
	ID3D11GeometryShader* GS_part_calc;
	ID3D11GeometryShader* GS_part_rend;
	ID3D11GeometryShader* GS_part_firstpass;
	ID3D11PixelShader*   PS_part_rend;

	
	ID3D11BlendState*			additive_blend; 

	ID3D11DepthStencilState*		g_DepthState;
	ID3D11ShaderResourceView*		g_pTexRV;

	UINT stride;
	bool first_pass;
	bool exploding;
public:
	void destroy();
	void start_explode();
	spiked_alien(void);
	~spiked_alien(void);
	void CALLBACK Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 light_pos);
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	HRESULT setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext);
};

