//--------------------------------------------------------------------------------------
// File: BasicHLSL11.cpp
//
// This sample shows a simple example of the Microsoft Direct3D's High-Level 
// Shader Language (HLSL) using the Effect interface. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"
#include "SDKMesh.h"
#include "resource.h"
#include "testing.h"
#include "plane.h"
#include "sphere.h"
#include "Worm.h"
#include "billboard.h"
#include "billboard2.h"
#include "spiked_alien.h"
#include "Particle.h"
#include "skybox.h"
#include "building.h"
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CFirstPersonCamera          g_Camera;               // A model viewing camera
CDXUTDirectionWidget        g_LightControl;
CD3DSettingsDlg             g_D3DSettingsDlg;       // Device settings dialog
CDXUTDialog                 g_HUD;                  // manages the 3D   
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
D3DXMATRIXA16               g_mCenterMesh;
D3DXVECTOR3					lightpos;
float                       g_fLightScale;

int                         g_nNumActiveLights;
int                         g_nActiveLight;
bool                        g_bShowHelp = false;    // If true, it renders the UI control text


float						displacement_level;
bool drawwire;
//----------------------------------------
//OBJECTS
//----------------------------------------

testing test;
plane tessplane;
sphere tesscube;
sphere lightsphere;
Worm fuse;
billboard board1;
billboard2 deboard;//distance estimate ray tracing
spiked_alien geo_alien;
Particle MissilePart;
Particle FirePart;
skybox sky;
building buildings;


CDXUTTextHelper*            g_pTxtHelper = NULL;


bool show_buildings = true;

//POSTPROCCESSING 
float width;
float height;

ID3D11RenderTargetView * rtv_render_to_texture_ori;
ID3D11RenderTargetView * rtv_render_to_texture_1;
ID3D11RenderTargetView * rtv_render_to_texture_2;

ID3D11Texture2D *		original_texture;
ID3D11Texture2D *		texture1;
ID3D11Texture2D *		texture2;

ID3D11ShaderResourceView* sr_texture_original;
ID3D11ShaderResourceView* sr_texture1;
ID3D11ShaderResourceView* sr_texture2;

ID3D11VertexShader*                 VSPPFirstPass; //extract bright spot
ID3D11PixelShader*                  PSPPFirstPass;

ID3D11VertexShader*                 VSPPBlur; //blur shader can be use twice to blur the image hor and vert
ID3D11PixelShader*                  PSPPBlur;

ID3D11VertexShader*                 VSPPComb; //combine shader to mix both 
ID3D11PixelShader*                  PSPPComb;

ID3D11SamplerState*  g_pSamLinear;
ID3D11DepthStencilState* g_DepthState;

ID3D11Buffer*       blur_cb_buffer;



//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4

#define IDC_DISPLACEMENTLVL       5
#define IDC_DISPLACEMENTLVL_STATIC  6
#define IDC_TOGGLEWIRE				7
#define IDC_TOGGLEDT				8 //distanced based tesselation
#define IDC_PARTITION_MODE        9
#define IDC_PARTITION_INTEGER     10
#define IDC_PARTITION_FRAC_EVEN   11
#define IDC_PARTITION_FRAC_ODD    12
#define IDC_TESSLVL       13
#define IDC_TESSLVL_STATIC  14
#define IDC_TOGGLEBUILDING  15
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );


bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext );

float Gauss(float n);
void Compute_blur(float dx, float dy);

void InitApp();
void RenderText();


struct blur_cbuffer
{
	D3DXVECTOR4 offset[15]; //x,y offset, z offset
	//float weight[15];
	//float placeholder[15];
};
blur_cbuffer blur_cb_data;
//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"BasicHLSL11" );
    DXUTCreateDevice (D3D_FEATURE_LEVEL_11_0, true, 800, 600 );
    //DXUTCreateDevice(true, 640, 480);
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}

