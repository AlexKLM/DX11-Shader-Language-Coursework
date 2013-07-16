#include "DXUT.h"
#include "building.h"
#include <stdlib.h> 
#include <time.h> 

struct VS_CBuffer
{
	D3DXMATRIX mWorldViewProjection;
	D3DXMATRIX mWorld;
};

struct PS_CBuffer
{
	D3DXVECTOR4 m_lightpos;
	D3DXVECTOR3		mcam_pos;
	float		placeholder; 
};
struct instance_data
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 height;//only y should be used
};

struct building_vertex
{
	float m_vPosition[3];
	float uv[2];
	float Normal[3];

};

const building_vertex g_Building[24] ={
	//front
	{-10, -10, -10, 0.0f, 1.0f,-1.0f, -1.0f, -1.0f},
	{-10,  10, -10, 0.0f, 0.0f,-1.0f,  1.0f, -1.0f},
	{ 10,  10, -10, 1.0f, 0.0f, 1.0f,  1.0f, -1.0f},
	{ 10, -10, -10, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f},

	// Back Face
	{-10, -10, 10, 1.0f, 1.0f,-1.0f, -1.0f, 1.0f},
	{ 10, -10, 10, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f},
	{ 10,  10, 10, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
	{-10,  10, 10, 1.0f, 0.0f,-1.0f,  1.0f, 1.0f},

	// Top Face
	{-10, 10, -10, 0.0f, 1.0f,-1.0f, 1.0f, -1.0f},
	{-10, 10,  10, 0.0f, 0.0f,-1.0f, 1.0f,  1.0f},
	{ 10, 10,  10, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f},
	{ 10, 10, -10, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f},

	// Bottom Face
	{-10, -10, -10, 1.0f, 1.0f,-1.0f, -1.0f, -1.0f},
	{10, -10, -10, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f},
	{ 10, -10,  10, 0.0f, 0.0f, 1.0f, -1.0f,  1.0f},
	{-10, -10,  10, 1.0f, 0.0f,-1.0f, -1.0f,  1.0f},

	// Left Face
	{-10, -10,  10, 0.0f, 1.0f,-1.0f, -1.0f,  1.0f},
	{-10,  10,  10, 0.0f, 0.0f,-1.0f,  1.0f,  1.0f},
	{-10,  10, -10, 1.0f, 0.0f,-1.0f,  1.0f, -1.0f},
	{-10, -10, -10, 1.0f, 1.0f,-1.0f, -1.0f, -1.0f},

	// Right Face
	{ 10, -10, -10, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f},
	{ 10,  10, -10, 0.0f, 0.0f, 1.0f,  1.0f, -1.0f},
	{ 10,  10,  10, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f},
	{ 10, -10,  10, 1.0f, 1.0f, 1.0f, -1.0f,  1.0f},
};

DWORD building_indices[] = {
	// Top Face
	8,  9, 10,
	8, 10, 11,

	// Front Face
	0,  1,  2,
	0,  2,  3,

	// Back Face
	4,  5,  6,
	4,  6,  7,



	// Bottom Face
	12, 13, 14,
	12, 14, 15,

	// Left Face
	16, 17, 18,
	16, 18, 19,

	// Right Face
	20, 21, 22,
	20, 22, 23
};




building::building(void)
{
}


building::~building(void)
{
}


std::vector<instance_data> building_data;

HRESULT building::CompileShaderFromFile( WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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

HRESULT building::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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

void building::create_buildings()
{
	instance_data building;
	srand (time(NULL));
	int xmod = -900;
	int zmod = -900;
	int height = 1;
	int valuez = 0;
	int valuex = 0;
	int building_type = 0;
	for(int x = -900; x < 1000; x+= 50)
	{
		for(int z = -900; z < 1000; z+=50)
		{
			valuex = rand() % 30-10;
			
			height = rand() % 10 + 5;
			building_type = rand() %4;
			building.pos = D3DXVECTOR3(xmod+ valuex,0,zmod);
			building.height = D3DXVECTOR3(0,height,building_type);

			building_data.push_back(building);
			valuez = rand()% 50 + 20;
			zmod+=valuez;
			num_of_building++;
		}
		valuex = rand() % 50+ 20;
		xmod+=valuex;
		zmod=-900;
	}
	
	
}

HRESULT building::setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext)
{
	D3DXMatrixTranslation( &g_mCenterMesh, 0.0f,0.0f,0.0f ); //set first builind on the end of the plane
	HRESULT hr;
	num_of_building = 0;
	ID3DBlob* pBlobVS = NULL;
	ID3DBlob* pBlobPS = NULL;

	create_buildings();
	V_RETURN( CompileShaderFromFile( L"buildings.hlsl", NULL, "VS", "vs_5_0",  &pBlobVS ) );
	V_RETURN( CompileShaderFromFile( L"buildings.hlsl", NULL, "PS", "ps_5_0", &pBlobPS ) );

	    V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "buildings_VS" );


    V_RETURN( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "buildings_PS" );

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
		{ "NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		// Instance elements

		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTANCEHEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1}

	};
	UINT size = ARRAYSIZE(layout);

	   V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pBlobVS->GetBufferPointer(),
                                             pBlobVS->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_top1.jpg", NULL, NULL, &g_tex_top1, NULL );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_top2.jpg", NULL, NULL, &g_tex_top2, NULL );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_top3.jpg", NULL, NULL, &g_tex_top3, NULL );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_top4.jpg", NULL, NULL, &g_tex_top4, NULL );

	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_side.jpg", NULL, NULL, &g_tex_side1, NULL );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_side2.png", NULL, NULL, &g_tex_side2, NULL );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_side3.jpg", NULL, NULL, &g_tex_side3, NULL );
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"building_side4.jpg", NULL, NULL, &g_tex_side4, NULL );
	D3D11_BUFFER_DESC vbDesc;
    ZeroMemory( &vbDesc, sizeof(D3D11_BUFFER_DESC) );
    vbDesc.ByteWidth = sizeof(building_vertex) * ARRAYSIZE(g_Building);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbInitData;
    ZeroMemory( &vbInitData, sizeof(vbInitData) );
    vbInitData.pSysMem = g_Building;
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

	iinitData.pSysMem = building_indices;
	V_RETURN(pd3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, &g_pIndexBuffer));


	D3D11_BUFFER_DESC instBuffDesc;	
	ZeroMemory( &instBuffDesc, sizeof(instBuffDesc) );

	instBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	instBuffDesc.ByteWidth = sizeof( instance_data ) * num_of_building;
	instBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instBuffDesc.CPUAccessFlags = 0;
	instBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA instData;
	ZeroMemory( &instData, sizeof(instData) );

	instData.pSysMem = &building_data[0];
	hr = pd3dDevice->CreateBuffer( &instBuffDesc, &instData, &g_pInstanceBuffer);


	D3D11_RASTERIZER_DESC RasterDesc;
	ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
	RasterDesc.FillMode = D3D11_FILL_SOLID;
	RasterDesc.CullMode = D3D11_CULL_NONE;
	RasterDesc.DepthClipEnable = TRUE;
	V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateSolid ) );
	DXUT_SetDebugName( g_pRasterizerStateSolid, "Solid" );


	D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MaxAnisotropy = 16;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = 1;
    hr = pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamLinear );

	 D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;

    Desc.ByteWidth = sizeof( VS_CBuffer );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &VS_cbuff ) );
    DXUT_SetDebugName( VS_cbuff, "CB_VS_PER_OBJECT" );

    Desc.ByteWidth = sizeof( PS_CBuffer );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &PS_cbuff ) );
    DXUT_SetDebugName( PS_cbuff, "CB_PS_PER_FRAME" );

	

}


