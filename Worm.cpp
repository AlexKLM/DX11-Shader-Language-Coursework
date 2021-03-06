#include "DXUT.h"
#include "Worm.h"

struct CB_PER_OBJECT
{
    D3DXMATRIX mViewProjection;
	D3DXMATRIX m_World;
   	D3DXVECTOR3		mcam_pos;

	float		tess_lvl;
	D3DXVECTOR4 m_vObjectColor;
};
UINT                       wormPerObjectBind = 0;

struct CB_PS_PER_FRAME
{
    D3DXVECTOR4 m_vLightDirAmbient;
};
UINT                        wormPerFrameBind = 1;

struct WormPoints
{
	float m_vPosition[3];
};

enum E_PARTITION_MODE
{
   PARTITION_INTEGER,
   PARTITION_FRACTIONAL_EVEN,
   PARTITION_FRACTIONAL_ODD
};

E_PARTITION_MODE                    WormPartitionMode = PARTITION_INTEGER;


const WormPoints g_Worm[32] = {
	//bottom #1
	{ 0,0,0 },
	{9.9,0,15 },
	{ -3.5,0,35},
	{9.8,0,50 },

	{0,-3,0 },
	{9.9,-3,15 },
	{-3.5,-3,35 },
	{ 9.8,-3,50},

	{ 5,-3,0},
	{14.9,-3,15 },
	{ 1.4,-3,35 },
	{14.8,-3,50 },

	{ 5,0,0},
	{ 14.9,0,15 },
	{1.4,0,35},
	{14.8,0,50 },

	////top #1
	{9.8,0,50},
	{-3.5,0,35},
	{9.9,0,15},
	{0,0,0},

    {9.8,3,50},
	{-3.5,3,35},
	{9.9,3,15},
	{0,3,0},

    {14.8,3,50},
	{1.4,3,35},
	{14.9,3,15},
	{5,3,0},

    {14.8,0,50},
	{1.4,0,35},
	{14.9,0,15},
	{5,0,0},
};

void Worm::destroy()
{
	SAFE_RELEASE( g_pVertexLayout11 );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pPixelShader );
	SAFE_RELEASE( g_pHullShaderInteger );
    SAFE_RELEASE( g_pDomainShader );

    SAFE_RELEASE( g_pSamLinear );
	SAFE_RELEASE( g_pControlPointVB);

	SAFE_RELEASE( g_pRasterizerStateSolid);

    SAFE_RELEASE( g_pcbVSPerObject );
    SAFE_RELEASE( g_pcbPSPerObject );
    SAFE_RELEASE( g_pcbPSPerFrame );
}

Worm::Worm(void)
{
}


Worm::~Worm(void)
{
}

