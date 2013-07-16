
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
#define Missle_launcher 0 //emit shoot missle particle
#define Missle 1 //emit smoke and turn to explosion when it dies
#define Explosion 2//Emit yellow to red particle, emite smoke when dies
#define Smoke   3 //smoke
#define Trial 4//trail
#define Min_launcher_emit_time  10 // minimum missle emit cooldown
#define Man_launcher_emit_time  30 // maximum missle emit cooldown
#define Missle_min_life 20 // minimum life of missle
#define Missle_max_life 30 // maximum life of missle
#define Missle_min_speed 10 //minmum speed of missle
#define Missle_max_speed 50 //max speed of missle
#define Explosion_life 10//life of explosion
#define smoke_life 20//life of smoke
#define Emit_explison_amount 30//amount of explosion particle 
#define trail_emit 0.3//cool down of trial emit
#define trail_life 5//life of trail

//VERTEX SHADER
VertexOut Vertex_Render(VertexIn vin)
{
	VertexOut vout;

	vout.pos = vin.pos;
	vout.radius = 2;
	vout.Type = vin.Type;

	if(vin.Type == Missle_launcher)
	{
		vout.color = float4(0,0,0,0);
	}
	else if(vin.Type == Missle)
	{
		vout.color = float4(0,0,1,1);
		//vout.color.z = clamp((vin.Timer / Flame_life ) * 1.2,0.0001,0.1);
		//vout.color.y = clamp((vin.Timer / Flame_life ) * 1.2,0.0001,0.1);
		//vout.color.x = clamp((vin.Timer / Flame_life ) * 0.6,0.0001,1);
		//vout.color.a = (vin.Timer / Flame_life ) * 0.5;
		vout.radius = 2;
	}
	else if(vin.Type == Explosion)
	{
		vout.color = float4(1,1,0,1);
		vout.color.y = (vin.Timer / Explosion_life ) * 0.5;
		//vout.color *= (vin.Timer / Spark_life ) * 2;
		vout.color.a = (vin.Timer / Explosion_life ) * 0.5;
		vout.radius = 2;
	}
	else if(vin.Type == Smoke)
	{
		vout.color = float4(0,0,0,1);
		//vout.color.y = (vin.Timer / Spark_life ) * 0.5;
		//vout.color *= (vin.Timer / Spark_life ) * 2;
		//vout.color.a = clamp((vin.Timer / Spark_life ) * 0.4,0.8,1);
		vout.radius = 2;
	}
	else if(vin.Type == Trial)
	{
		vout.color = float4(0.5,0.5,0.5,0.5);
		vout.color.a = (vin.Timer / trail_life ) * 0.1;
		vout.radius = 2;
	}
	return vout;
}
Texture2D particle_tex: register(t0); 
[maxvertexcount(4)]
void GS_Render(point VertexOut vin[1], inout TriangleStream<PixelIn> OutStream)
{
	PixelIn pin;
	
    float3 trial_positions[4] =
    {
        float3( -1, 1, 0 ),
        float3( 1, 1, 0 ),
        float3( -1, -1, 0 ),
        float3( 1, -1, 0 ),
    };

    float3 explosion_positions[4] =
    {
        float3( -8, 8, 0 ),
        float3( 8, 8, 0 ),
        float3( -8, -8, 0 ),
        float3( 8, -8, 0 ),
    };
	float3 missle_positions[4] = 
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
		
		if(vin[0].Type == Missle)
		{
			position = missle_positions[i]*vin[0].radius;
		}
		else if(vin[0].Type == Explosion)
		{
			position = explosion_positions[i]*vin[0].radius;
		}
		else if(vin[0].Type == Smoke)
		{
			position = explosion_positions[i]*vin[0].radius;
		}
		else if(vin[0].Type == Trial)
		{
			position = trial_positions[i]*vin[0].radius;
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
