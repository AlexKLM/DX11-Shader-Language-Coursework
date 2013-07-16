// Add the following to the effects file properties: Configuations Proporties->Custom Build Tool->General
// [Debug] Command Line:  fxc /Fc "fx/%(Filename).cod" /Od /Zi /T fx_5_0 /Fo "fx/%(Filename).fxo" "%(FullPath)"
// [Debug] Output: fx/%(Filename).fxo; fx/%(Filename).cod
// [Release] Command Line: fxc  /T fx_5_0 /Fo "fx/%(Filename).fxo" "%(FullPath)"
// [Release] Outputs: fx/%(Filename).fxo

// Global variables
cbuffer vars_for_calculate:register(b0) 
{
	float shouldemit; //act as boolean(1 or -1), also as a padding
	float3 emit_pos;
	float4 wind;
};

cbuffer global_delta:register(b1)
{
	float2 padding2;
	float GlobalTime;
	float DeltaTime;
};

SamplerState g_samPoint : register(s0);



// Shaders
struct VertexIn {
    float3 pos : POSITION; 
    float3 vel : NORMAL;
	float Timer : TIMER;
	//float angle : ANGLE;
	uint Type : TYPE;
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
VertexIn VSPassthrough(VertexIn vin) //do nothin, only pass data to GS
{
	return vin;
}

Texture1D ramdom_tex: register(t0);
//UTILITY METHODS
float3 RandomDir(float fOffset) //use random texture to create a random direction
{
    float tCoord = (GlobalTime + fOffset) / 300.0;
    return ramdom_tex.SampleLevel( g_samPoint, tCoord, 0 );
}


//GEO SHADERS
[maxvertexcount(128)]
void GS_Calc_Emitter(VertexIn vin, inout PointStream<VertexIn> OutStream)
{

	
	//vin.pos = emit_pos;
	if(vin.Timer <= 0)
	{
		VertexIn Flame_p;
		VertexIn Spark_p;
		for(int i=0; i < Emit_amount_flame ; i++)
		{
			float3 vel =  RandomDir( vin.Type );
			vel.y = clamp(vel.y,50,10);
			//if(vel.y <0)
			//{
			//vel.y *= -1;
			//}
			vel.x = clamp(vel.x,-1,1);
			vel.z = clamp(vel.z,-1,1);
			Flame_p.pos = vin.pos;
			Flame_p.vel = vel;
			Flame_p.Timer = Flame_life;
			//water_drop.angle = vin.angle;
			Flame_p.Type = Flame;
			OutStream.Append(Flame_p);
		}
		for(int i = 0; i < Emit_amount_sparks; i++)
		{
			float3 vel = RandomDir( vin.Type ) ;
			vel.y = clamp(vel.y,10,13);
			vel.x = clamp(vel.x,-1,1);
			vel.z = clamp(vel.z,-1,1);
			Spark_p.pos = vin.pos;
			Spark_p.vel = vel;
			Spark_p.Timer = Spark_life;
			Spark_p.Type = Sparks;
			OutStream.Append(Spark_p);
		}
		vin.Timer = Emit_time;
	}
	else
	{
		vin.Timer -= DeltaTime;
	}
	OutStream.Append(vin);
}
[maxvertexcount(128)]
void GS_Calc_Flame(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
    if(vin.Timer > 0)//only calculate particle if still alive 
	{
		float3 windvel = wind*5;
		vin.pos += vin.vel*DeltaTime;
		vin.pos.xz += windvel.xz*DeltaTime;;
		//vin.vel += gravity*DeltaTime;
		vin.Timer -= DeltaTime;
		OutStream.Append(vin); 
	}

}
[maxvertexcount(128)]
void GS_Calc_Sparks(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
	if(vin.Timer > 0)
	{
		float3 windvel = wind*5;
		vin.pos += vin.vel*DeltaTime;
		vin.pos.xz += windvel.xz*DeltaTime;;
		//float3 vel = normalize( RandomDir( vin.Type ) );
		//if(vel.y < 0)
		//{
		//	vel.y *= -1;
		//}
		//vin.vel = vel;
		vin.Timer -= DeltaTime;
		OutStream.Append(vin);
	}
}

[maxvertexcount(128)]
void GScalculate_Particles_main(point VertexIn vin[1], inout PointStream<VertexIn> OutStream)
{
	if(vin[0].Type == Emitter)
	{
		GS_Calc_Emitter(vin[0],OutStream);
	}
	else if(vin[0].Type == Flame)
	{
		GS_Calc_Flame(vin[0],OutStream);
	}
	else if(vin[0].Type == Sparks)
	{
		GS_Calc_Sparks(vin[0],OutStream);
	}
}
