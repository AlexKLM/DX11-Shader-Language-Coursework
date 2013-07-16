#include "DXUT.h"
#include "billboard2.h"

struct CB_PER_OBJECT
{
    D3DXMATRIX mViewProjection;
	D3DXMATRIX m_World;
   	D3DXVECTOR3		mcam_pos;

	float		mplaceholder;
	D3DXVECTOR4 m_vObjectColor;
};

struct CB_PS_PER_FRAME
{
    D3DXVECTOR4 m_vLightDirAmbient;
};

struct ray_PS
{
    D3DXMATRIX View;
	D3DXVECTOR4 viewpos;
	int type;
	D3DXVECTOR3 padding;
};

struct Points
{
	float m_vPosition[3];
	float m_texture[2];
};

const Points g_Quad[4] ={

	{160,120,0,1,0},
	{160,-120,0,0},
	{-160,-120,0,0,1},
	{-160,120,0,1,1},
};

billboard2::billboard2(void)
{
	rotation = 0;
}


billboard2::~billboard2(void)
{
}

void billboard2::destroy()
{
	       
	g_Mesh11.Destroy();

    SAFE_RELEASE( g_pVertexLayout11 );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pPixelShader );
	SAFE_RELEASE( g_pHullShaderInteger );
    SAFE_RELEASE( g_pDomainShader );

	SAFE_RELEASE( g_pVertexShaderRay );
    SAFE_RELEASE( g_pPixelShaderRay );
	SAFE_RELEASE( g_pcbPSraytrace );
	SAFE_RELEASE(RayTraceVertexLayout11);
	 
	SAFE_RELEASE( g_pSamLinear );
	SAFE_RELEASE( g_pControlPointVB);
	SAFE_RELEASE(g_pTextureRV);
	SAFE_RELEASE(rtv_render_to_texture);
	SAFE_RELEASE(reflection_texture);
	SAFE_RELEASE(srv_render_to_texture);

	SAFE_RELEASE( g_pRasterizerStateSolid);

    SAFE_RELEASE( g_pcbVSPerObject );
    SAFE_RELEASE( g_pcbPSPerObject );
    SAFE_RELEASE( g_pcbPSPerFrame );
}

