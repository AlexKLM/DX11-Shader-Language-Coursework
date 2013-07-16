#include "DXUT.h"
#include "Particle.h"

static const int MAX_PARTICLES =  30000;

struct particle_Vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 vel;
	float Timer;
	UINT Type;
};

struct particle_Vertex2
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 tex;
	D3DXVECTOR4 color;
};

struct GSCalc_Buffer
{
	float g_shouldemit;
	D3DXVECTOR3 emit_pos;
	D3DXVECTOR4 wind;
};

struct global_delta_Buffer
{
	D3DXVECTOR2 padding2;
	float GlobalTime;
	float DeltaTime;
};

struct render_vars_Buffer
{
	D3DXMATRIX m_view;
	D3DXMATRIX m_Projection;
};

Particle::Particle(void)
{
}


Particle::~Particle(void)
{
}


void Particle::destroy()
{
	SAFE_RELEASE( additive_blend );
	SAFE_RELEASE( calc_buffer );
	SAFE_RELEASE(render_buffer);
	SAFE_RELEASE(dt_buffer);

	SAFE_RELEASE(g_pRasterizerStateSolid);
	SAFE_RELEASE(g_pVertexLayout11);
	SAFE_RELEASE(g_pVertexBuffer);


	SAFE_RELEASE(g_DepthState);
	SAFE_RELEASE(Stream_target_buffer);
	SAFE_RELEASE(rendering_buffer);
	SAFE_RELEASE(start_buffer);

	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pVertexShaderPass);
	SAFE_RELEASE(g_pGeoShader);
	SAFE_RELEASE(g_pGeoShaderCalc);
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pSamLinear);

	SAFE_RELEASE(RandomTexture);
	SAFE_RELEASE(g_pRandRV);
	SAFE_RELEASE(g_pTexRV);
}

HRESULT Particle::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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
std::vector<particle_Vertex> seed;
int fire_amount = 0;
void Particle::generate_particles(int amount)
{
	fire_amount = 0;
	srand (time(NULL));
	particle_Vertex part_seed;
	for(int i = 0; i < amount; i++)
	{
		part_seed.pos.x = rand() % 1800 -900;
		part_seed.pos.y = 0;
		part_seed.pos.z = rand() % 1800 -900;

		part_seed.vel.x = 0;
		part_seed.vel.y =  rand() % 30+50;
		part_seed.vel.z = 0;

		part_seed.Timer = 0;
		part_seed.Type = UINT(0);
		fire_amount++;
		seed.push_back(part_seed);
	}
	
	
}


