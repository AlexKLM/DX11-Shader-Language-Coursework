
cbuffer vars_for_render:register(b0)
{
	float4x4 View;
	float4x4 Proj;
}

SamplerState g_samLinear:register(s0);



// Shaders
struct VertexIn {
    float3 pos : POSITION; 
    float3 vel : NORMAL;
	float Timer : TIMER;
	//float angle : ANGLE;
	uint Type : TYPE;
};

struct VertexOut { 
    float3 pos : POSITION; 
    float4 color : COLOR; 
	float radius : RADIUS;
	uint Type :TYPE;
};

struct PixelIn{
	float4 pos : SV_Position;
	float2 tex : TEXTURE0;
	float4 color : COLOR0;
};
#define Emitter 0 //emit other particles
#define Flame 1 //water spary particles
#define Sparks 2//spark particles
#define Emit_time  0.3 //cooldown between emits
#define Flame_life 20 // life of water particle
#define Emit_amount_flame 1 //number of Flame particle emitted
#define Spark_life 10.5 //life of the spark particle
#define Emit_amount_sparks 1 //num of spark emitted

//VERTEX SHADER
VertexOut Vertex_Render(VertexIn vin)
{
	//PixelIn output = (PixelIn)0;
	//float2 pixelPos=sign(vin.pos.xy);
	//output.pos =float4(pixelPos, 0, 1);
	//output.tex = pixelPos;
	//output.color = float4(1,0,0,1);
	//return output;

	VertexOut vout;

	vout.pos = vin.pos;
	vout.radius = 2;
	vout.Type = vin.Type;

	if(vin.Type == Emitter)
	{
		vout.color = float4(0,0,0,0);
	}
	else if(vin.Type == Flame)
	{
		vout.color = float4(1,0,0,0.8);
		vout.color.z = clamp((vin.Timer / Flame_life ) * 1.2,0.0001,0.1);
		vout.color.y = clamp((vin.Timer / Flame_life ) * 1.2,0.0001,0.1);
		vout.color.x = clamp((vin.Timer / Flame_life ) * 0.6,0.0001,1);
		vout.color.a = (vin.Timer / Flame_life ) * 0.5;
	}
	else if(vin.Type == Sparks)
	{
		vout.color = float4(1,1,0,1);
		vout.color.y = (vin.Timer / Spark_life ) * 0.5;
		vout.color *= (vin.Timer / Spark_life ) * 2;
		vout.color.a = clamp((vin.Timer / Spark_life ) * 0.4,0.8,1);
	}
	return vout;
}
Texture2D particle_tex: register(t0); 
[maxvertexcount(4)]
void GS_Render(point VertexOut vin[1], inout TriangleStream<PixelIn> OutStream)
{
	PixelIn pin;


    float3 spark_positions[4] =
    {
        float3( -0.5, 0.5, 0 ),
        float3( 0.5, 0.5, 0 ),
        float3( -0.5, -0.5, 0 ),
        float3( 0.5, -0.5, 0 ),
    };
	float3 flame_positions[4] = 
	{
        float3( -10, 10, 0 ),
        float3( 10, 10, 0 ),
        float3( -10, -10, 0 ),
        float3( 10, -10, 0 ),
    };

    float2 g_texcoords[4] = 
    { 
        float2(0,1), 
        float2(1,1),
        float2(0,0),
        float2(1,0),
    };
	float3x3 invView=(float3x3)transpose(View);
	float3 position;
	for(int i = 0; i < 4; i ++)
	{
		
		if(vin[0].Type == Sparks)
		{
			position = flame_positions[i]*vin[0].radius;
		}
		else if(vin[0].Type == Flame)
		{
			position = flame_positions[i]*vin[0].radius;
		}
		position.xyz=mul(position.xyz, invView);
		position +=vin[0].pos;
		//position = mul(position,(float3x3)InvViewMx) + vin[0].pos;
		//pin.pos = mul(float4(position,1.0), worldViewProj);
		pin.pos = mul(float4(position,1), View);
		pin.pos = mul(pin.pos, Proj);
		pin.color = vin[0].color;
		pin.tex = g_texcoords[i];
		OutStream.Append(pin);
	}
}


//Pixel Shader
float4 PS_Render(PixelIn pin) : SV_TARGET
{
	return particle_tex.Sample( g_samLinear, pin.tex ) * pin.color;
	//return pin.color;
}