HRESULT billboard2::CompileShaderFromFile( WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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

HRESULT billboard2::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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


void billboard2::change_screen()
{
	if(type < 2)
	{
		type++;
	}
	else
	{
		type = 0;
	}
}

HRESULT billboard2::setup(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,void* pUserContext,D3DXVECTOR3 pos)
{
	HRESULT hr;
	//300, -200,-200
	D3DXVECTOR3 vCenter( pos.x, pos.y,pos.z );
	//D3DXMatrixIdentity(&g_mCenterMesh);
    FLOAT fObjectRadius = 378.15607f;
	type = 0;

    D3DXMatrixTranslation( &g_mCenterMesh, -vCenter.x, -vCenter.y, -vCenter.z );
    //D3DXMATRIXA16 m;
    //D3DXMatrixRotationY( &m, D3DX_PI );
    //g_mCenterMesh *= m;
    //D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    //g_mCenterMesh *= m;

	ID3DBlob* pBlobVS = NULL;
    ID3DBlob* pBlobHSInt = NULL;
	ID3DBlob* pBlobDS = NULL;
    ID3DBlob* pBlobPS = NULL;

	 D3D_SHADER_MACRO integerPartitioning[] = { { "BEZIER_HS_PARTITION", "\"integer\"" }, { 0 } };

	V_RETURN( CompileShaderFromFile( L"billboard.hlsl", NULL, "QuadTess_VS", "vs_5_0",  &pBlobVS ) );
    V_RETURN( CompileShaderFromFile( L"billboard.hlsl", integerPartitioning, "QuadTess_HS", "hs_5_0", &pBlobHSInt ) );
    V_RETURN( CompileShaderFromFile( L"billboard.hlsl", NULL, "QuadTess_DS", "ds_5_0", &pBlobDS ) );
    V_RETURN( CompileShaderFromFile( L"billboard.hlsl", NULL, "QuadTess_PS", "ps_5_0", &pBlobPS ) );


	hr = D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"frog2.png", NULL, NULL, &g_pTextureRV, NULL );


	D3D11_RASTERIZER_DESC RasterDesc;
    ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_NONE;
    RasterDesc.DepthClipEnable = TRUE;
    V_RETURN( pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateSolid ) );
    DXUT_SetDebugName( g_pRasterizerStateSolid, "Solid" );

    // Create shaders
    V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pVertexShader ) );
    DXUT_SetDebugName( g_pVertexShader, "QuadTess_VS" );

    V_RETURN( pd3dDevice->CreateHullShader( pBlobHSInt->GetBufferPointer(), pBlobHSInt->GetBufferSize(), NULL, &g_pHullShaderInteger ) );
    DXUT_SetDebugName( g_pHullShaderInteger, "QuadTess_HS int" );

    V_RETURN( pd3dDevice->CreateDomainShader( pBlobDS->GetBufferPointer(), pBlobDS->GetBufferSize(), NULL, &g_pDomainShader ) );
    DXUT_SetDebugName( g_pDomainShader, "QuadTess_DS" );

    V_RETURN( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &g_pPixelShader ) );
    DXUT_SetDebugName( g_pPixelShader, "QuadTess_PS" );



    // Create our vertex input layout
    const D3D11_INPUT_ELEMENT_DESC patchlayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
    };

    V_RETURN( pd3dDevice->CreateInputLayout( patchlayout, ARRAYSIZE( patchlayout ), pBlobVS->GetBufferPointer(),
                                             pBlobVS->GetBufferSize(), &g_pVertexLayout11 ) );
    DXUT_SetDebugName( g_pVertexLayout11, "Primary" );


	SAFE_RELEASE( pBlobVS );
    SAFE_RELEASE( pBlobHSInt );
    SAFE_RELEASE( pBlobDS );
    SAFE_RELEASE( pBlobPS );

	ID3DBlob* pBlobVSRay = NULL;
	ID3DBlob* pBlobPSRay = NULL;

	ID3DBlob* pVertexShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"raytrace2.hlsl", "VS_CanvasSetup", "vs_4_0", &pVertexShaderBuffer ) );

    ID3DBlob* pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"raytrace2.hlsl", "Image", "ps_4_0", &pPixelShaderBuffer ) );


    V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShaderRay ) );
    DXUT_SetDebugName( g_pVertexShaderRay, "VSRay" );
    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShaderRay ) );
    DXUT_SetDebugName( g_pPixelShaderRay, "PSRay" );


	const D3D11_INPUT_ELEMENT_DESC raylayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	V_RETURN( pd3dDevice->CreateInputLayout( raylayout, ARRAYSIZE( raylayout ), pVertexShaderBuffer->GetBufferPointer(),
                                             pVertexShaderBuffer->GetBufferSize(), &RayTraceVertexLayout11 ) );
    DXUT_SetDebugName( RayTraceVertexLayout11, "Primary" );

	SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );
	SAFE_RELEASE( pBlobVSRay );
	SAFE_RELEASE( pBlobPSRay );
		V_RETURN( g_Mesh11.Create( pd3dDevice, L"tiny\\tiny.sdkmesh", true ) );

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

	Desc.ByteWidth = sizeof( ray_PS );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSraytrace ) );
    DXUT_SetDebugName( g_pcbPSraytrace, "ray_ps" );

	
    D3D11_BUFFER_DESC vbDesc;
    ZeroMemory( &vbDesc, sizeof(D3D11_BUFFER_DESC) );
    vbDesc.ByteWidth = sizeof(Points) * ARRAYSIZE(g_Quad);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbInitData;
    ZeroMemory( &vbInitData, sizeof(vbInitData) );
    vbInitData.pSysMem = g_Quad;
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

	//---------------------------
	//setup texture
	//---------------------------
		
	//texture2d describtion
	DXGI_FORMAT format;
	D3D11_TEXTURE2D_DESC info ;
	ZeroMemory (& info , sizeof ( info ));
	info.Width = 800;
	info.Height = 600;
	info.MipLevels = 1;
	info.ArraySize = 1;
	info.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	info.SampleDesc . Count = 1;
	info.Usage = D3D11_USAGE_DEFAULT;
	info.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE ;
	info.CPUAccessFlags = 0;
	info.MiscFlags = 0;
	hr = pd3dDevice->CreateTexture2D(&info,nullptr,&reflection_texture);
	format = info.Format;

 

	// render target view
	D3D11_RENDER_TARGET_VIEW_DESC target_info;
	target_info.Format = format;
	target_info.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	target_info.Texture2D.MipSlice = 0;
	hr = pd3dDevice->CreateRenderTargetView(reflection_texture, &target_info ,&rtv_render_to_texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_info;
	shader_resource_info.Format = format;
	shader_resource_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_info.Texture2D . MostDetailedMip = 0;
	shader_resource_info.Texture2D . MipLevels = 1;
	hr = pd3dDevice->CreateShaderResourceView(reflection_texture, &shader_resource_info ,&srv_render_to_texture);

	//D3D11_SHADER_RESOURCE_VIEW_DESC reflect_SRVDesc;
	//memset( &reflect_SRVDesc, 0, sizeof( reflect_SRVDesc ) );
	//reflect_SRVDesc.Format = format;
	//reflect_SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//reflect_SRVDesc.Texture2D.MipLevels = 1;

	//hr = pd3dDevice->CreateShaderResourceView(reflection_texture,&reflect_SRVDesc,&reflect_SRview);
	
	D3DXVECTOR3 Eye(1.5, 0, 1.5 );
    D3DXVECTOR3 At( 0.0f, 0.0f, 0.0f );
    billboard_camera.SetViewParams( &Eye, &At );

	return S_OK;
}

HRESULT CALLBACK billboard2::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    billboard_camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 5000.0f );
	billboard_camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	 return S_OK;
}

void billboard2::setup_texture()
{
	
}