HRESULT Particle::setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext, int _amount,int type)
{
	first_cycle = true;
	HRESULT hr;
	generate_particles(_amount);
	//particle_Vertex seed[3] = //set up base emmiter
	//{
	//	{pos,D3DXVECTOR3(0,50,0),float(0),UINT(0)},
	//	{D3DXVECTOR3(202,0,220),D3DXVECTOR3(0,50,0),float(0),UINT(0)},
	//	{D3DXVECTOR3(-100,0,120),D3DXVECTOR3(0,50,0),float(0),UINT(0)}
	//};

	/*set_pos(pos);
	set_gravity(D3DXVECTOR4(0,-1,0,0));*/


	D3D11_BUFFER_DESC buffDesc;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	buffDesc.CPUAccessFlags = 0;
	buffDesc.MiscFlags = 0;
	buffDesc.ByteWidth = MAX_PARTICLES * sizeof(particle_Vertex);
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	hr = pd3dDevice->CreateBuffer(&buffDesc,0,&rendering_buffer);
	DXUT_SetDebugName( rendering_buffer, "rendering buffer" );
	hr = pd3dDevice->CreateBuffer(&buffDesc,0,&Stream_target_buffer);
	DXUT_SetDebugName( Stream_target_buffer, "stream out buffer" );


	//vbInitData.SysMemSlicePitch = 0;
	//vbInitData.SysMemPitch = sizeof(particle_Vertex);

	D3D11_SUBRESOURCE_DATA vbInitData;
	//ZeroMemory(&vbInitData,sizeof(D3D11_SUBRESOURCE_DATA));
	vbInitData.pSysMem = &seed[0];
	vbInitData.SysMemPitch = 0;
	buffDesc.ByteWidth = sizeof(particle_Vertex)* fire_amount;
	hr = pd3dDevice->CreateBuffer(&buffDesc,&vbInitData,&start_buffer);//seed buffer for first cycle

	
	const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TIMER", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT size = ARRAYSIZE(layout);
	stride = sizeof( particle_Vertex );

	ID3DBlob* pBlobVS = NULL;
	ID3DBlob* pBlobGS= NULL;
	ID3DBlob* pPSBlob = NULL;
	if(type ==0)
	{
	V_RETURN(CompileShaderFromFile(L"Fire_Part.hlsl","VSPassthrough","vs_4_0",&pBlobVS));


   V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(),
                                              pBlobVS->GetBufferSize(), NULL, &g_pVertexShaderPass ) );
    DXUT_SetDebugName( g_pVertexShaderPass, "VSPass" );

	V_RETURN( pd3dDevice->CreateInputLayout( layout, size, pBlobVS->GetBufferPointer(),
                                             pBlobVS->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

	pBlobVS = NULL;


	
	V_RETURN( CompileShaderFromFile( L"Fire_Render.hlsl", "Vertex_Render", "vs_4_0", &pBlobVS ) );

	V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "VSMain" );
	
	pBlobGS = NULL;
	V_RETURN(CompileShaderFromFile(L"Fire_Render.hlsl","GS_Render","gs_4_0",&pBlobGS));

	V_RETURN( pd3dDevice->CreateGeometryShader( pBlobGS->GetBufferPointer(), pBlobGS->GetBufferSize(), NULL, &g_pGeoShader ) );
    DXUT_SetDebugName( g_pGeoShader, "GeoRender" );


	D3D11_SO_DECLARATION_ENTRY pDecl[] =
	{
		// semantic name, semantic index, start component, component count, output slot
		{ 0,"POSITION", 0, 0, 3, 0 },   // output all components of position
		{ 0,"NORMAL", 0, 0, 3, 0 },     // output the first 3 of the normal
		{ 0,"TIMER", 0, 0, 1, 0 },     // output the first 2 texture coordinates
		{ 0,"TYPE", 0, 0, 1, 0 }, 
	};
	UINT strides[1] = { ((7*sizeof(float))) + (1 * sizeof(UINT) )}; 
	strides[0] = sizeof(float)*7 + sizeof(UINT) *1;
	UINT SOstride = strides[0];

	V_RETURN( CompileShaderFromFile( L"Fire_Part.hlsl", "GScalculate_Particles_main", "gs_4_0", &pBlobGS ) );

	V_RETURN(pd3dDevice->CreateGeometryShaderWithStreamOutput(pBlobGS->GetBufferPointer(),pBlobGS->GetBufferSize(),pDecl,4,strides,1,0 ,NULL,&g_pGeoShaderCalc));

    V_RETURN(CompileShaderFromFile( L"Fire_Render.hlsl", "PS_Render", "ps_4_0", &pPSBlob )) ;

	V_RETURN( pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(),
                                             pPSBlob->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "PSMain" );

	//V_RETURN( pd3dDevice->CreateGeometryShader( pBlobGSCalc->GetBufferPointer(),
 //                                             pBlobGSCalc->GetBufferSize(), NULL, &g_pGeoShaderCalc ) );
 //   DXUT_SetDebugName( g_pGeoShaderCalc, "GeoCalc" );


	}
	else //MISSLE
	{
		V_RETURN(CompileShaderFromFile(L"missle_Part.hlsl","VSPassthrough","vs_4_0",&pBlobVS));


		V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(),
			pBlobVS->GetBufferSize(), NULL, &g_pVertexShaderPass ) );
		DXUT_SetDebugName( g_pVertexShaderPass, "VSPass" );

		V_RETURN( pd3dDevice->CreateInputLayout( layout, size, pBlobVS->GetBufferPointer(),
			pBlobVS->GetBufferSize(), &g_pVertexLayout11 ) );
		DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

		pBlobVS = NULL;



		V_RETURN( CompileShaderFromFile( L"Missle_Render.hlsl", "Vertex_Render", "vs_4_0", &pBlobVS ) );

		V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pVertexShader ) );
		DXUT_SetDebugName( g_pVertexShader, "VSMain" );

		pBlobGS = NULL;
		V_RETURN(CompileShaderFromFile(L"Missle_Render.hlsl","GS_Render","gs_4_0",&pBlobGS));

		V_RETURN( pd3dDevice->CreateGeometryShader( pBlobGS->GetBufferPointer(), pBlobGS->GetBufferSize(), NULL, &g_pGeoShader ) );
		DXUT_SetDebugName( g_pGeoShader, "GeoRender" );

		D3D11_SO_DECLARATION_ENTRY pDecl[] =
		{
			// semantic name, semantic index, start component, component count, output slot
			{ 0,"POSITION", 0, 0, 3, 0 },   // output all components of position
			{ 0,"NORMAL", 0, 0, 3, 0 },     // output the first 3 of the normal
			{ 0,"TIMER", 0, 0, 1, 0 },     // output the first 2 texture coordinates
			{ 0,"TYPE", 0, 0, 1, 0 }, 
		};
		UINT strides[1] = { ((7*sizeof(float))) + (1 * sizeof(UINT) )}; 
		strides[0] = sizeof(float)*7 + sizeof(UINT) *1;
		UINT SOstride = strides[0];

		V_RETURN( CompileShaderFromFile( L"missle_Part.hlsl", "GScalculate_Particles_main", "gs_4_0", &pBlobGS ) );

		V_RETURN(pd3dDevice->CreateGeometryShaderWithStreamOutput(pBlobGS->GetBufferPointer(),pBlobGS->GetBufferSize(),pDecl,4,strides,1,0 ,NULL,&g_pGeoShaderCalc));


		V_RETURN(CompileShaderFromFile( L"Missle_Render.hlsl", "PS_Render", "ps_4_0", &pPSBlob )) ;

		V_RETURN( pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(),
			pPSBlob->GetBufferSize(), NULL, &g_pPixelShader ) );
		DXUT_SetDebugName( g_pPixelShader, "PSMain" );

	}
	
	SAFE_RELEASE( pBlobVS );
	SAFE_RELEASE(pPSBlob);
    SAFE_RELEASE( pBlobGS );

	 
	//V_RETURN( CompileShaderFromFile( L"Fire_Part.hlsl", "Vertex_Render2", "vs_4_0_level_9_1", &pBlobVS2 ) );

	D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;
	Desc.ByteWidth = sizeof( GSCalc_Buffer );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &calc_buffer ) );
    DXUT_SetDebugName( calc_buffer, "calc buffer" );

	
	Desc.ByteWidth = sizeof( global_delta_Buffer );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &dt_buffer ) );
    DXUT_SetDebugName( dt_buffer, "dt buffer" );

		
	Desc.ByteWidth = sizeof( render_vars_Buffer );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &render_buffer ) );
    DXUT_SetDebugName( render_buffer, "ren buffer" );
	


	create_randomtexture(pd3dDevice);
	D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"particle_tex2.png", NULL, NULL, &g_pTexRV, NULL );
	
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

    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamPoint );

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
void Particle::calculate_particle(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, CFirstPersonCamera *g_Camera,CDXUTDirectionWidget *wind)
{
	HRESULT hr;

	pd3dImmediateContext->IASetInputLayout(g_pVertexLayout11);
	pd3dImmediateContext->VSSetShader(g_pVertexShaderPass,NULL,0);
	pd3dImmediateContext->PSSetShader(NULL,NULL,0);
	pd3dImmediateContext->GSSetShader(g_pGeoShaderCalc,NULL,0);

	pd3dImmediateContext->OMSetDepthStencilState(g_DepthState, 0);

	 D3D11_MAPPED_SUBRESOURCE MappedResource;
    V(pd3dImmediateContext->Map( calc_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ));
	GSCalc_Buffer* calB = ( GSCalc_Buffer* )MappedResource.pData;
	calB->g_shouldemit = 1;
	calB->emit_pos = position;
	calB->wind = D3DXVECTOR4(wind->GetLightDirection(),1);
	pd3dImmediateContext->Unmap( calc_buffer, 0 );
	pd3dImmediateContext->GSSetConstantBuffers(0,1,&calc_buffer);
	

	V(pd3dImmediateContext->Map( dt_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ));
	global_delta_Buffer* dtB = ( global_delta_Buffer* )MappedResource.pData;
	dtB->DeltaTime = fTime;
	dtB->GlobalTime =  float( ( rand() % 10000 ) - 5000 );
	dtB->padding2 = D3DXVECTOR2(0,0);
	pd3dImmediateContext->Unmap( dt_buffer, 0 );
	pd3dImmediateContext->GSSetConstantBuffers(1,1,&dt_buffer);

	pd3dImmediateContext->GSSetSamplers(0,1,&g_pSamPoint);
	pd3dImmediateContext->GSSetShaderResources(0,1,&g_pRandRV);
	//pd3dImmediateContext->GSSetSamplers(0,1,&g_pSamLinear);


	//pd3dImmediateContext->OMSetDepthStencilState(g_DepthState, 1);

	UINT off[1];
	off[0] = 0;
	pd3dImmediateContext->SOSetTargets( 1, &Stream_target_buffer, off );

	

	UINT offset = 0;
	if(!first_cycle)
	{
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, &rendering_buffer, &stride,&offset );
	}
	else
	{
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, &start_buffer, &stride,&offset );
	}
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
	if(!first_cycle)
	{
		pd3dImmediateContext->DrawAuto();
	}
	else
	{
		pd3dImmediateContext->Draw(fire_amount,0);
	}


	
	ID3D11Buffer* nullbuffer[1] = { NULL };		
	pd3dImmediateContext->SOSetTargets( 1, nullbuffer, off);		

	//pBuffers[0] = NULL;
    //pd3dImmediateContext->SOSetTargets( 1, pBuffers, offset );
	ID3D11Buffer* tempB = rendering_buffer;
	rendering_buffer = Stream_target_buffer;
	Stream_target_buffer = tempB;
	first_cycle = false;
	//std::swap();
	pd3dImmediateContext->GSSetShader(NULL,NULL,0);

}


