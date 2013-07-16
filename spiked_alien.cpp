#include "DXUT.h"
#include "spiked_alien.h"



static const int MAX_PARTICLES =  30000;

struct CB_PER_OBJECT
{
    D3DXMATRIX m_World;
    D3DXMATRIX m_View;
	D3DXMATRIX m_Projection;
	D3DXVECTOR4 m_time;
};
UINT                        geoalien_VSPerObjectBind = 0;

struct CB_PS_PER_FRAME
{
    D3DXVECTOR4 m_vLightDirAmbient;
	D3DXVECTOR4 m_Eye;
};
UINT                       geo_alienPSPerFrameBind = 1;

struct VS_explode
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 vel;
	float timer;
};

spiked_alien::spiked_alien(void)
{
}


spiked_alien::~spiked_alien(void)
{
}


void spiked_alien::destroy()
{
	g_Mesh11.Destroy();
                
    SAFE_RELEASE( g_pVertexLayout11 );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );
    SAFE_RELEASE( g_pVertexShader );
	SAFE_RELEASE(g_pGeoShader);
    SAFE_RELEASE( g_pPixelShader );
	SAFE_RELEASE( g_pSamLinear );
	SAFE_RELEASE(g_pRasterizerStateSolid);

    SAFE_RELEASE( g_pcbVSPerObject );
    SAFE_RELEASE( g_pcbPSPerObject );
    SAFE_RELEASE( g_pcbPSPerFrame );
}

HRESULT spiked_alien::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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