HRESULT Worm::CompileShaderFromFile( WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
	  HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName ) );

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( str, pDefines, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

HRESULT Worm::setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext)
{
	HRESULT hr;

	D3DXVECTOR3 vCenter( 300, 200, -120 );
    FLOAT fObjectRadius = 378.15607f;
	//D3DXMatrixIdentity(&g_mCenterMesh);
	tesslvl = 80;
    D3DXMatrixTranslation( &g_mCenterMesh, -vCenter.x, -vCenter.y, -vCenter.z );
    D3DXMATRIXA16 m;
    D3DXMatrixRotationY( &m, D3DX_PI );
    g_mCenterMesh *= m;
    D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    g_mCenterMesh *= m;

	
	ID3DBlob* pBlobVS = NULL;
    ID3DBlob* pBlobHSInt = NULL;
    ID3DBlob* pBlobHSFracEven = NULL;
    ID3DBlob* pBlobHSFracOdd = NULL;
	ID3DBlob* pBlobDS = NULL;
    ID3DBlob* pBlobPS = NULL;

    D3D_SHADER_MACRO integerPartitioning[] = { { "BEZIER_HS_PARTITION", "\"integer\"" }, { 0 } };
    D3D_SHADER_MACRO fracEvenPartitioning[] = { { "BEZIER_HS_PARTITION", "\"fractional_even\"" }, { 0 } };
    D3D_SHADER_MACRO fracOddPartitioning[] = { { "BEZIER_HS_PARTITION", "\"fractional_odd\"" }, { 0 } };

	V_RETURN( CompileShaderFromFile( L"WormTess.hlsl", NULL, "WormVS", "vs_5_0",  &pBlobVS ) );

	V_RETURN( CompileShaderFromFile( L"WormTess.hlsl", integerPartitioning, "WormHS", "hs_5_0", &pBlobHSInt ) );

	V_RETURN( CompileShaderFromFile( L"WormTess.hlsl", fracEvenPartitioning, "WormHS", "hs_5_0", &pBlobHSFracEven ) );

    V_RETURN( CompileShaderFromFile( L"WormTess.hlsl", fracOddPartitioning, "WormHS", "hs_5_0", &pBlobHSFracOdd ) );

    V_RETURN( CompileShaderFromFile( L"WormTess.hlsl", NULL, "WormDS", "ds_5_0", &pBlobDS ) );
    V_RETURN( CompileShaderFromFile( L"WormTess.hlsl", NULL, "WormPS", "ps_5_0", &pBlobPS ) );



	D3D11_RASTERIZER_DESC RasterDesc;
    ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_NONE;
    RasterDesc.DepthClipEnable = TRUE;
    V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateSolid ) );
    DXUT_SetDebugName( g_pRasterizerStateSolid, "Solid" );

	//	D3D11_RASTERIZER_DESC RasterDesc;
    ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
    RasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    RasterDesc.CullMode = D3D11_CULL_NONE;
    RasterDesc.DepthClipEnable = TRUE;
    V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateWire ) );
    DXUT_SetDebugName( g_pRasterizerStateSolid, "Wire" );


    // Create shaders
    V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "WormVS" );

       V_RETURN( pd3dDevice->CreateHullShader( pBlobHSInt->GetBufferPointer(), pBlobHSInt->GetBufferSize(), NULL, &g_pHullShaderInteger ) );
    DXUT_SetDebugName( g_pHullShaderInteger, "WormHS int" );

	   V_RETURN( pd3dDevice->CreateHullShader( pBlobHSFracEven->GetBufferPointer(), pBlobHSFracEven->GetBufferSize(), NULL, &g_pHullShaderFracEven ) );
    DXUT_SetDebugName( g_pHullShaderFracEven, "WormHS frac even" );

    V_RETURN( pd3dDevice->CreateHullShader( pBlobHSFracOdd->GetBufferPointer(), pBlobHSFracOdd->GetBufferSize(), NULL, &g_pHullShaderFracOdd ) );
    DXUT_SetDebugName( g_pHullShaderFracOdd, "WormHS frac odd" );

    V_RETURN( pd3dDevice->CreateDomainShader( pBlobDS->GetBufferPointer(), pBlobDS->GetBufferSize(), NULL, &g_pDomainShader ) );
    DXUT_SetDebugName( g_pDomainShader, "WormDS" );

    V_RETURN( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "WormPS" );



    // Create our vertex input layout
    const D3D11_INPUT_ELEMENT_DESC spherelayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    V_RETURN( pd3dDevice->CreateInputLayout( spherelayout, ARRAYSIZE( spherelayout ), pBlobVS->GetBufferPointer(),
                                             pBlobVS->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

	SAFE_RELEASE( pBlobVS );
    SAFE_RELEASE( pBlobHSInt );
    SAFE_RELEASE( pBlobDS );
    SAFE_RELEASE( pBlobPS );


	//setup cbuffers
    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;

    Desc.ByteWidth = sizeof( CB_PER_OBJECT );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbVSPerObject ) );
    DXUT_SetDebugName( g_pcbVSPerObject, "CB_VS_PER_OBJECT" );

    Desc.ByteWidth = sizeof( CB_PS_PER_FRAME );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSPerFrame ) );
    DXUT_SetDebugName( g_pcbPSPerFrame, "CB_PS_PER_FRAME" );


	
    D3D11_BUFFER_DESC vbDesc;
    ZeroMemory( &vbDesc, sizeof(D3D11_BUFFER_DESC) );
    vbDesc.ByteWidth = sizeof(WormPoints) * ARRAYSIZE(g_Worm);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbInitData;
    ZeroMemory( &vbInitData, sizeof(vbInitData) );
    vbInitData.pSysMem = g_Worm;
    V_RETURN( pd3dDevice->CreateBuffer( &vbDesc, &vbInitData, &g_pControlPointVB ) );
    DXUT_SetDebugName( g_pControlPointVB, "Control Points" );

	D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamLinear );

	return S_OK;
}