void CALLBACK Particle::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera )
{
	HRESULT hr;

	UINT offset = 0;
	D3DXMATRIX mWorldViewProjection;
    D3DXMATRIX mView;
    D3DXMATRIX mProj;
	mProj = *g_Camera->GetProjMatrix();
    mView = *g_Camera->GetViewMatrix();
	pd3dImmediateContext->IASetInputLayout(g_pVertexLayout11);
	pd3dImmediateContext->IASetVertexBuffers( 0, 1, &rendering_buffer, &stride, &offset );
	pd3dImmediateContext->OMSetDepthStencilState(g_DepthState, 0);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V( pd3dImmediateContext->Map( render_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    render_vars_Buffer* RenB = ( render_vars_Buffer* )MappedResource.pData;
	D3DXMatrixTranspose( &RenB->m_view, &mView );
	D3DXMatrixTranspose(&RenB->m_Projection,&mProj);
	pd3dImmediateContext->Unmap( render_buffer, 0 );
	
	pd3dImmediateContext->VSSetShader(g_pVertexShader,NULL,0);
	pd3dImmediateContext->PSSetShader(g_pPixelShader,NULL,0);
	pd3dImmediateContext->GSSetShader(g_pGeoShader,NULL,0);
	pd3dImmediateContext->HSSetShader(NULL,NULL,0);
	pd3dImmediateContext->DSSetShader(NULL,NULL,0);
	pd3dImmediateContext->GSSetConstantBuffers(0,1,&render_buffer);

	pd3dImmediateContext->PSSetShaderResources(0,1,&g_pTexRV);
	pd3dImmediateContext->PSSetSamplers(0,1,&g_pSamLinear);

	


	pd3dImmediateContext->OMSetBlendState(additive_blend,NULL,0xffffffff);

	

	

	

	//pd3dImmediateContext->VSSetConstantBuffers(2,1,&render_buffer);

	//pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );

	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	
	pd3dImmediateContext->DrawAuto();
	//pd3dImmediateContext->DrawIndexed(MAX_PARTICLES,MAX_PARTICLES,-1);

	pd3dImmediateContext->OMSetBlendState(NULL,NULL,NULL);
	pd3dImmediateContext->GSSetShader(NULL,NULL,0);
	//pd3dImmediateContext->OMSetDepthStencilState(NULL, 1);

	pd3dImmediateContext->OMSetDepthStencilState(NULL, 0);
}

void Particle::create_randomtexture(ID3D11Device* pd3dDevice)
{
	int iNumRandValues = 1024;
    srand( (unsigned)time(NULL));

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = new float[iNumRandValues * 4];
    InitData.SysMemPitch = iNumRandValues * 4 * sizeof( float );
    InitData.SysMemSlicePitch = iNumRandValues * 4 * sizeof( float );
    for( int i = 0; i < iNumRandValues * 4; i++ )
    {
		float f = float( ( rand() % 10000 ) - 5000 );
        ( ( float* )InitData.pSysMem )[i] = f;
    }

    D3D11_TEXTURE1D_DESC dstex;
    dstex.Width = iNumRandValues;
    dstex.MipLevels = 1;
    dstex.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    dstex.Usage = D3D11_USAGE_DEFAULT;
    dstex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    dstex.CPUAccessFlags = 0;
    dstex.MiscFlags = 0;
    dstex.ArraySize = 1;
    pd3dDevice->CreateTexture1D( &dstex, &InitData, &RandomTexture ) ;
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
    SRVDesc.Format = dstex.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    SRVDesc.Texture2D.MipLevels = dstex.MipLevels;
    pd3dDevice->CreateShaderResourceView( RandomTexture, &SRVDesc, &g_pRandRV ) ;

}

void Particle::set_pos(D3DXVECTOR3 input)
{
	position = input;
}

void Particle::set_gravity(D3DXVECTOR4 input)
{
	grav = input;
}