HRESULT spiked_alien::setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext)
{
	HRESULT hr;
	exploding = false;
	first_pass = false;
	D3DXVECTOR3 vCenter( 500, 100,100 );
	//D3DXVECTOR3 vCenter( 1000, 200,200 );
	D3DXVECTOR3 origin(0,0,0);
    FLOAT fObjectRadius = 378.15607f;
	 D3DXMATRIXA16 m;

    D3DXMatrixTranslation( &g_mCenterMesh, origin.x, origin.y, origin.z );

   
	D3DXMatrixRotationZ( &m, D3DX_PI/2.0f);
	g_mCenterMesh *= m;
	D3DXMatrixRotationY( &m, D3DX_PI );
    g_mCenterMesh *= m;

	
	
	D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
	
    g_mCenterMesh *= m;

	D3DXMatrixTransformation(&m,NULL,NULL,NULL,NULL,NULL,&vCenter);
	  g_mCenterMesh *= m;
    //D3DXMatrixTranslation( &g_mCenterMesh, vCenter.x,vCenter.y, vCenter.z );

	//D3DXMatrixTranslation( &g_mCenterMesh, 50, 50, 0 );

	ID3DBlob* pVertexShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"geo_alien.hlsl", "VS", "vs_4_0_level_9_1", &pVertexShaderBuffer ) );

	ID3DBlob* pGeometryShaderBuffer = NULL;
	V_RETURN(CompileShaderFromFile(L"geo_alien.hlsl","GS_AKLM","gs_4_0",&pGeometryShaderBuffer));

    ID3DBlob* pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"geo_alien.hlsl", "PS", "ps_4_0_level_9_1", &pPixelShaderBuffer ) );

    // Create the shaders
    V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "VSMain" );

	V_RETURN( pd3dDevice->CreateGeometryShader( pGeometryShaderBuffer->GetBufferPointer(),
                                              pGeometryShaderBuffer->GetBufferSize(), NULL, &g_pGeoShader ) );
    DXUT_SetDebugName( g_pGeoShader, "GeoSpike" );

    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "PSMain" );

	pGeometryShaderBuffer = NULL;
	V_RETURN(CompileShaderFromFile(L"geo_alien.hlsl","GS2Quad_AKLM","gs_4_0",&pGeometryShaderBuffer));
	
	V_RETURN( pd3dDevice->CreateGeometryShader( pGeometryShaderBuffer->GetBufferPointer(),
                                              pGeometryShaderBuffer->GetBufferSize(), NULL, &g_pGeoexplode ) );
    DXUT_SetDebugName( g_pGeoexplode, "GeoExplode" );

	
	D3D11_RASTERIZER_DESC RasterDesc;
    ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_NONE;
    RasterDesc.DepthClipEnable = TRUE;
    V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateSolid ) );
    DXUT_SetDebugName( g_pRasterizerStateSolid, "Solid" );

    // Create our vertex input layout
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVertexShaderBuffer->GetBufferPointer(),
                                             pVertexShaderBuffer->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

	pVertexShaderBuffer = NULL;
	V_RETURN( CompileShaderFromFile( L"geo_alien.hlsl", "VS_explode", "vs_4_0", &pVertexShaderBuffer ) );
	    V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &VS_part_calc ) );
    DXUT_SetDebugName( VS_part_calc, "VS_part_calc" );

	pVertexShaderBuffer = NULL;
	V_RETURN( CompileShaderFromFile( L"geo_alien.hlsl", "VS_pass", "vs_4_0", &pVertexShaderBuffer ) );
	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &VS_part_pass ) );
    DXUT_SetDebugName( VS_part_pass, "VS_Part_pass" );

	const D3D11_INPUT_ELEMENT_DESC layout_part[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TIMER", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT size = ARRAYSIZE(layout_part);
	stride = sizeof(VS_explode);
	 V_RETURN( pd3dDevice->CreateInputLayout( layout_part, size, pVertexShaderBuffer->GetBufferPointer(),
                                             pVertexShaderBuffer->GetBufferSize(), &part_in_layout ) );
    DXUT_SetDebugName( part_in_layout, "explode_layout" );

		D3D11_SO_DECLARATION_ENTRY pDecl[] =
	{
		// semantic name, semantic index, start component, component count, output slot
		{ 0,"POSITION", 0, 0, 3, 0 }, 
		{ 0,"NORMAL", 0, 0, 3, 0 },    
		{ 0,"TIMER", 0, 0, 1, 0 },     
	};
	UINT strides[1] = { ((7*sizeof(float)))}; 
	strides[0] = sizeof(float)*7;
	UINT SOstride = strides[0];

	pGeometryShaderBuffer = NULL;
	V_RETURN(CompileShaderFromFile(L"geo_alien.hlsl","GS_EXPLODE_FIRSTPASS","gs_4_0",&pGeometryShaderBuffer));

	V_RETURN(pd3dDevice->CreateGeometryShaderWithStreamOutput(pGeometryShaderBuffer->GetBufferPointer(),pGeometryShaderBuffer->GetBufferSize(),pDecl,3,strides,1,0 ,NULL,&GS_part_firstpass));

	pGeometryShaderBuffer = NULL;
	V_RETURN(CompileShaderFromFile(L"geo_alien.hlsl","GS_EXPLODE_Calc","gs_4_0",&pGeometryShaderBuffer));
	V_RETURN(pd3dDevice->CreateGeometryShaderWithStreamOutput(pGeometryShaderBuffer->GetBufferPointer(),pGeometryShaderBuffer->GetBufferSize(),pDecl,3,strides,1,0 ,NULL,&GS_part_calc));

	pGeometryShaderBuffer = NULL;
	V_RETURN(CompileShaderFromFile(L"geo_alien.hlsl","GS_Render","gs_4_0",&pGeometryShaderBuffer));
	V_RETURN( pd3dDevice->CreateGeometryShader( pGeometryShaderBuffer->GetBufferPointer(),
                                              pGeometryShaderBuffer->GetBufferSize(), NULL, &GS_part_rend ) );

	pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"geo_alien.hlsl", "PS_Render", "ps_4_0_level_9_1", &pPixelShaderBuffer ) );

	 V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &PS_part_rend ) );
    DXUT_SetDebugName( PS_part_rend, "PSExplode" );


    SAFE_RELEASE( pVertexShaderBuffer );
	SAFE_RELEASE(pGeometryShaderBuffer);
    SAFE_RELEASE( pPixelShaderBuffer );

	D3D11_BUFFER_DESC buffDesc;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	buffDesc.CPUAccessFlags = 0;
	buffDesc.MiscFlags = 0;
	buffDesc.ByteWidth = MAX_PARTICLES * sizeof(VS_explode);
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	hr = pd3dDevice->CreateBuffer(&buffDesc,0,&g_pStreamBuffer);
	DXUT_SetDebugName( g_pStreamBuffer, " explode stream output buffer" );
	hr = pd3dDevice->CreateBuffer(&buffDesc,0,&g_pRenderBuffer);
	DXUT_SetDebugName( g_pRenderBuffer, "explode render buffer" );

    // Load the mesh
    V_RETURN( g_Mesh11.Create( pd3dDevice, L"tiny\\tiny.sdkmesh", true ) );

	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"particle_tex.png", NULL, NULL, &g_pTexRV, NULL );

    // Create a sampler state
    D3D11_SAMPLER_DESC SamDesc;
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 1;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
    SamDesc.MinLOD = 0;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN( pd3dDevice->CreateSamplerState( &SamDesc, &g_pSamLinear ) );
    DXUT_SetDebugName( g_pSamLinear, "Primary" );

    // Setup constant buffers
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

	D3D11_DEPTH_STENCIL_DESC descDS;
	ZeroMemory(&descDS, sizeof(descDS));
	descDS.DepthEnable = true;
	descDS.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;	
	descDS.DepthFunc = D3D11_COMPARISON_LESS_EQUAL     ;
	descDS.StencilEnable = FALSE;
	hr = pd3dDevice->CreateDepthStencilState( &descDS, &g_DepthState);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory( &blendDesc, sizeof(blendDesc) );
	blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;        
    blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F ;
	hr = pd3dDevice->CreateBlendState(&blendDesc, &additive_blend);


}

