#include "DXUT.h"
#include "sphere.h"


struct CB_PER_OBJECT
{
    D3DXMATRIX mViewProjection;
	D3DXMATRIX m_World;
	D3DXMATRIX m_View;
   	D3DXVECTOR3		mcam_pos;

	float		sphere_size;
	D3DXVECTOR4 m_vObjectColor;
};
UINT                        cubePerObjectBind = 0;

struct CB_PS_PER_FRAME
{
    D3DXVECTOR4 m_vLightDirAmbient;
	D3DXVECTOR3		mcam_pos;
	float		mplaceholder;
};
UINT                        cubePerFrameBind = 1;

struct CubePoints
{
	float m_vPosition[3];
};

enum E_PARTITION_MODE
{
   PARTITION_INTEGER,
   PARTITION_FRACTIONAL_EVEN,
   PARTITION_FRACTIONAL_ODD
};

E_PARTITION_MODE                    SpherePartitionMode = PARTITION_INTEGER;


const CubePoints g_Cube[24] ={
	//bottom
	{50.0f,-50.0f,50.0f},
	{50.0f,-50.0f,-50.0f},
	{-50.0f,-50.0f,-50.0f},
	{-50.0f,-50.0f,50.0f},
	//top
	{50.0f,50.0f,50.0f},
	{50.0f,50.0f,-50.0f},
	{-50.0f,50.0f,-50.0f},
	{-50.0f,50.0f,50.0f},
	//"left"
	{-50.0f,50.0f,50.0f},
	{-50.0f,-50.0f,50.0f},
	{-50.0f,-50.0f,-50.0f},
	{-50.0f,50.0f,-50.0f},
	//"front
	{-50.0f,50.0f,-50.0f},
	{50.0f,50.0f,-50.0f},
	{50.0f,-50.0f,-50.0f},
	{-50.0f,-50.0f,-50.0f},
	//right
	{50.0f,-50.0f,-50.0f},
	{50.0f,-50.0f,50.0f},
	{50.0f,50.0f,50.0f},
	{50.0f,50.0f,-50.0f},
	//back
	{50.0f,50.0f,50.0f},
	{50.0f,-50.0f,50.0f},
	{-50.0f,-50.0f,50.0f},
	{-50.0f,50.0f,50.0f},

};

sphere::sphere(void)
{
}


sphere::~sphere(void)
{
}


void sphere::destroy()
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
	SAFE_RELEASE(g_pTextureRV);

	SAFE_RELEASE( g_pRasterizerStateSolid);

    SAFE_RELEASE( g_pcbVSPerObject );
    SAFE_RELEASE( g_pcbPSPerObject );
    SAFE_RELEASE( g_pcbPSPerFrame );
}

HRESULT sphere::CompileShaderFromFile( WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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

HRESULT sphere::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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
    hr = D3DX11CompileFromFile( str, NULL, NULL, szEntryPoint, szShaderModel, 
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

HRESULT sphere::setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext,float _size,D3DXVECTOR3 pos)
{
	HRESULT hr;
	size = _size;
	tesslvl = 80;
	position = pos;
	showwire = false;
    FLOAT fObjectRadius = 378.15607f;
	//D3DXMatrixIdentity(&g_mCenterMesh);

     D3DXMatrixTranslation( &g_mCenterMesh, position.x, position.y, position.z );
    //D3DXMATRIXA16 m;
    //D3DXMatrixRotationY( &m, D3DX_PI );
    //g_mCenterMesh *= m;
    //D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    //g_mCenterMesh *= m;

	ID3DBlob* pBlobVS = NULL;
    ID3DBlob* pBlobHSInt = NULL;
    ID3DBlob* pBlobHSFracEven = NULL;
    ID3DBlob* pBlobHSFracOdd = NULL;
	ID3DBlob* pBlobDS = NULL;
    ID3DBlob* pBlobPS = NULL;

	D3D_SHADER_MACRO integerPartitioning[] = { { "BEZIER_HS_PARTITION", "\"integer\"" }, { 0 } };
    D3D_SHADER_MACRO fracEvenPartitioning[] = { { "BEZIER_HS_PARTITION", "\"fractional_even\"" }, { 0 } };
    D3D_SHADER_MACRO fracOddPartitioning[] = { { "BEZIER_HS_PARTITION", "\"fractional_odd\"" }, { 0 } };

	V_RETURN( CompileShaderFromFile( L"Cube2Sphere.hlsl", NULL, "QuadTess_VS", "vs_5_0",  &pBlobVS ) );
	
    V_RETURN( CompileShaderFromFile( L"Cube2Sphere.hlsl", integerPartitioning, "QuadTess_HS", "hs_5_0", &pBlobHSInt ) );

	V_RETURN( CompileShaderFromFile( L"Cube2Sphere.hlsl", fracEvenPartitioning, "QuadTess_HS", "hs_5_0", &pBlobHSFracEven ) );

    V_RETURN( CompileShaderFromFile( L"Cube2Sphere.hlsl", fracOddPartitioning, "QuadTess_HS", "hs_5_0", &pBlobHSFracOdd ) );

    V_RETURN( CompileShaderFromFile( L"Cube2Sphere.hlsl", NULL, "QuadTess_DS", "ds_5_0", &pBlobDS ) );
    V_RETURN( CompileShaderFromFile( L"Cube2Sphere.hlsl", NULL, "QuadTess_PS", "ps_5_0", &pBlobPS ) );


	hr = D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"height.png", NULL, NULL, &g_pTextureRV, NULL );


	D3D11_RASTERIZER_DESC RasterDesc;
    ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_NONE;
    RasterDesc.DepthClipEnable = TRUE;
    V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateSolid ) );
    DXUT_SetDebugName( g_pRasterizerStateSolid, "Solid" );

	 ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
    RasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    RasterDesc.CullMode = D3D11_CULL_NONE;
    RasterDesc.DepthClipEnable = TRUE;
    V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateWire ) );
    DXUT_SetDebugName( g_pRasterizerStateWire, "Wire" );


    // Create shaders
    V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "QuadTess_VS" );

    V_RETURN( pd3dDevice->CreateHullShader( pBlobHSInt->GetBufferPointer(), pBlobHSInt->GetBufferSize(), NULL, &g_pHullShaderInteger ) );
    DXUT_SetDebugName( g_pHullShaderInteger, "QuadTess_HS int" );

    V_RETURN( pd3dDevice->CreateHullShader( pBlobHSFracEven->GetBufferPointer(), pBlobHSFracEven->GetBufferSize(), NULL, &g_pHullShaderFracEven ) );
    DXUT_SetDebugName( g_pHullShaderFracEven, "QuadTess_HS frac even" );

    V_RETURN( pd3dDevice->CreateHullShader( pBlobHSFracOdd->GetBufferPointer(), pBlobHSFracOdd->GetBufferSize(), NULL, &g_pHullShaderFracOdd ) );
    DXUT_SetDebugName( g_pHullShaderFracOdd, "QuadTess_HS frac odd" );
    V_RETURN( pd3dDevice->CreateDomainShader( pBlobDS->GetBufferPointer(), pBlobDS->GetBufferSize(), NULL, &g_pDomainShader ) );
    DXUT_SetDebugName( g_pDomainShader, "QuadTess_DS" );

    V_RETURN( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "QuadTess_PS" );



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
    vbDesc.ByteWidth = sizeof(CubePoints) * ARRAYSIZE(g_Cube);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbInitData;
    ZeroMemory( &vbInitData, sizeof(vbInitData) );
    vbInitData.pSysMem = g_Cube;
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