void CALLBACK billboard2::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext, CFirstPersonCamera *g_Camera,D3DXVECTOR3 lightpos )
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
    pPerFrame->m_vLightDirAmbient = D3DXVECTOR4( vLightDir.x, vLightDir.y, vLightDir.z, fAmbient );
    pd3dImmediateContext->Unmap( g_pcbPSPerFrame, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( 1, 1, &g_pcbPSPerFrame );

	
	UINT Stride = sizeof( Points );


    
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
	pPerObject->m_vObjectColor = D3DXVECTOR4( 1, 1, 1, 1 );
    pd3dImmediateContext->Unmap( g_pcbVSPerObject, 0 );

    pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &g_pcbVSPerObject );
	pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &g_pcbVSPerObject );
    pd3dImmediateContext->HSSetConstantBuffers( 0, 1, &g_pcbVSPerObject );
    pd3dImmediateContext->DSSetConstantBuffers( 0, 1, &g_pcbVSPerObject );

    pd3dImmediateContext->PSSetConstantBuffers( 0, 1, &g_pcbVSPerObject );

	//pd3dImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );
	//pd3dImmediateContext->HSSetShaderResources( 0, 1, &g_pTextureRV );
	//pd3dImmediateContext->DSSetShaderResources( 0, 1, &g_pTextureRV );
	pd3dImmediateContext->PSSetShaderResources( 0, 1, &srv_render_to_texture );
		pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);



	pd3dImmediateContext->VSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->HSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->DSSetSamplers(0, 1, &g_pSamLinear);
    pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	// Set the shaders
    pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

	pd3dImmediateContext->HSSetShader( g_pHullShaderInteger, NULL, 0 );
	pd3dImmediateContext->DSSetShader( g_pDomainShader, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

    //Render
    SDKMESH_SUBSET* pSubset = NULL;
    D3D11_PRIMITIVE_TOPOLOGY PrimType;

	    UINT Offset = 0;

	pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	pd3dImmediateContext->IASetInputLayout( g_pVertexLayout11 );
	 pd3dImmediateContext->IASetVertexBuffers( 0, 1, &g_pControlPointVB, &Stride, &Offset );

    //pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST );
	pd3dImmediateContext->Draw( sizeof(g_Quad), 0 );
}

void CALLBACK billboard2::RenderTexture( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext,CDXUTDirectionWidget *g_LightControl )
{
	HRESULT hr;

     float ClearColor[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

	pd3dImmediateContext->ClearRenderTargetView( rtv_render_to_texture, ClearColor );
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->OMSetRenderTargets(1,&rtv_render_to_texture,pDSV);
	D3DXMATRIX mWorld;
    D3DXMATRIX mView;
    D3DXMATRIX mProj;

	D3DXVECTOR3* cam_pos = (D3DXVECTOR3*)billboard_camera.GetEyePt();
	rotation += 0.001;
	//radius for mandlebulb = 2;
	cam_pos->x = sin(rotation) * 2.5;
	cam_pos->z = cos(rotation) * 2.5;
		pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);
    D3DXVECTOR3 At( 0.0f, 0.0f, 0.0f );
    billboard_camera.SetViewParams( cam_pos, &At );
	cam_pos = (D3DXVECTOR3*)billboard_camera.GetEyePt();
  	D3DXMatrixTranspose( &mView, billboard_camera.GetViewMatrix() );

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V( pd3dImmediateContext->Map( g_pcbPSraytrace, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    ray_PS* pPerObject = ( ray_PS* )MappedResource.pData;
	pPerObject->View = mView;
	pPerObject->viewpos = D3DXVECTOR4(cam_pos->x,cam_pos->y,cam_pos->z,1);
	pPerObject->type = type;
    pd3dImmediateContext->Unmap( g_pcbPSraytrace, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( 0, 1, &g_pcbPSraytrace );


	/*pd3dImmediateContext->IASetInputLayout( RayTraceVertexLayout11 );
    UINT Strides[1];
    UINT Offsets[1];
    ID3D11Buffer* pVB[1];
    pVB[0] = g_Mesh11.GetVB11( 0, 0 );
    Strides[0] = ( UINT )g_Mesh11.GetVertexStride( 0, 0 );
    Offsets[0] = 0;
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
    pd3dImmediateContext->IASetIndexBuffer( g_Mesh11.GetIB11( 0 ), g_Mesh11.GetIBFormat11( 0 ), 0 );*/

    // Set the shaders
    pd3dImmediateContext->VSSetShader( g_pVertexShaderRay, NULL, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShaderRay, NULL, 0 );
	pd3dImmediateContext->GSSetShader(NULL,NULL,0);
	pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
    //Render
    SDKMESH_SUBSET* pSubset = NULL;
   pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	 pd3dImmediateContext->Draw(9,0);

    //for( UINT subset = 0; subset < g_Mesh11.GetNumSubsets( 0 ); ++subset )
    //{
    //    // Get the subset
    //    pSubset = g_Mesh11.GetSubset( 0, subset );

    //    PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
    //    pd3dImmediateContext->IASetPrimitiveTopology( PrimType );

    //    // TODO: D3D11 - material loading
    //    ID3D11ShaderResourceView* pDiffuseRV = g_Mesh11.GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
    //    pd3dImmediateContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

    //    pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
    //}


}