void spiked_alien::start_explode()
{
	//D3DXMatrixIdentity(&g_mCenterMesh);
	if(!exploding)
	{
		first_pass = true;
		exploding = true;
	/*	D3DXVECTOR3 vCenter( 1000, 200,200 );
		D3DXVECTOR3 origin(0,0,0);
		FLOAT fObjectRadius = 378.15607f;
		D3DXMATRIXA16 m;
		D3DXMatrixTranslation( &g_mCenterMesh, origin.x, origin.y, origin.z );
		D3DXMatrixRotationZ( &m, D3DX_PI/2.0f);
		g_mCenterMesh *= m;
		D3DXMatrixRotationY( &m, D3DX_PI );
		g_mCenterMesh *= m;
		D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
		g_mCenterMesh *= m;
		D3DXMatrixTransformation(&m,NULL,NULL,NULL,NULL,NULL,&vCenter);
		g_mCenterMesh *= m;*/
	}
	else
	{
		first_pass = true;
		exploding = false;
		/*D3DXVECTOR3 vCenter( 500, 100,100 );
		D3DXVECTOR3 origin(0,0,0);
		FLOAT fObjectRadius = 378.15607f;
		D3DXMATRIXA16 m;
		D3DXMatrixTranslation( &g_mCenterMesh, origin.x, origin.y, origin.z );
		D3DXMatrixRotationZ( &m, D3DX_PI/2.0f);
		g_mCenterMesh *= m;
		D3DXMatrixRotationY( &m, D3DX_PI );
		g_mCenterMesh *= m;
		D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
		g_mCenterMesh *= m;
		D3DXMatrixTransformation(&m,NULL,NULL,NULL,NULL,NULL,&vCenter);
		g_mCenterMesh *= m;*/
	}
}