void sphere::move(D3DXVECTOR3 movement)
{
	//position += movement;
	D3DXMatrixTranslation( &g_mCenterMesh, movement.x, movement.y, movement.z );
}

void CALLBACK sphere::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 lightpos )
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
    vLightDir = lightpos;
    // Per frame cb update
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pd3dImmediateContext->Map( g_pcbPSPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_PS_PER_FRAME* pPerFrame = ( CB_PS_PER_FRAME* )MappedResource.pData;
    float fAmbient = 0.1f;
    pPerFrame->m_vLightDirAmbient = D3DXVECTOR4( lightpos.x, lightpos.y, lightpos.z, fAmbient );
	pPerFrame->mcam_pos = *g_Camera->GetEyePt();
    pd3dImmediateContext->Unmap( g_pcbPSPerFrame, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( cubePerFrameBind, 1, &g_pcbPSPerFrame );
	
	UINT Stride = sizeof( CubePoints );
   
    // Set the per object constant data
    mWorld = g_mCenterMesh;
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();

    mViewProjection = mView * mProj;
        
    // VS Per object
    V( pd3dImmediateContext->Map( g_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_PER_OBJECT* pPerObject = ( CB_PER_OBJECT* )MappedResource.pData;
    D3DXMatrixTranspose( &pPerObject->mViewProjection, &mViewProjection );
	D3DXMatrixTranspose( &pPerObject->m_World, &mWorld );
	pPerObject->mcam_pos = *g_Camera->GetEyePt();
	pPerObject->m_View = mView;
	pPerObject->sphere_size = size;
	pPerObject->m_vObjectColor = D3DXVECTOR4( tesslvl, 1, 1, 1 );
    pd3dImmediateContext->Unmap( g_pcbVSPerObject, 0 );

    pd3dImmediateContext->VSSetConstantBuffers( cubePerObjectBind, 1, &g_pcbVSPerObject );
	pd3dImmediateContext->VSSetConstantBuffers( cubePerObjectBind, 1, &g_pcbVSPerObject );
    pd3dImmediateContext->HSSetConstantBuffers( cubePerObjectBind, 1, &g_pcbVSPerObject );
    pd3dImmediateContext->DSSetConstantBuffers( cubePerObjectBind, 1, &g_pcbVSPerObject );

    //pd3dImmediateContext->PSSetConstantBuffers( cubePerObjectBind, 1, &g_pcbVSPerObject );

	//pd3dImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );
	//pd3dImmediateContext->HSSetShaderResources( 0, 1, &g_pTextureRV );
	pd3dImmediateContext->DSSetShaderResources( 0, 1, &g_pTextureRV );
	pd3dImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );
	pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);



	pd3dImmediateContext->VSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->HSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->DSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	// Set the shaders
    pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	pd3dImmediateContext->GSSetShader(NULL,NULL,0);
	if (SpherePartitionMode == PARTITION_INTEGER)
		pd3dImmediateContext->HSSetShader( g_pHullShaderInteger, NULL, 0 );
	else if (SpherePartitionMode == PARTITION_FRACTIONAL_EVEN)
		pd3dImmediateContext->HSSetShader( g_pHullShaderFracEven, NULL, 0 );
	else if (SpherePartitionMode == PARTITION_FRACTIONAL_ODD)
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
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST );
	pd3dImmediateContext->Draw( sizeof(g_Cube), 0 );
}

void sphere::settessmethod(int input)
{
	if(input == 0)
	{
		SpherePartitionMode = PARTITION_INTEGER;
	}
	else if(input == 1)
	{
		SpherePartitionMode = PARTITION_FRACTIONAL_EVEN;
	}
	else
	{
		SpherePartitionMode = PARTITION_FRACTIONAL_ODD;
	}
}

void sphere::toggleWire(bool input)
{
	showwire = input;
}


void sphere::set_tesslvl(float input)
{
	tesslvl = input;
}