float tess_lvl;
//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	displacement_level = 1.5;
	tess_lvl = 80;
	drawwire = false;
    D3DXVECTOR3 vLightDir( -1, 1, -1 );
    D3DXVec3Normalize( &vLightDir, &vLightDir );
    g_LightControl.SetLightDirection( vLightDir );

    // Initialize dialogs
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 23 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += 26, 170, 23, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += 26, 170, 23, VK_F2 );

    g_SampleUI.SetCallback( OnGUIEvent ); 
	iY = -30;

	WCHAR sz[100];
    //iY += 24;
	  
    swprintf_s( sz, L"Tessellation Level: %2.1f", tess_lvl );
    g_SampleUI.AddStatic( IDC_TESSLVL_STATIC, sz, -10, iY += 26, 150, 22 );
    g_SampleUI.AddSlider( IDC_TESSLVL, 10, iY += 24, 150, 22, 1, 80, tess_lvl );

	 iY += 14;
    swprintf_s( sz, L"Displacement Level: %2.1f", displacement_level );
    g_SampleUI.AddStatic( IDC_DISPLACEMENTLVL_STATIC, sz, -10, iY += 26, 150, 22 );
    g_SampleUI.AddSlider( IDC_DISPLACEMENTLVL, 10, iY += 24, 150, 22, 0, 150, (float)(displacement_level)/ 100.0f );

	iY += 10;
    g_SampleUI.AddCheckBox( IDC_TOGGLEWIRE, L"Toggle Wires", 20, iY += 26, 150, 22, drawwire );


	iY += 10;
    g_SampleUI.AddCheckBox( IDC_TOGGLEDT, L"Toggle Distanced Based tesselation", -120, iY += 26, 150, 22, true );
	

    iY += 10;
    g_SampleUI.AddRadioButton( IDC_PARTITION_INTEGER, IDC_PARTITION_MODE, L"Integer", 20, iY += 26, 170, 22 );
    g_SampleUI.AddRadioButton( IDC_PARTITION_FRAC_EVEN, IDC_PARTITION_MODE, L"Fractional Even", 20, iY += 26, 170, 22 );
    g_SampleUI.AddRadioButton( IDC_PARTITION_FRAC_ODD, IDC_PARTITION_MODE, L"Fractional Odd", 20, iY += 26, 170, 22 );
    g_SampleUI.GetRadioButton( IDC_PARTITION_INTEGER )->SetChecked( true );

	iY += 10;
    g_SampleUI.AddCheckBox( IDC_TOGGLEBUILDING, L"Toggle buildings", 20, iY += 26, 150, 22, true );
	
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    // Uncomment this to get debug information from D3D11
    //pDeviceSettings->d3d11.CreateFlags |= D3D11_CREATE_DEVICE_DEBUG;

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    UINT nBackBufferHeight = ( DXUTIsAppRenderingWithD3D9() ) ? DXUTGetD3D9BackBufferSurfaceDesc()->Height :
            DXUTGetDXGIBackBufferSurfaceDesc()->Height;

    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 2, 0 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

    // Draw help
    if( g_bShowHelp )
    {
        g_pTxtHelper->SetInsertionPos( 2, nBackBufferHeight - 20 * 6 );
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Controls:" );

        g_pTxtHelper->SetInsertionPos( 20, nBackBufferHeight - 20 * 5 );
        g_pTxtHelper->DrawTextLine( L"Rotate model: Left mouse button\n"
                                    L"Rotate light: Right mouse button\n"
                                    L"Rotate camera: Middle mouse button\n"
                                    L"Zoom camera: Mouse wheel scroll\n" );

        g_pTxtHelper->SetInsertionPos( 550, nBackBufferHeight - 20 * 5 );
        g_pTxtHelper->DrawTextLine( L"Hide help: F1\n"
                                    L"Quit: ESC\n" );
    }
    else
    {
        g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        g_pTxtHelper->DrawTextLine( L"Press F1 for help" );
    }

    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    g_LightControl.HandleMessages( hWnd, uMsg, wParam, lParam );

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp; break;
			case 0x49://i
				lightpos += D3DXVECTOR3(0,0,5);
				lightsphere.move(lightpos);
				break;
			case 0x4B://k
				lightpos += D3DXVECTOR3(0,0,-5);
				lightsphere.move(lightpos);
				break;
			case 0x4A://j
				lightpos += D3DXVECTOR3(-5,0,0);
				lightsphere.move(lightpos);
				break;
			case 0x4C://l
				lightpos += D3DXVECTOR3(5,0,0);
				lightsphere.move(lightpos);
				break;
			case 0x55: //U
				lightpos += D3DXVECTOR3(0,-5,0);
				lightsphere.move(lightpos);
				break;
			case 0x4F://O
				lightpos += D3DXVECTOR3(0,5,0);
				lightsphere.move(lightpos);
				break;
			case 0x56://v
				geo_alien.start_explode();
				break;
			case 0x20://spacebar
				board1.change_screen();
				deboard.change_screen();
				break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); break;

		case IDC_DISPLACEMENTLVL:
        {
            displacement_level = g_SampleUI.GetSlider( IDC_DISPLACEMENTLVL )->GetValue()/ 100.0f;
			tessplane.setdislvl(displacement_level);
            WCHAR sz[100];
            swprintf_s( sz, L"Displacement Level: %2.1f", displacement_level );
            g_SampleUI.GetStatic( IDC_DISPLACEMENTLVL_STATIC )->SetText( sz );
        }
            break;

		case IDC_TESSLVL:
        {
            tess_lvl = g_SampleUI.GetSlider( IDC_TESSLVL )->GetValue();
			tessplane.setTesslvl(tess_lvl);
			tesscube.set_tesslvl(tess_lvl);
			fuse.set_tesslvl(tess_lvl);
            WCHAR sz[100];
            swprintf_s( sz, L"Tesselation Level: %2.1f", tess_lvl );
            g_SampleUI.GetStatic( IDC_TESSLVL_STATIC )->SetText( sz );
        }
            break;
		case IDC_TOGGLEBUILDING:
			show_buildings = g_SampleUI.GetCheckBox( IDC_TOGGLEBUILDING )->GetChecked();
			break;
		case IDC_TOGGLEWIRE:
			tessplane.toggleWire(g_SampleUI.GetCheckBox( IDC_TOGGLEWIRE )->GetChecked());
			tesscube.toggleWire(g_SampleUI.GetCheckBox( IDC_TOGGLEWIRE )->GetChecked());
			fuse.toggleWire(g_SampleUI.GetCheckBox( IDC_TOGGLEWIRE )->GetChecked());
            break;
		case IDC_TOGGLEDT:
			tessplane.toggleDT(g_SampleUI.GetCheckBox( IDC_TOGGLEDT )->GetChecked());
            break;
        case IDC_PARTITION_INTEGER:
			tessplane.settessmethod(0);
			tesscube.settessmethod(0);
			fuse.settessmethod(0);
            break;
        case IDC_PARTITION_FRAC_EVEN:
			tessplane.settessmethod(1);
			tesscube.settessmethod(1);
			fuse.settessmethod(1);
            break;
        case IDC_PARTITION_FRAC_ODD:
			tesscube.settessmethod(2);
			tessplane.settessmethod(2);
			fuse.settessmethod(2);
            break;
    }

}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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


