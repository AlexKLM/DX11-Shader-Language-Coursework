//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mViewProjection;
	matrix		g_mWorld;
	float3		g_mcam_pos;
	float		g_mplaceholder;
	float4		g_vObjectColor;
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir;
	float		g_fAmbient;
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
	 float2 tex: TEXCOORD0;
};

struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition	: POSITION;
	 float2 tex: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_CONTROL_POINT_OUTPUT QuadTess_VS( VS_CONTROL_POINT_INPUT Input )
{
    VS_CONTROL_POINT_OUTPUT Output;

    Output.vPosition = Input.vPosition;
	Output.tex = Input.tex;

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
	 float2 tex: TEXCOORD0;
};


HS_CONSTANT_DATA_OUTPUT BezierConstantHS( InputPatch<VS_CONTROL_POINT_OUTPUT, INPUT_PATCH_SIZE> ip,
                                          uint PatchID : SV_PrimitiveID )
{    
    HS_CONSTANT_DATA_OUTPUT Output;

    float TessAmount = 100;//dynamic tess amount to replace 8

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
	Output.tex = p[i].tex;
    return Output;
}

//----------------------------------------------------------------------------------
//Domain Shader
//----------------------------------------------------------------------------------
struct DS_OUTPUT
{
    float4 vPosition        : SV_POSITION;
	 float2 tex: TEXCOORD0;
    float3 vWorldPos        : WORLDPOS;
    float3 vNormal            : NORMAL;
};
float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4( invT * invT * invT,
                   3.0f * t * invT * invT,
                   3.0f * t * t * invT,
                   t * t * t );
}

//--------------------------------------------------------------------------------------
float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4( -3 * invT * invT,
                   3 * invT * invT - 6 * t * invT,
                   6 * t * invT - 3 * t * t,
                   3 * t * t );
}

//--------------------------------------------------------------------------------------
float3 EvaluateBezier( const OutputPatch<HS_OUTPUT, OUTPUT_PATCH_SIZE> bezpatch,
                       float4 BasisU,
                       float4 BasisV )
{
    float3 Value = float3(0,0,0);
    Value  = BasisV.x * ( bezpatch[0].vPosition * BasisU.x + bezpatch[1].vPosition * BasisU.y + bezpatch[2].vPosition * BasisU.z + bezpatch[3].vPosition * BasisU.w );
    Value += BasisV.y * ( bezpatch[4].vPosition * BasisU.x + bezpatch[5].vPosition * BasisU.y + bezpatch[6].vPosition * BasisU.z + bezpatch[7].vPosition * BasisU.w );
    Value += BasisV.z * ( bezpatch[8].vPosition * BasisU.x + bezpatch[9].vPosition * BasisU.y + bezpatch[10].vPosition * BasisU.z + bezpatch[11].vPosition * BasisU.w );
    Value += BasisV.w * ( bezpatch[12].vPosition * BasisU.x + bezpatch[13].vPosition * BasisU.y + bezpatch[14].vPosition * BasisU.z + bezpatch[15].vPosition * BasisU.w );

    return Value;
}

[domain("quad")]
DS_OUTPUT QuadTess_DS( HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad )
{
	DS_OUTPUT Output;
	float3 verticalPos1 = lerp(quad[0].vPosition,quad[1].vPosition,UV.y);
	float3 verticalPos2 = lerp(quad[3].vPosition,quad[2].vPosition,UV.y);
	float3 finalPos = lerp(verticalPos1,verticalPos2,UV.x);


	float2 texPos1 = lerp(quad[0].tex,quad[1].tex,UV.y);
	float2 texPos2 = lerp(quad[3].tex,quad[2].tex,UV.y);
	float2 finalTex = lerp(texPos1,texPos2,UV.x);
	float3 normal = normalize(cross((quad[0].vPosition-quad[1].vPosition),(quad[1].vPosition-quad[2].vPosition)));
	Output.tex = UV;

	float3 pos1 = quad[0].vPosition;
	float3 pos2 = quad[1].vPosition;
	float3 pos3 = quad[2].vPosition;
	float3 pos4 = quad[3].vPosition;

	float3 test = pos1 + pos2 + pos3 + pos4;
	////float3 WorldPos = EvaluateBezier( bezpatch, BasisU, BasisV );
 //   float3 Tangent = EvaluateBezier( quad, dBasisU, BasisV );
 //   float3 BiTangent = EvaluateBezier( quad, BasisU, dBasisV );
 //   float3 Norm = normalize( cross( Tangent, BiTangent ) );

	float3 Norm = cross(verticalPos2,verticalPos1);
	
	float4 WorldPos = mul(float4(finalPos,1),g_mWorld);

	
   Output.vWorldPos =  WorldPos;

	
	Output.vPosition = mul( WorldPos, g_mViewProjection );
	Norm = normalize(finalPos);
	
	//Norm = cross(verticalPos2,verticalPos1); 
	//Output.vNormal = float3(0,0,1);
	Output.vNormal = mul(normal,(float3x3)g_mWorld);
	//Output.vNormal = float4(normal,1);
	//Output.vNormal = WorldPos.xyz;
	return Output;
}






float4 QuadTess_PS( DS_OUTPUT Input ) : SV_TARGET
{
	float3 N = normalize(Input.vNormal);
	//return float4(N,1);
	//return float4(N,1);
	float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.tex );
	//float4 vDiffuse = float4(1,0,0,1);
	//float3 ldir = normalize(g_vLightDir - Input.vWorldPos);
	////float fLighting = saturate( dot( ldir, N ) );
	//fLighting = max( fLighting, g_fAmbient );
	

	float3 latt = float3(0.0,0.005,0); //x = constant, y = linear, z = exponential
	float lrange = 1000;
	//return float4(N, 1);
	//float3 nom = float3(0,1,0);
	float3 finalcol = float3(0,0,0);
	float3 lightamb = float3(0.05,0.05,0.05);
	float3 ldiffuse = float3(1,1,1);
	float3 light2Vpos = g_vLightDir - Input.vWorldPos;
	float d = length(light2Vpos);
	float3 amb = vDiffuse * lightamb;
	//if(d > lrange)
	//{
	//	return float4(amb,1);
	//}
	//light2Vpos = normalize(light2Vpos);
	light2Vpos /= d; 
	float lightamt = dot(light2Vpos,N);

	if(lightamt > 0)
	{
		finalcol += vDiffuse * ldiffuse *lightamt;
		finalcol /= latt.x +(latt.y*d)+(latt.z*(d*d));
	}
	finalcol = saturate(finalcol + amb);
	return float4(finalcol, 1);

	//return vDiffuse * fLighting;
}

