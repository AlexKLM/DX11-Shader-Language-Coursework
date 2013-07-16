#pragma once
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"

class test{
private:
	ID3D11InputLayout*  g_pVertexLayout11;
	ID3D11Buffer*		g_pVertexBuffer;
	ID3D11Buffer*       g_pIndexBuffer;
	ID3D11VertexShader*  g_pVertexShader;
	ID3D11PixelShader*   g_pPixelShader;
	ID3D11SamplerState*  g_pSamLinear;

public:
	test(void);
	~test(void);
	void setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext);
};

