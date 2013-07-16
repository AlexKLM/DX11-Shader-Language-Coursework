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
	//return g_txDiffuse.Sample(g_samLinear, input.Tex);
	float4 output = 0;
    
    for (int i = 0; i < 15; i++)
    { //output.x += sampleW[i]*10000;
       output += g_txDiffuse.Sample(g_samLinear, input.Tex + blurstuff[i].xy) * blurstuff[i].z ;
    }
    
    return output;

}
