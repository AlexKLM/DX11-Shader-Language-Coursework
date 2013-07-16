#pragma once
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"
#include <vector>

class building
{
private:
	int num_of_building;
	ID3D11Buffer*               VS_cbuff;
	ID3D11Buffer*               PS_cbuff;

	ID3D11SamplerState*  g_pSamLinear;
	ID3D11ShaderResourceView*   g_tex_top1;
	ID3D11ShaderResourceView*   g_tex_side1;


	ID3D11ShaderResourceView*   g_tex_top2;
	ID3D11ShaderResourceView*   g_tex_side2;

	ID3D11ShaderResourceView*   g_tex_top3;
	ID3D11ShaderResourceView*   g_tex_side3;

	ID3D11ShaderResourceView*   g_tex_top4;
	ID3D11ShaderResourceView*   g_tex_side4;

	ID3D11Buffer*				g_pControlPointVB; 
	ID3D11InputLayout*			g_pVertexLayout11;
	ID3D11Buffer*				g_pIndexBuffer;
	ID3D11Buffer*				g_pInstanceBuffer;

	ID3D11RasterizerState*      g_pRasterizerStateSolid;

	D3DXMATRIXA16               g_mCenterMesh;

	ID3D11VertexShader*                 g_pVertexShader;
	ID3D11PixelShader*                  g_pPixelShader;


public:
	building(void);
	~building(void);
	void create_buildings();
	HRESULT CompileShaderFromFile( WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

	HRESULT setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext);
	void CALLBACK Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 lightpos );
};

