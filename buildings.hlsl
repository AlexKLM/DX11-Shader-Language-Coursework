//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mWorldViewProjection;
	matrix		g_mWorld;
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir;
	float		g_fAmbient;
	float3		cam_pos;
	float		placeholder;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D 	g_style1 : register( t0 );
Texture2D 	g_style2 : register( t1 );
Texture2D 	g_style3 : register( t2 );
Texture2D 	g_style4 : register( t3 );
SamplerState g_samLinear : register( s0 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Pos  : POSITION;         //position
	float2 Tex	: TEXCOORD0;
	float3 Norm : NORMAL;

	float3 iPos : INSTANCEPOS;
	float3 height :INSTANCEHEIGHT;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 WorldPos :WORLDPOS;
    float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
	int type :TYPE;
};

//TUTORIAL CODE


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	float ground = step(0,input.Pos.y);
	float3 pos = input.Pos;
	pos += input.iPos;
	pos.y *= input.height.y *ground;
	float4 WorldPos = mul(pos,g_mWorld);
	output.WorldPos = WorldPos;
	output.Pos = mul(float4(pos,1),g_mWorldViewProjection);
	output.Tex = input.Tex;
	output.Tex.y = input.Tex.y*input.height.y;
	output.Normal = mul(input.Norm,g_mWorld);
	output.type = input.height.z;
	//output.Normal = input.height;
	return output;
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) :SV_TARGET
{
	/*float4 col = float4(1,1,1,1);
		col = float4(input.Normal,1);*/
	float4 vDiffuse;
	float3 N = normalize(input.Normal);
	 if(input.type == 0)
	{
		vDiffuse = g_style1.Sample(g_samLinear, input.Tex);
	}
	else if(input.type ==1)
	{
		vDiffuse = g_style2.Sample(g_samLinear, input.Tex);
	}
	else if(input.type ==2)
	{
		vDiffuse = g_style3.Sample(g_samLinear, input.Tex);
	}
	else
	{
		vDiffuse = g_style4.Sample(g_samLinear, input.Tex);
	}

	float3 latt = float3(0.0,0.005,0);
	float lrange = 1000;
	//return float4(N, 1);
	//float3 nom = float3(0,1,0);
	float3 finalcol = float3(0,0,0);
	float3 lightamb = float3(0.05,0.05,0.05);
	float3 ldiffuse = float3(1,1,1);
	float3 light2Vpos = g_vLightDir - input.WorldPos;
	float d = length(light2Vpos);
	float3 amb = vDiffuse * lightamb;
	//if(d < lrange)
	//{
	//	return float4(amb,1);
	//}

	//light2Vpos = normalize(light2Vpos);
	light2Vpos /= d; 
	float lightamt = dot(light2Vpos,N);

	//float howMuchLight = dot(light2Vpos, input.Normal);

	if(lightamt > 0)
	{
		finalcol += vDiffuse * ldiffuse *lightamt;
		finalcol /= latt[0] +(latt[1]*d)+(latt[2]*(d*d));
	}
	finalcol = saturate(finalcol + amb);
	return float4(finalcol, 1);
	//col.xyz *= 0.1;
	//return col;
}