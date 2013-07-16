//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mViewProjection;
	matrix		g_mWorld;
	float3		g_mcam_pos;
	float		use_distance;
	float4		displacement_lvl; //x is displacement lvl ,y is tess lvl(if not distance based)
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
Texture2D	g_txGrass : register( t1 );
Texture2D	g_txSands : register( t2 );
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
	float3 vPosition	: POSITION0;
	float3 wPos :POSITION1;
	 float2 tex: TEXCOORD0;
	// float TessDistance : VERTEXDISTANCEFACTOR;//distance modifyer
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_CONTROL_POINT_OUTPUT QuadTess_VS( VS_CONTROL_POINT_INPUT Input )
{

	
    VS_CONTROL_POINT_OUTPUT Output;

    Output.vPosition = Input.vPosition;
	Output.tex = Input.tex;
	Output.wPos = mul(Input.vPosition,g_mWorld); 
	//float Distance = distance( wPos.xyz, g_mcam_pos.xyz );//distance between vertex point and camera
	//Output.TessDistance = 1.0 - clamp( ( ( Distance - MinDist ) / ( MaxDist - MinDist ) ), 
 //                                            1, 1 -20);
	//Output.TessDistance = 8;
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
	float3 wPos	: POSITION;
	 float2 tex: TEXCOORD0;
};


HS_CONSTANT_DATA_OUTPUT BezierConstantHS( InputPatch<VS_CONTROL_POINT_OUTPUT, INPUT_PATCH_SIZE> ip,
                                          uint PatchID : SV_PrimitiveID )
{    
    HS_CONSTANT_DATA_OUTPUT Output;
	float MinDist = 0;
    float MaxDist = 5000;
	float mintess = 8;
	float maxtess = 100;
	//float TessAmount = 4;
	if(use_distance == 1)
	{
	float distanceRange = MaxDist - MinDist;
	float tess0 = lerp(mintess, maxtess, (1.0f - (saturate((distance(g_mcam_pos, ip[0].vPosition) - MinDist) / distanceRange))));
	float tess1 = lerp(mintess, maxtess, (1.0f - (saturate((distance(g_mcam_pos, ip[1].vPosition) - MinDist) / distanceRange))));
	float tess2 = lerp(mintess, maxtess, (1.0f - (saturate((distance(g_mcam_pos, ip[2].vPosition) - MinDist) / distanceRange))));
	float tess3 = lerp(mintess, maxtess, (1.0f - (saturate((distance(g_mcam_pos, ip[3].vPosition) - MinDist) / distanceRange))));


	Output.Edges[0] = min(tess0, tess3);
	Output.Edges[1] = min(tess0, tess1);
	Output.Edges[2] = min(tess1, tess2);
	Output.Edges[3] = min(tess2, tess3);

    //Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount; 
	float overallminTess = min(Output.Edges[1], Output.Edges[3]);

    Output.Inside[0] = Output.Inside[1] = overallminTess;
	}
	else
	{
		float TessAmount = displacement_lvl.y;
		Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount; 
		 Output.Inside[0] = Output.Inside[1] = TessAmount;
	}

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
	Output.wPos = p[i].wPos;
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
	float2 Height_weight :HEIGHT;
};


//procedural displacement
float HightMap(float2 uv){ 
	float2 st=uv-0.5; 
	float Bx=smoothstep(0.3, 0.5, uv.x)-smoothstep(0.5, 0.7, uv.x); 
	float By=smoothstep(0.3, 0.5, uv.y)-smoothstep(0.5, 0.7, uv.y); 
	return Bx*By; 
}