void CALLBACK spiked_alien::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 light_pos  )
{

	 HRESULT hr;

	D3DXMATRIX mWorldViewProjection;
    D3DXVECTOR3 vLightDir;
    D3DXMATRIX mWorld;
    D3DXMATRIX mView;
    D3DXMATRIX mProj;

    // Get the projection & view matrix from the camera class
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();

    // Get the light direction
   // vLightDir = g_LightControl->GetLightDirection();
	if(exploding)
	{
		if(!first_pass)
		{
			pd3dImmediateContext->IASetInputLayout(part_in_layout);
		}
		else
		{
			pd3dImmediateContext->IASetInputLayout(g_pVertexLayout11);
		}
		pd3dImmediateContext->OMSetBlendState(additive_blend,NULL,0xffffffff);
		pd3dImmediateContext->OMSetDepthStencilState(g_DepthState, 0);
	}
	else
	{
		pd3dImmediateContext->IASetInputLayout(g_pVertexLayout11);
		
	}
	//pd3dImmediateContext->OMSetDepthStencilState(NULL, 1);
    // Per frame cb update
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pd3dImmediateContext->Map( g_pcbPSPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_PS_PER_FRAME* pPerFrame = ( CB_PS_PER_FRAME* )MappedResource.pData;
    float fAmbient = 0.1f;
    pPerFrame->m_vLightDirAmbient = D3DXVECTOR4( vLightDir.x, vLightDir.y, vLightDir.z, fAmbient );
	pPerFrame->m_Eye = D3DXVECTOR4(*g_Camera->GetEyePt(),0);
    pd3dImmediateContext->Unmap( g_pcbPSPerFrame, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( geo_alienPSPerFrameBind, 1, &g_pcbPSPerFrame );

    //Get the mesh
    //IA setup
    //pd3dImmediateContext->IASetInputLayout( g_pVertexLayout11 );
	int explode;
	if(exploding)
	{
		UINT offset = 0;
		if(first_pass)
		{
			UINT Strides[1];
			UINT Offsets[1];
			ID3D11Buffer* pVB[1];
			pVB[0] = g_Mesh11.GetVB11( 0, 0 );
			Strides[0] = ( UINT )g_Mesh11.GetVertexStride( 0, 0 );
			Offsets[0] = 0;
			pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
			pd3dImmediateContext->IASetIndexBuffer( g_Mesh11.GetIB11( 0 ), g_Mesh11.GetIBFormat11( 0 ), 0 );
			pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
			pd3dImmediateContext->HSSetShader(NULL,NULL,0);
			pd3dImmediateContext->DSSetShader(NULL,NULL,0);
			pd3dImmediateContext->PSSetShader( NULL, NULL, 0 );
			pd3dImmediateContext->GSSetShader(GS_part_firstpass,NULL,0);
		}
		else
		{
			pd3dImmediateContext->IASetVertexBuffers( 0, 1, &g_pRenderBuffer, &stride,&offset );
			pd3dImmediateContext->VSSetShader( VS_part_pass, NULL, 0 );
			pd3dImmediateContext->HSSetShader(NULL,NULL,0);
			pd3dImmediateContext->DSSetShader(NULL,NULL,0);
			pd3dImmediateContext->PSSetShader( NULL, NULL, 0 );
			pd3dImmediateContext->GSSetShader(GS_part_calc,NULL,0);
		}
		explode = 1;

		UINT off[1];
		off[0] = 0;
		pd3dImmediateContext->SOSetTargets( 1, &g_pStreamBuffer, off );

	}
	else
	{
		explode = 0;
		UINT Strides[1];
		UINT Offsets[1];
		ID3D11Buffer* pVB[1];
		pVB[0] = g_Mesh11.GetVB11( 0, 0 );
		Strides[0] = ( UINT )g_Mesh11.GetVertexStride( 0, 0 );
		Offsets[0] = 0;
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
		pd3dImmediateContext->IASetIndexBuffer( g_Mesh11.GetIB11( 0 ), g_Mesh11.GetIBFormat11( 0 ), 0 );
		 pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		pd3dImmediateContext->HSSetShader(NULL,NULL,0);
		pd3dImmediateContext->DSSetShader(NULL,NULL,0);
		pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		pd3dImmediateContext->GSSetShader(g_pGeoShader,NULL,0);
		pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	}

    // Set the shaders
   
    
    // Set the per object constant data
    mWorld = g_mCenterMesh;
    mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();

    mWorldViewProjection = mWorld * mView * mProj;
        
    // VS Per object
    V( pd3dImmediateContext->Map( g_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    CB_PER_OBJECT* pPerObject = ( CB_PER_OBJECT* )MappedResource.pData;
	D3DXMatrixTranspose( &pPerObject->m_View, &mView );
    D3DXMatrixTranspose( &pPerObject->m_World, &mWorld );
	D3DXMatrixTranspose(&pPerObject->m_Projection,&mProj);

	pPerObject->m_time = D3DXVECTOR4(fTime,fElapsedTime,explode,0);
	//pPerObject->m_vObjectColor = D3DXVECTOR4( 1, 1, 1, 1 );
    pd3dImmediateContext->Unmap( g_pcbVSPerObject, 0 );

    pd3dImmediateContext->VSSetConstantBuffers( geoalien_VSPerObjectBind, 1, &g_pcbVSPerObject );
	pd3dImmediateContext->GSSetConstantBuffers( geoalien_VSPerObjectBind, 1, &g_pcbVSPerObject );

    pd3dImmediateContext->PSSetConstantBuffers( geoalien_VSPerObjectBind, 1, &g_pcbVSPerObject );
	if(explode)
	{
		if(first_pass)
		{
			//Render
			SDKMESH_SUBSET* pSubset = NULL;
			D3D11_PRIMITIVE_TOPOLOGY PrimType;

			pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );

			for( UINT subset = 0; subset < g_Mesh11.GetNumSubsets( 0 ); ++subset )
			{
				// Get the subset
				pSubset = g_Mesh11.GetSubset( 0, subset );

				PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
				pd3dImmediateContext->IASetPrimitiveTopology( PrimType );

				// TODO: D3D11 - material loading
				ID3D11ShaderResourceView* pDiffuseRV = g_Mesh11.GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
				pd3dImmediateContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

				pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
			}
		}
		else
		{
			pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
			pd3dImmediateContext->DrawAuto();

		}
	}
	else
	{
		//Render
		SDKMESH_SUBSET* pSubset = NULL;
		D3D11_PRIMITIVE_TOPOLOGY PrimType;

		pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );

		for( UINT subset = 0; subset < g_Mesh11.GetNumSubsets( 0 ); ++subset )
		{
			// Get the subset
			pSubset = g_Mesh11.GetSubset( 0, subset );

			PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
			pd3dImmediateContext->IASetPrimitiveTopology( PrimType );

			// TODO: D3D11 - material loading
			ID3D11ShaderResourceView* pDiffuseRV = g_Mesh11.GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
			pd3dImmediateContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

			pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
		}
	}

	UINT off[1];
	off[0] = 0;
	ID3D11Buffer* nullbuffer[1] = { NULL };		
	pd3dImmediateContext->SOSetTargets( 1, nullbuffer, off);		

	//pBuffers[0] = NULL;
    //pd3dImmediateContext->SOSetTargets( 1, pBuffers, offset );
	ID3D11Buffer* tempB = g_pRenderBuffer;
	g_pRenderBuffer = g_pStreamBuffer;
	g_pStreamBuffer = tempB;
	first_pass = false;

	if(exploding)
	{
		UINT offset = 0;
		pd3dImmediateContext->IASetInputLayout(part_in_layout);

		pd3dImmediateContext->IASetVertexBuffers( 0, 1, &g_pRenderBuffer, &stride, &offset );
		pd3dImmediateContext->OMSetDepthStencilState(g_DepthState, 0);

		pd3dImmediateContext->VSSetShader(VS_part_calc,NULL,0);
		pd3dImmediateContext->PSSetShader(PS_part_rend,NULL,0);
		pd3dImmediateContext->GSSetShader(GS_part_rend,NULL,0);
		pd3dImmediateContext->HSSetShader(NULL,NULL,0);
		pd3dImmediateContext->DSSetShader(NULL,NULL,0);

		pd3dImmediateContext->VSSetConstantBuffers( geoalien_VSPerObjectBind, 1, &g_pcbVSPerObject );
		pd3dImmediateContext->GSSetConstantBuffers( geoalien_VSPerObjectBind, 1, &g_pcbVSPerObject );

		pd3dImmediateContext->PSSetConstantBuffers( geoalien_VSPerObjectBind, 1, &g_pcbVSPerObject );
		pd3dImmediateContext->PSSetShaderResources(1,1,&g_pTexRV);
		pd3dImmediateContext->PSSetSamplers(0,1,&g_pSamLinear);

		pd3dImmediateContext->OMSetBlendState(additive_blend,NULL,0xffffffff);

		pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

		pd3dImmediateContext->DrawAuto();

	}
   

	pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);
	//pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);
	pd3dImmediateContext->OMSetDepthStencilState(NULL, 0);
}