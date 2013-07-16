#pragma once
#include <time.h>
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"
#include "DXUT\d3dx11effect.h"
#include <vector>
#include <stdlib.h> 
#include <time.h> 


class Particle
{


	ID3D11BlendState*			additive_blend; 
	ID3D11Buffer*               calc_buffer;
	ID3D11Buffer*               render_buffer;
	ID3D11Buffer*               dt_buffer;


	ID3D11RasterizerState*      g_pRasterizerStateSolid;
	ID3D11InputLayout*			g_pVertexLayout11;
	ID3D11Buffer*				g_pVertexBuffer;

	
	ID3D11Buffer*				Stream_target_buffer;
	ID3D11Buffer*				rendering_buffer;
	ID3D11Buffer*				start_buffer;

	ID3D11VertexShader*			g_pVertexShader;
	ID3D11VertexShader*			g_pVertexShaderPass;
	ID3D11GeometryShader*		g_pGeoShader;
	ID3D11GeometryShader*		g_pGeoShaderCalc;
	ID3D11PixelShader*			g_pPixelShader;
	ID3D11SamplerState*			g_pSamLinear;
	ID3D11SamplerState*			g_pSamPoint;

	ID3D11Texture1D*			RandomTexture;

	ID3D11ShaderResourceView*   g_pRandRV;
	ID3D11ShaderResourceView*   g_pTexRV;

	ID3D11DepthStencilState*		g_DepthState;


	D3DXVECTOR4 grav;
	D3DXVECTOR3 position;
	bool first_cycle;
	UINT stride;
public:
	void create_randomtexture(ID3D11Device* pd3dDevice);
	void destroy();
	void set_gravity(D3DXVECTOR4 input);
	void set_pos(D3DXVECTOR3 input);
	Particle(void);
	~Particle(void);
	void generate_particles(int amount);
	void calculate_particle(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, CFirstPersonCamera *g_Camera,CDXUTDirectionWidget *wind);
	void CALLBACK Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera );
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	HRESULT setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext, int _amount ,int type); //0 = fire, 1 = missle
	HRESULT CompileShaderFromFile( WCHAR* szFileName,ID3DBlob** ppBlobOut);
};