void CALLBACK Worm::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,CDXUTDirectionWidget *g_LightControl )
{
	HRESULT hr;

	D3DXMATRIX mViewProjection;
    D3DXVECTOR3 vLightDir;
    D3DXMATRIX mWorld;
    D3DXMATRIX mView;
    D3DXMATRIX mProj;

    // Get the projection & view matrix from the camera class
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();

    // Get the light direction
    vLightDir = g_LightControl->GetLightDirection();
	pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);
    // Per frame cb update
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pd3dImmediateContext->Map( g_pcbPSPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_PS_PER_FRAME* pPerFrame = ( CB_PS_PER_FRAME* )MappedResource.pData;
    float fAmbient = 0.1f;
    pPerFrame->m_vLightDirAmbient = D3DXVECTOR4( vLightDir.x, vLightDir.y, vLightDir.z, fAmbient );
    pd3dImmediateContext->Unmap( g_pcbPSPerFrame, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( wormPerObjectBind, 1, &g_pcbPSPerFrame );

	
	UINT Stride = sizeof( WormPoints );


    
    // Set the per object constant data
    mWorld = g_mCenterMesh;
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();

    mViewProjection = mWorld* mView * mProj;
        
    // VS Per object
    V( pd3dImmediateContext->Map( g_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_PER_OBJECT* pPerObject = ( CB_PER_OBJECT* )MappedResource.pData;
    D3DXMatrixTranspose( &pPerObject->mViewProjection, &mViewProjection );
	D3DXMatrixTranspose( &pPerObject->m_World, &mWorld );
	pPerObject->mcam_pos = *g_Camera->GetEyePt();
	pPerObject->m_vObjectColor = D3DXVECTOR4( 1, 1, 1, 1 );
	pPerObject->tess_lvl = tesslvl;
    pd3dImmediateContext->Unmap( g_pcbVSPerObject, 0 );

    pd3dImmediateContext->VSSetConstantBuffers( wormPerObjectBind, 1, &g_pcbVSPerObject );
	pd3dImmediateContext->VSSetConstantBuffers( wormPerObjectBind, 1, &g_pcbVSPerObject );
    pd3dImmediateContext->HSSetConstantBuffers( wormPerObjectBind, 1, &g_pcbVSPerObject );
    pd3dImmediateContext->DSSetConstantBuffers( wormPerObjectBind, 1, &g_pcbVSPerObject );

    pd3dImmediateContext->PSSetConstantBuffers( wormPerObjectBind, 1, &g_pcbVSPerObject );

	//pd3dImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );
	//pd3dImmediateContext->HSSetShaderResources( 0, 1, &g_pTextureRV );
	//pd3dImmediateContext->DSSetShaderResources( 0, 1, &g_pTextureRV );
	//pd3dImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );




	pd3dImmediateContext->VSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->HSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->DSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	// Set the shaders
    pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	pd3dImmediateContext->GSSetShader(NULL,NULL,0);
	if (WormPartitionMode == PARTITION_INTEGER)
		pd3dImmediateContext->HSSetShader( g_pHullShaderInteger, NULL, 0 );
	else if (WormPartitionMode == PARTITION_FRACTIONAL_EVEN)
		pd3dImmediateContext->HSSetShader( g_pHullShaderFracEven, NULL, 0 );
	else if (WormPartitionMode == PARTITION_FRACTIONAL_ODD)
		pd3dImmediateContext->HSSetShader( g_pHullShaderFracOdd, NULL, 0 );
	pd3dImmediateContext->DSSetShader( g_pDomainShader, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

    //Render
    SDKMESH_SUBSET* pSubset = NULL;
    D3D11_PRIMITIVE_TOPOLOGY PrimType;

	    UINT Offset = 0;

	if(showwire)
	{
		pd3dImmediateContext->RSSetState( g_pRasterizerStateWire);
	}
	else
	{
		pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	}
	pd3dImmediateContext->IASetInputLayout( g_pVertexLayout11 );
	 pd3dImmediateContext->IASetVertexBuffers( 0, 1, &g_pControlPointVB, &Stride, &Offset );

    //pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST );
	pd3dImmediateContext->Draw( sizeof(g_Worm), 0 );
}

void Worm::toggleWire(bool input)
{
	showwire = input;
}


void Worm::settessmethod(int input)
{
	if(input == 0)
	{
		WormPartitionMode = PARTITION_INTEGER;
	}
	else if(input == 1)
	{
		WormPartitionMode = PARTITION_FRACTIONAL_EVEN;
	}
	else
	{
		WormPartitionMode = PARTITION_FRACTIONAL_ODD;
	}
}

void Worm::set_tesslvl(float input)
{
	tesslvl = input;
}