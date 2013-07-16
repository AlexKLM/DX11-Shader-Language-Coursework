//--------------------------------------------------------------------------------------
// Globals
cbuffer c_buffer : register(b0)
{
	float4 blurstuff[15];
	/*float2 sampleOff[15];
	float sampleW[15];
	float placeholder[15];*/
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txbloom : register( t0 );
Texture2D	g_txbase : register( t1 );
SamplerState g_samLinear : register( s0 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD1;
};

//TUTORIAL CODE


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( uint id : SV_VertexID )
{

	PS_INPUT Output;
    Output.Tex = float2((id << 1) & 2, id & 2);
    Output.Pos = float4(Output.Tex * float2(2,-2) + float2(-1,1), 0, 1);
    //return Output;

//	PS_INPUT output = (PS_INPUT)0;
	float2 pixelPos=sign(Output.Pos.xy);
	Output.Pos =float4(pixelPos, 0, 1);
	Output.Pos.y *=-1;
	Output.Tex = clamp(pixelPos,0,1);
	return Output;
}


float4 AdjustSaturation(float4 color, float saturation) //code from a sample
{
    // The constants 0.3, 0.59, and 0.11 are chosen because the
    // human eye is more sensitive to green light, and less to blue.
    float grey = dot(color, float3(0.3, 0.59, 0.11));

    return lerp(grey, color, saturation);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	//return g_txDiffuse.Sample(g_samLinear, input.Tex);
	float BloomIntensity = 1.25;
	float BaseIntensity = 1;

	float BloomSaturation = 1;
	float BaseSaturation = 1;

	float4 bloom = g_txbloom.Sample(g_samLinear, input.Tex);
	float4 base = g_txbase.Sample(g_samLinear, input.Tex);

	//modify color with sat and intensity
	bloom = AdjustSaturation(bloom, BloomSaturation) * BloomIntensity;
    base = AdjustSaturation(base, BaseSaturation) * BaseIntensity;

	base *= (1 - saturate(bloom));
	//return base;
	return bloom+base;
    
}
