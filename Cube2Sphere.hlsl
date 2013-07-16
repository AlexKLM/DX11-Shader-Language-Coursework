//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mViewProjection;
	matrix		g_mWorld;
	matrix		g_mView;
	float3		g_mcam_pos;
	float		sphere_size;
	float4		tess_lvl;//x = tess_lvl
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir;
	float		g_fAmbient;
	float3		cam_pos;
	float		placeholder;
};


#ifndef BEZIER_HS_PARTITION
#define BEZIER_HS_PARTITION "integer"
#endif // BEZIER_HS_PARTITION

// The input patch size.  In this sample, it is 16 control points.
// This value should match the call to IASetPrimitiveTopology()
#define INPUT_PATCH_SIZE 4

// The output patch size.  In this sample, it is also 16 control points.
#define OUTPUT_PATCH_SIZE 4

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
struct VS_CONTROL_POINT_INPUT
{
	float3 vPosition	: POSITION;
};

struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition	: POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_CONTROL_POINT_OUTPUT QuadTess_VS( VS_CONTROL_POINT_INPUT Input )
{
    VS_CONTROL_POINT_OUTPUT Output;

    Output.vPosition = Input.vPosition;
    return Output;
}



//--------------------------------------------------------------------------------------
// Hull Shader
//--------------------------------------------------------------------------------------
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4]             : SV_TessFactor;
    float Inside[2]            : SV_InsideTessFactor;
};

struct Tri_ConstantOutput
{
	float edges [3]: SV_TessFactor;
    float inside: SV_InsideTessFactor;
};

struct HS_OUTPUT
{
    float3 vPosition           : BEZIERPOS;
};


HS_CONSTANT_DATA_OUTPUT BezierConstantHS( InputPatch<VS_CONTROL_POINT_OUTPUT, INPUT_PATCH_SIZE> ip,
                                          uint PatchID : SV_PrimitiveID )
{    
    HS_CONSTANT_DATA_OUTPUT Output;

    float TessAmount = tess_lvl.x;//dynamic tess amount to replace 8

    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount; 
    Output.Inside[0] = Output.Inside[1] = TessAmount;

    return Output;
}



[domain("quad")]
[partitioning(BEZIER_HS_PARTITION)]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("BezierConstantHS")]
HS_OUTPUT QuadTess_HS(InputPatch<VS_CONTROL_POINT_OUTPUT, INPUT_PATCH_SIZE> p, 
                    uint i : SV_OutputControlPointID,
                    uint PatchID : SV_PrimitiveID)
{
	HS_OUTPUT Output;
    Output.vPosition = p[i].vPosition;
    return Output;
}

//----------------------------------------------------------------------------------
//Domain Shader
//----------------------------------------------------------------------------------
struct DS_OUTPUT
{
    float4 vPosition        : SV_POSITION;
    float3 vWorldPos        : WORLDPOS;
    float3 vNormal            : NORMAL;
};


//procedural displacement


[domain("quad")]
DS_OUTPUT QuadTess_DS( HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad )
{
	DS_OUTPUT Output;
	float3 verticalPos1 = lerp(quad[0].vPosition,quad[1].vPosition,UV.y);
	float3 verticalPos2 = lerp(quad[3].vPosition,quad[2].vPosition,UV.y);
	float3 finalPos = lerp(verticalPos1,verticalPos2,UV.x);

	//float3 normal = cross((quad[0].vPosition-quad[1].vPosition),(quad[1].vPosition-quad[2].vPosition));
	float3 Norm = normalize(finalPos);
	float3 spherePos = Norm * sphere_size;
	float3 WorldPos =  mul( float4(spherePos,1), g_mWorld );
	//float3 pos = normalize(spherePos);

	Output.vWorldPos =  WorldPos;

	//Output.vPosition = mul( float4(WorldPos,1), g_mViewProjection );
	Output.vPosition = mul (float4(WorldPos, 1), g_mViewProjection);
	
	Norm = normalize(cross((quad[0].vPosition - spherePos),(spherePos-quad[2].vPosition)));
	//Output.tex = UV;
	//Output.vNormal = Norm;
	float3 normal = (spherePos - float3(0,0,0));
	Output.vNormal =  mul( normal, (float3x3)g_mWorld );

	return Output;
}






float4 QuadTess_PS( DS_OUTPUT Input ) : SV_TARGET
{
	float shininess = 100;
	float3 n = normalize(Input.vNormal);
	float4 col = float4(float3(0,0,0)*0.2,1);
	float4 specularColor = col*shininess;
	//return float4(n,1);
	float3 view = normalize(cam_pos - Input.vWorldPos );
	float4 result;
	float4 amb = float4(col.xyz,1);
	//float3 emissive =  float4(0.1,0.1,0.1,0.1);
	float3 l = normalize(g_vLightDir - Input.vWorldPos);
	float3 halfway = normalize(l + view);

	float NdotL = dot(n, l);
	float4 diff = float4(1,1,1,1)*saturate(NdotL);
	float3 r = reflect(l, n);
	//float spec = float4(0.5,0.5,0.5,1)*pow(saturate(dot(v, r)), 50) * (NdotL > 0.0);
	float3 specular = pow(saturate(dot(n, halfway)), shininess) * (NdotL > 0.0);

	float3 output = (saturate(amb + diff) *col+ specular)* float4(1,1,1,1) ;

	return float4(output,1);



	
	//float lightrange = 25;
	//float3 N = normalize(Input.vNormal);
	//float d = distance(g_vLightDir , Input.vWorldPos);
	//float att = saturate(1-dot(d/lightrange,1/lightrange));
	////float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.tex );
	//float4 vDiffuse = float4(0.2,0,0,1);
	//float3 fakepos = float3(0,0,0);
	//float3 l =  (g_vLightDir - Input.vWorldPos);
	//float fLighting = saturate( dot( l, N ) );
	//fLighting = max( fLighting, g_fAmbient );
	//
	//return vDiffuse * fLighting ;

	/*float3 col = float3(1,0,0)*0.05;
	float3 N = normalize(Input.vNormal);
	float dist = length(g_vLightDir - Input.vWorldPos);
	float3 dir = normalize(g_vLightDir - Input.vWorldPos);
	float att = 1/dist;
	float3 lightcol = (1,1,1);
	col += lightcol * att*dot(N,dir);
	return float4(col,1);*/
}

