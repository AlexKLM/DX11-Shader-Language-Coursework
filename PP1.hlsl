//--------------------------------------------------------------------------------------
// Globals


//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register( t0 );
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


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{

	float BloomThreshold = 0.5;
	float4 ori_color = g_txDiffuse.Sample( g_samLinear, input.Tex );
	//return ori_color;
	return saturate((ori_color - BloomThreshold) / (1 - BloomThreshold));

}