float Gauss(float n)
{
	 float blur_amount = 4;

    return (float)((1.0 / sqrt(2 * M_PI * blur_amount)) * exp(-(n * n) / (2 * blur_amount * blur_amount)));
}


void Compute_blur(float dx, float dy)
{
	int sample = 15;
	float sampleWeights[15];
    D3DXVECTOR2 sampleOffsets[15];
	D3DXVECTOR4 blurstuff[15];
	//x,y offset, z offset
	//I NEED TO STOP CHANGING THIS, the first sample is to be 0 offset on texel
	blurstuff[0].z = Gauss(0);
	//sampleWeights[0] = Gauss(0);
	blurstuff[0].x = 0;
	blurstuff[0].y = 0;
	//sampleOffsets[0] = D3DXVECTOR2(0,0);

	float totalW = blurstuff[0].z;
	for (int i = 0; i < 15 / 2; i++)
	{
		// Store weights for the positive and negative taps.
		float weight = Gauss(i + 1);

		blurstuff[i * 2 + 1].z = weight;
		blurstuff[i * 2 + 2].z = weight;

		totalW += weight * 2;

		float sampleOffset = i * 2 + 1.5f;

		D3DXVECTOR2 delta = D3DXVECTOR2(dx,dy)*sampleOffset;

		// Store texture coordinate offsets for the positive and negative taps.
		blurstuff[i * 2 + 1].x = delta.x;
		blurstuff[i * 2 + 1].y = delta.y;
		blurstuff[i * 2 + 2].x = -delta.x;
		blurstuff[i * 2 + 2].y = -delta.y;
	}

	// Normalize the list of sample weightings, so they will always sum to one.
	for (int i = 0; i < 15; i++)
	{
		//blurstuff[i].z = 1;
		blurstuff[i].z /= totalW;
	}
	//std::copy(&sampleOffsets[0], &sampleOffsets[15], blur_cb_data.offset);
	//std::copy(&sampleWeights[0], &sampleWeights[15], blur_cb_data.weight);
	std::copy(&blurstuff[0], &blurstuff[15], blur_cb_data.offset);
	//blur_cb_data.weight = sampleWeights;

}