void CALLBACK building::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 lightpos )
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
    V( pd3dImmediateContext->Map( PS_cbuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    PS_CBuffer* pPerFrame = ( PS_CBuffer* )MappedResource.pData;
    float fAmbient = 0.1f;
    pPerFrame->m_lightpos = D3DXVECTOR4( vLightDir.x, vLightDir.y, vLightDir.z, fAmbient );
	pPerFrame->mcam_pos = *g_Camera->GetEyePt();
    pd3dImmediateContext->Unmap( PS_cbuff, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( 1, 1, &PS_cbuff );

	
	//UINT Stride = sizeof( Points );


    
    // Set the per object constant data
    mWorld = g_mCenterMesh;
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();

    mViewProjection = mWorld * mView * mProj;
        
    // VS Per object
    V( pd3dImmediateContext->Map( VS_cbuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    VS_CBuffer* pPerObject = ( VS_CBuffer* )MappedResource.pData;
    D3DXMatrixTranspose( &pPerObject->mWorldViewProjection, &mViewProjection );
	D3DXMatrixTranspose( &pPerObject->mWorld, &mWorld );
    pd3dImmediateContext->Unmap( VS_cbuff, 0 );

	pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &VS_cbuff );
	pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);

	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	pd3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	pd3dImmediateContext->GSSetShader(NULL,NULL,0);
	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );

	pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

	SDKMESH_SUBSET* pSubset = NULL;
    D3D11_PRIMITIVE_TOPOLOGY PrimType;

	 //UINT Offset = 0;
	// UINT Stride = sizeof( building_vertex );

	 UINT strides[2] = {sizeof( building_vertex ), sizeof( instance_data )};
	UINT offsets[2] = {0, 0};

	// Store the vertex and instance buffers into an array
	// The leaves will use the same instance buffer as the trees, because we need each leaf
	// to go to a certain tree
	ID3D11Buffer* vertInstBuffers[2] = {g_pControlPointVB, g_pInstanceBuffer};
	pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	pd3dImmediateContext->IASetInputLayout( g_pVertexLayout11 );
	// pd3dImmediateContext->IASetVertexBuffers( 0, 1, &g_pControlPointVB, &Stride, &Offset );
	pd3dImmediateContext->IASetVertexBuffers( 0, 2, vertInstBuffers, strides, offsets );
	 pd3dImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	 pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	 pd3dImmediateContext->PSSetShaderResources(0,1,&g_tex_top1);
	 pd3dImmediateContext->PSSetShaderResources(1,1,&g_tex_top2);
	 pd3dImmediateContext->PSSetShaderResources(2,1,&g_tex_top3);
	 pd3dImmediateContext->PSSetShaderResources(3,1,&g_tex_top4);
	 pd3dImmediateContext->DrawIndexedInstanced( 6, num_of_building, 0,0, 0 );


	 pd3dImmediateContext->PSSetShaderResources(0,1,&g_tex_side1);
	 pd3dImmediateContext->PSSetShaderResources(1,1,&g_tex_side2);
	 pd3dImmediateContext->PSSetShaderResources(2,1,&g_tex_side3);
	 pd3dImmediateContext->PSSetShaderResources(3,1,&g_tex_side4);
	 pd3dImmediateContext->DrawIndexedInstanced( 30, num_of_building, 6,0, 0 );

    //pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );
	
	//pd3dImmediateContext->DrawIndexed( 36, 0, 0);
	
	//pd3dImmediateContext->Draw( sizeof(g_Cube), 0 );
	pd3dImmediateContext->OMSetDepthStencilState(NULL, 0);
}