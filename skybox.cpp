#include "DXUT.h"
#include "skybox.h"

struct Cube
{
	float m_vPosition[3];
};

struct skybox_cbuffer
{
	D3DXMATRIX mViewProjection;
	D3DXVECTOR3 cam_pos;
	float placeholder;
};

const Cube g_Box[8] ={
	{-50,-50,-50},
	{-50,50,-50},
	{50,50,-50},
	{50,-50,-50},
	{-50,-50,50},
	{-50,50,50},
	{50,50,50},
	{50,-50,50},
};

DWORD indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3, 
		4, 3, 7
	};


skybox::skybox(void)
{
}


skybox::~skybox(void)
{
}


HRESULT skybox::CompileShaderFromFile( WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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

HRESULT skybox::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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

HRESULT skybox::setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext,D3DXVECTOR3 pos)
{
	HRESULT hr;
    FLOAT fObjectRadius = 378.15607f;
	//D3DXMatrixIdentity(&g_mCenterMesh);

     D3DXMatrixTranslation( &g_mCenterMesh, pos.x, pos.y, pos.z );
    //D3DXMATRIXA16 m;
    //D3DXMatrixRotationY( &m, D3DX_PI );
    //g_mCenterMesh *= m;
    //D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    //g_mCenterMesh *= m;

	ID3DBlob* pBlobVS = NULL;
    ID3DBlob* pBlobPS = NULL;


	V_RETURN( CompileShaderFromFile( L"skybox.hlsl", NULL, "VS", "vs_5_0",  &pBlobVS ) );
    V_RETURN( CompileShaderFromFile( L"skybox.hlsl", NULL, "PS", "ps_5_0", &pBlobPS ) );


	D3D11_RASTERIZER_DESC RasterDesc;
    ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_NONE;
    RasterDesc.DepthClipEnable = TRUE;
    V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateSolid ) );
    DXUT_SetDebugName( g_pRasterizerStateSolid, "Solid" );


    // Create shaders
    V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "SkyBox_VS" );


    V_RETURN( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "SkyBox_PS" );

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	pd3dDevice->CreateDepthStencilState(&dssDesc, &g_DepthState);

    // Create our vertex input layout
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pBlobVS->GetBufferPointer(),
                                             pBlobVS->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

	SAFE_RELEASE( pBlobVS );
    SAFE_RELEASE( pBlobPS );


	//setup cbuffers
    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;

    Desc.ByteWidth = sizeof( skybox_cbuffer );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbVSPerObject ) );
    DXUT_SetDebugName( g_pcbVSPerObject, "skybox_cbuffer" );
	
    D3D11_BUFFER_DESC vbDesc;
    ZeroMemory( &vbDesc, sizeof(D3D11_BUFFER_DESC) );
    vbDesc.ByteWidth = sizeof(Cube) * ARRAYSIZE(g_Box);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbInitData;
    ZeroMemory( &vbInitData, sizeof(vbInitData) );
    vbInitData.pSysMem = g_Box;
    V_RETURN( pd3dDevice->CreateBuffer( &vbDesc, &vbInitData, &g_pControlPointVB ) );
    DXUT_SetDebugName( g_pControlPointVB, "Control Points" );

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;
	V_RETURN(pd3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, &g_pIndexBuffer));

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

	
	D3DX11_IMAGE_LOAD_INFO loadSMInfo;
	loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* SMTexture = 0;
	hr = D3DX11CreateTextureFromFile(pd3dDevice, L"cubemap_night.dds", 
		&loadSMInfo, 0, (ID3D11Resource**)&SMTexture, 0);

	D3D11_TEXTURE2D_DESC SMTextureDesc;
	SMTexture->GetDesc(&SMTextureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = SMTextureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = pd3dDevice->CreateShaderResourceView(SMTexture, &SMViewDesc, &g_pTextureRV);

	return S_OK;
}



void CALLBACK skybox::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 lightpos )
{
		HRESULT hr;
	D3DXMATRIX Scale;
	D3DXMATRIX Translation;
	D3DXVECTOR3 eyepoint = *g_Camera->GetEyePt();
	D3DXMatrixIdentity(&g_mCenterMesh);
	D3DXMatrixScaling( &Scale,5.0f, 5.0f, 5.0f );
	D3DXMatrixTranslation( &Translation, eyepoint.x, eyepoint.y, eyepoint.z );
	g_mCenterMesh = Scale * Translation;
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
  
	UINT Stride = sizeof( Cube );
   
    // Set the per object constant data
    mWorld = g_mCenterMesh;
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();
	pd3dImmediateContext->OMSetDepthStencilState(g_DepthState, 0);
    mViewProjection = mWorld* mView * mProj;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    // VS Per object
    V( pd3dImmediateContext->Map( g_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    skybox_cbuffer* pPerObject = ( skybox_cbuffer* )MappedResource.pData;
    D3DXMatrixTranspose( &pPerObject->mViewProjection, &mViewProjection );
	pPerObject->cam_pos = eyepoint;
    pd3dImmediateContext->Unmap( g_pcbVSPerObject, 0 );

    pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &g_pcbVSPerObject );
	//pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &g_pcbVSPerObject );

	pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);


	pd3dImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );

    pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	// Set the shaders
    pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	pd3dImmediateContext->GSSetShader(NULL,NULL,0);
	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );

    //Render
    SDKMESH_SUBSET* pSubset = NULL;
    D3D11_PRIMITIVE_TOPOLOGY PrimType;

	    UINT Offset = 0;

	pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	pd3dImmediateContext->IASetInputLayout( g_pVertexLayout11 );
	 pd3dImmediateContext->IASetVertexBuffers( 0, 1, &g_pControlPointVB, &Stride, &Offset );
	 pd3dImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


    //pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	pd3dImmediateContext->DrawIndexed( 36, 0, 0);
	//pd3dImmediateContext->Draw( sizeof(g_Cube), 0 );
	pd3dImmediateContext->OMSetDepthStencilState(NULL, 0);
}