//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );

   // D3DXVECTOR3 vCenter( 0.25767413f, -28.503521f, 111.00689f );
    FLOAT fObjectRadius = 378.15607f;
	lightpos = D3DXVECTOR3(300,300,-200);
	//test = new testing();
	hr = test.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext);
	hr = tessplane.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext);
	hr = tesscube.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext,80,D3DXVECTOR3(300,50,-200));//D3DXVECTOR3(300,50,-200
	hr = lightsphere.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext,10,lightpos);
	hr = fuse.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext);
	hr = board1.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext,D3DXVECTOR3(300,-300,-1000));
	hr = deboard.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext,D3DXVECTOR3(-300,-300,-1000));
	hr = geo_alien.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext);
	hr = FirePart.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext,80,0); //0 = fire
	hr = sky.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext,D3DXVECTOR3(0,0,0));
	hr = buildings.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext);
	hr = MissilePart.setup(pd3dDevice,pBackBufferSurfaceDesc,pUserContext,20,1);
    g_LightControl.SetRadius( 2000 );



	
    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye( 0.0f, 50.0f, -1000.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );

	g_Camera.SetRotateButtons(true,false,false);
    g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetEnablePositionMovement( true );
	g_Camera.SetScalers( 0.005f, 500.0f );

	D3D11_DEPTH_STENCIL_DESC descDS;
	ZeroMemory(&descDS, sizeof(descDS));
	descDS.DepthEnable = false;
	descDS.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;	
	descDS.DepthFunc = D3D11_COMPARISON_LESS;	
	

	descDS.StencilEnable = FALSE;
	hr = pd3dDevice->CreateDepthStencilState( &descDS, &g_DepthState);

	//setup stuff for post process


	ID3DBlob* pVertexShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"PP1.hlsl", "VS", "vs_4_0", &pVertexShaderBuffer ) );

    ID3DBlob* pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"PP1.hlsl", "PS", "ps_4_0", &pPixelShaderBuffer ) );

	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &VSPPFirstPass ) );
    DXUT_SetDebugName( VSPPFirstPass, "VSPost1" );
    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &PSPPFirstPass ) );
    DXUT_SetDebugName( PSPPFirstPass, "PSPost1" );

	pVertexShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"Gaussblur.hlsl", "VS", "vs_4_0", &pVertexShaderBuffer ) );

    pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"Gaussblur.hlsl", "PS", "ps_4_0", &pPixelShaderBuffer ) );

	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &VSPPBlur ) );
    DXUT_SetDebugName( VSPPBlur, "VSBlur" );
    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &PSPPBlur ) );
    DXUT_SetDebugName( PSPPBlur, "PSBlur" );

	pVertexShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"bloom_combine.hlsl", "VS", "vs_4_0", &pVertexShaderBuffer ) );

    pPixelShaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( L"bloom_combine.hlsl", "PS", "ps_4_0", &pPixelShaderBuffer ) );

	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                              pVertexShaderBuffer->GetBufferSize(), NULL, &VSPPComb ) );
    DXUT_SetDebugName( VSPPComb, "VSComb" );
    V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                             pPixelShaderBuffer->GetBufferSize(), NULL, &PSPPComb ) );
    DXUT_SetDebugName( PSPPComb, "PSComb" );





	
	D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;

    Desc.ByteWidth = sizeof( blur_cbuffer );
    V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &blur_cb_buffer ) );
    DXUT_SetDebugName( blur_cb_buffer, "blur_cb" );


    //g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
	width =  pBackBufferSurfaceDesc->Width;
	height = pBackBufferSurfaceDesc->Height;
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 100000.0f );
    //g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
   //g_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_SampleUI.SetSize( 170, 300 );
	//deboard.OnD3D11ResizedSwapChain(pd3dDevice,pSwapChain,pBackBufferSurfaceDesc,pUserContext);

		DXGI_FORMAT format;
	D3D11_TEXTURE2D_DESC info ;
	ZeroMemory (& info , sizeof ( info ));
	info.Width = pBackBufferSurfaceDesc->Width;
	info.Height = pBackBufferSurfaceDesc->Height;
	info.MipLevels = 1;
	info.ArraySize = 1;
	info.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	info.SampleDesc . Count = 1;
	info.Usage = D3D11_USAGE_DEFAULT;
	info.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE ;
	info.CPUAccessFlags = 0;
	info.MiscFlags = 0;
	hr = pd3dDevice->CreateTexture2D(&info,nullptr,&original_texture);
	hr = pd3dDevice->CreateTexture2D(&info,nullptr,&texture1);
	hr = pd3dDevice->CreateTexture2D(&info,nullptr,&texture2);
	format = info.Format;

 	D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP        ;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP        ;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP        ;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = 0;
    hr = pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamLinear );

	// render target view
	D3D11_RENDER_TARGET_VIEW_DESC target_info;
	target_info.Format = format;
	target_info.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	target_info.Texture2D.MipSlice = 0;
	hr = pd3dDevice->CreateRenderTargetView(original_texture, &target_info ,&rtv_render_to_texture_ori);
	hr = pd3dDevice->CreateRenderTargetView(texture1, &target_info ,&rtv_render_to_texture_1);
	hr = pd3dDevice->CreateRenderTargetView(texture2, &target_info ,&rtv_render_to_texture_2);

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_info;
	shader_resource_info.Format = format;
	shader_resource_info.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_info.Texture2D . MostDetailedMip = 0;
	shader_resource_info.Texture2D . MipLevels = 1;
	hr = pd3dDevice->CreateShaderResourceView(original_texture, &shader_resource_info ,&sr_texture_original);
	hr = pd3dDevice->CreateShaderResourceView(texture1, &shader_resource_info ,&sr_texture1);
	hr = pd3dDevice->CreateShaderResourceView(texture2, &shader_resource_info ,&sr_texture2);
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

		//Render light arrow
    D3DXMATRIX mView;
    D3DXMATRIX mProj;
	mProj = ( *g_Camera.GetProjMatrix() );
    mView = ( *g_Camera.GetViewMatrix() );
	D3DXCOLOR arrowColor = D3DXCOLOR( 1, 1, 0, 1 );
	//hr = g_LightControl.OnRender11( arrowColor, &mView, &mProj, g_Camera.GetEyePt() );
	FirePart.calculate_particle(pd3dDevice,pd3dImmediateContext,fElapsedTime,&g_Camera,&g_LightControl);
	MissilePart.calculate_particle(pd3dDevice,pd3dImmediateContext,fElapsedTime,&g_Camera,&g_LightControl);
	board1.RenderTexture(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_LightControl);	
    deboard.RenderTexture(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_LightControl);	

    // Clear the render target and depth stencil
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    //ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( rtv_render_to_texture_ori, ClearColor );
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->OMSetRenderTargets(1,&rtv_render_to_texture_ori,pDSV);
	
	
	
	
	pd3dImmediateContext->OMSetDepthStencilState(NULL, 0);
	sky.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);

	lightsphere.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	tessplane.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	tesscube.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	fuse.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,&g_LightControl);
	board1.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	deboard.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	
	test.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	if(show_buildings)
	{
		buildings.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	}
	geo_alien.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera,lightpos);
	FirePart.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera);
	MissilePart.Render(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext,&g_Camera);
	//ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();

	
	pd3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	pd3dImmediateContext->OMSetBlendState(NULL,NULL,0xffffffff);
	//find bright spot 
	pd3dImmediateContext->ClearRenderTargetView( rtv_render_to_texture_1, ClearColor );
    pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->OMSetRenderTargets(1,&rtv_render_to_texture_1,pDSV);

	pd3dImmediateContext->VSSetShader( VSPPFirstPass, NULL, 0 );
    pd3dImmediateContext->PSSetShader( PSPPFirstPass, NULL, 0 );

	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );

	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	pd3dImmediateContext->PSSetShaderResources(0,1,&sr_texture_original);
	//pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(9,0);

	//blur hori
	pd3dImmediateContext->ClearRenderTargetView( rtv_render_to_texture_2, ClearColor );
    pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->OMSetRenderTargets(1,&rtv_render_to_texture_2,pDSV);


	Compute_blur(1.0f / width, 0);

	D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pd3dImmediateContext->Map( blur_cb_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    blur_cbuffer* pPerFrame = ( blur_cbuffer* )MappedResource.pData;
	std::copy(&blur_cb_data.offset[0], &blur_cb_data.offset[15], pPerFrame->offset);
	//std::copy(&blur_cb_data.offset[0], &blur_cb_data.offset[15], pPerFrame->offset);
	//std::copy(&blur_cb_data.weight[0], &blur_cb_data.weight[15], pPerFrame->weight);
    pd3dImmediateContext->Unmap( blur_cb_buffer, 0 );

	pd3dImmediateContext->VSSetShader( VSPPBlur, NULL, 0 );
    pd3dImmediateContext->PSSetShader( PSPPBlur, NULL, 0 );

	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );

	pd3dImmediateContext->PSSetConstantBuffers(0,1,&blur_cb_buffer);
	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	pd3dImmediateContext->PSSetShaderResources(0,1,&sr_texture1);
	//pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(9,0);

	

	//blur vert
	pd3dImmediateContext->ClearRenderTargetView( rtv_render_to_texture_1, ClearColor );
     pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->OMSetRenderTargets(1,&rtv_render_to_texture_1,pDSV);



	Compute_blur(0, 1.0f / height);

    V( pd3dImmediateContext->Map( blur_cb_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    pPerFrame = ( blur_cbuffer* )MappedResource.pData;
	std::copy(&blur_cb_data.offset[0], &blur_cb_data.offset[15], pPerFrame->offset);
	//std::copy(&blur_cb_data.offset[0], &blur_cb_data.offset[15], pPerFrame->offset);
	//std::copy(&blur_cb_data.weight[0], &blur_cb_data.weight[15], pPerFrame->weight);
    pd3dImmediateContext->Unmap( blur_cb_buffer, 0 );

	pd3dImmediateContext->VSSetShader( VSPPBlur, NULL, 0 );
    pd3dImmediateContext->PSSetShader( PSPPBlur, NULL, 0 );

	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );

	pd3dImmediateContext->PSSetConstantBuffers(0,1,&blur_cb_buffer);
	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	pd3dImmediateContext->PSSetShaderResources(0,1,&sr_texture2);
	//pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(9,0);


	//combine effect
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
     pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	pd3dImmediateContext->OMSetRenderTargets(1,&pRTV,pDSV);




 //   V( pd3dImmediateContext->Map( blur_cb_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
 //   pPerFrame = ( blur_cbuffer* )MappedResource.pData;
	//std::copy(&blur_cb_data.offset[0], &blur_cb_data.offset[15], pPerFrame->offset);
 //   pd3dImmediateContext->Unmap( blur_cb_buffer, 0 );

	pd3dImmediateContext->VSSetShader( VSPPComb, NULL, 0 );
    pd3dImmediateContext->PSSetShader( PSPPComb, NULL, 0 );

	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );

	pd3dImmediateContext->PSSetConstantBuffers(0,1,&blur_cb_buffer);
	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamLinear);
	pd3dImmediateContext->PSSetShaderResources(0,1,&sr_texture1);//bloom blurred
	pd3dImmediateContext->PSSetShaderResources(1,1,&sr_texture_original);//base tex
	//pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid );
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(9,0);


    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_SampleUI.OnRender( fElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_D3DSettingsDlg.OnD3D11DestroyDevice();
    //CDXUTDirectionWidget::StaticOnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
	test.destroy();
	tessplane.destroy();
	lightsphere.destroy();
	tesscube.destroy();
	fuse.destroy();
	deboard.destroy();
	board1.destroy();
	geo_alien.destroy();
	FirePart.destroy();
    SAFE_DELETE( g_pTxtHelper );
	SAFE_RELEASE(g_DepthState);
    //g_Mesh11.Destroy();
    //            
    //SAFE_RELEASE( g_pVertexLayout11 );
    //SAFE_RELEASE( g_pVertexBuffer );
    //SAFE_RELEASE( g_pIndexBuffer );
    //SAFE_RELEASE( g_pVertexShader );
    //SAFE_RELEASE( g_pPixelShader );
    //SAFE_RELEASE( g_pSamLinear );

 /*   SAFE_RELEASE( g_pcbVSPerObject );
    SAFE_RELEASE( g_pcbPSPerObject );
    SAFE_RELEASE( g_pcbPSPerFrame );*/
}