float3 convert_to_normal(float2 uv, float3 normal)
{
	int heightMapSizeY = 1024;
	int heightMapSizeX = 1024;
	float bumpHeightScale = 50;
	float3 norm = normal;
	float me = g_txDiffuse.Sample( g_samLinear, uv ).x;
	float n =  g_txDiffuse.Sample( g_samLinear,float2(uv.x,uv.y+1.0/heightMapSizeY)).x;
	float s = g_txDiffuse.Sample( g_samLinear,float2(uv.x,uv.y-1.0/heightMapSizeY)).x;
	float e = g_txDiffuse.Sample( g_samLinear,float2(uv.x+1.0/heightMapSizeX,uv.y)).x;
	float w = g_txDiffuse.Sample( g_samLinear,float2(uv.x-1.0/heightMapSizeX,uv.y)).x;              
	

	float3 temp = norm; //a temporary vector that is not parallel to norm
	if(norm.x==1)
		temp.y+=0.5;
	else
		temp.x+=0.5;

	//form a basis with norm being one of the axes:
	float3 perp1 = normalize(cross(norm,temp));
	float3 perp2 = normalize(cross(norm,perp1));

	//use the basis to move the normal in its own space by the offset        
	float3 normalOffset = -bumpHeightScale*(((n-me)-(s-me))*perp1 + ((e-me)-(w-me))*perp2);
	norm += normalOffset;
	norm = normalize(norm);
	return norm;
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
	float3 normal = cross((quad[0].vPosition-quad[1].vPosition),(quad[1].vPosition-quad[2].vPosition));
	Output.tex = UV;

	float3 Norm = cross(verticalPos2,verticalPos1);
	//Norm = float3(0,1,0);
	float3 WorldPos = finalPos;
	//float4 heightlevel = g_txDiffuse.SampleLevel( g_samLinear, UV,3)*0.005;
	float vNormalHeight = g_txDiffuse.SampleLevel( g_samLinear, UV, 0) * (displacement_lvl.x*0.00005);
	WorldPos += (normal * vNormalHeight);
	Output.vWorldPos =  WorldPos;
	float2 weight;
	weight.x = saturate(  abs((normal * vNormalHeight).y -0.0005) / 0.0001f);
	weight.y = saturate( 1-  abs((normal * vNormalHeight).y - 0.00005) / 0.0001f);
	float total_weight = weight.x + weight.y;
	Output.Height_weight = weight/total_weight;

	Output.vPosition = mul( float4(WorldPos,1), g_mViewProjection );
	//Norm = Norm - finalPos;
	
	//Norm = cross(verticalPos2,verticalPos1); 
	//Output.vNormal = Norm;
	//Output.vNormal = convert_to_normal(UV,normal);
	Output.vNormal = normal;
	return Output;
}






float4 QuadTess_PS( DS_OUTPUT Input ) : SV_TARGET
{
	////float3 N = normalize(Input.vNormal);
	float3 N  = convert_to_normal(Input.tex,normalize(Input.vNormal));//read the texel around the targeted texel to modify the normal
	//float4 vDiffuse = float4(0.139,lerp(0.5,0.8,Input.vWorldPos.y),0.19,1)*0.2;

	//float4 vDiffuse = float4(clamp(lerp(1.39*0.1,1.39,Input.vWorldPos.y),1.39*0.1,1.39),clamp(lerp(0.69 *0.1,0.69,Input.vWorldPos.y),0.69 *0.1,0.69),clamp(lerp(0.19 *0.1,0.19,Input.vWorldPos.y),0.19 *0.1,0.19),1)*0.05;
	//return vDiffuse;

	float3 up = float3(0,1,0);
   float dp = dot(up,normalize(Input.vNormal));
   dp = clamp(4*dp-2.5,0,1);
   float4 grass = g_txGrass.Sample( g_samLinear, Input.tex );
   float4 sand = g_txSands.Sample( g_samLinear, Input.tex );
   //return float4(Input.Height_weight.y,Input.Height_weight.y,Input.Height_weight.y,1);
   //return grass;
	float4 vDiffuse  = grass * Input.Height_weight.x + sand * Input.Height_weight.y;
   //return lerp(sand,grass,dp);

	////float4 vDiffuse = float4(1,0,0,1);
	//float3 ldir = normalize(g_vLightDir - Input.vWorldPos);
	//float fLighting = saturate( dot( -ldir, N ) );
	//fLighting = max( fLighting, g_fAmbient );
	//
	//return vDiffuse * fLighting;
	//float3 lightpos= float3(g_vLightDir.x,g_vLightDir.y,-g_vLightDir.z);
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
}

