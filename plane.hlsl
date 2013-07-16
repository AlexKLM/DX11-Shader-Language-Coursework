//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mWorldViewProjection;
	matrix		g_mWorld;
	float4		g_vObjectColor;
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir;
	float		g_fAmbient;
	float3		cam_pos;
	float		placeholder;
};

//--------------------------------------------------------------------------------------
// DepthStates/BlendStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
};

struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = mul( Input.vPosition, g_mWorldViewProjection );
	//Output.vNormal = mul( Input.vNormal, (float3x3)g_mWorld );
	//Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET
{
	float4 vDiffuse = float4(1,1,1,1);
	return vDiffuse;
	float3 Norm = float3(0,1,0);
	float3 light
	float fLighting = saturate( dot( g_vLightDir, Norm ) );
	fLighting = max( fLighting, g_fAmbient );
	
	return vDiffuse * fLighting;
}


technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSMain() ) );
        
        SetDepthStencilState( EnableDepth, 0 );
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
    }
}
