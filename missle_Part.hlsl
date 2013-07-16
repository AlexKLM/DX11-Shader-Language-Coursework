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
	float4 gravity;
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
	uint Type : TYPE;
};


#define Missle_launcher 0 //emit shoot missle particle
#define Missle 1 //emit smoke and turn to explosion when it dies
#define Explosion 2//Emit yellow to red particle, emite smoke when dies
#define Smoke   3 //smoke
#define Trial 4//trail
#define Min_launcher_emit_time  10 // minimum missle emit cooldown
#define Max_launcher_emit_time  30 // maximum missle emit cooldown
#define Missle_min_life 10 // minimum life of missle
#define Missle_max_life 15 // maximum life of missle
#define Missle_min_speed 10 //minmum speed of missle
#define Missle_max_speed 20 //max speed of missle
#define Explosion_life 10//life of explosion
#define smoke_life 20//life of smoke
#define Emit_explison_amount 30//amount of explosion particle 
#define trail_emit 0.3//cool down of trial emit
#define trail_life 5//life of trail

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

[maxvertexcount(128)]
void GS_Calc_Launcher(VertexIn vin,inout PointStream<VertexIn> OutStream)
{
	//vin.pos = emit_pos;
	if(vin.Timer <= 0)
	{
		VertexIn pMissle;
		
			
			float3 vel = normalize(RandomDir( vin.vel.y ))*50;
			//vel.y = clamp(vel.y,50,10);
			if(vel.y <0)
			{
				vel.y*=-1;
			}
			/*if(vel.x <0)
			{
				vel.x = clamp(vel.x,-Missle_max_speed,-Missle_min_speed);
			}
			else
			{
				vel.x = clamp(vel.x,Missle_min_speed,Missle_max_speed);
			}
			if(vel.z <0)
			{
				vel.z = clamp(vel.x,-Missle_max_speed,-Missle_min_speed);
			}
			else
			{
				vel.z = clamp(vel.x,Missle_min_speed,Missle_max_speed);
			}*/
			
			pMissle.pos = vin.pos;
			pMissle.vel = vel;

			pMissle.Timer = clamp(RandomDir( vin.vel.y ),Missle_min_life,Missle_max_life);
			//water_drop.angle = vin.angle;
			pMissle.Type = Missle;
			//OutStream.Append(RandomDir( vin.Type +vin.vel.y*0.01 ),Min_launcher_emit_time,Max_launcher_emit_time);
		OutStream.Append(pMissle);
		vin.Timer = clamp(RandomDir( vin.vel.y),Min_launcher_emit_time,Max_launcher_emit_time);
		
	}
	else
	{
		vin.Timer -= DeltaTime;
	}
	OutStream.Append(vin);
}

//GEO SHADERS
[maxvertexcount(128)]
void GS_Calc_Missle(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
    if(vin.Timer > 0)//only calculate particle if still alive 
	{
		if(shouldemit >0)//use cbuffer value
		{
			VertexIn trial_smoke;
			trial_smoke.pos = vin.pos;
			trial_smoke.vel = float3(0,0,0);
			trial_smoke.Timer = trail_life;
			trial_smoke.Type = Trial;
			OutStream.Append(trial_smoke);
		}
		vin.pos += vin.vel*DeltaTime;
		//vin.vel += gravity*DeltaTime;
		vin.Timer -= DeltaTime;
		OutStream.Append(vin); 
	}
	else
	{
		VertexIn pexplostion;
		//float offset = 0;
		for(int i = 0; i < Emit_explison_amount; i++)
		{
			float3 vel =  normalize(RandomDir( vin.Type+ i )) * 0.05 ;
			pexplostion.pos = vin.pos;
			pexplostion.vel = vel;
			//pexplostion.vel = vel;
			//pexplostion.vel.x = clamp(vel, -50,50);
			//pexplostion.vel.y = clamp(vel, -50,50);
			//pexplostion.vel.z = clamp(vel, -50,50);
			pexplostion.Timer = Explosion_life;
			pexplostion.Type = Explosion;
			//OutStream.Append(pexplostion);
			OutStream.Append(pexplostion); 
		}
	}

}
[maxvertexcount(128)]
void GS_Calc_Trial(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
	if(vin.Timer > 0)
	{
		vin.Timer -= DeltaTime;
		OutStream.Append(vin);
	}
}

[maxvertexcount(128)]
void GS_Calc_Explosion(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
	if(vin.Timer > 0)
	{
		vin.pos += vin.vel;
		vin.Timer -= DeltaTime;
		OutStream.Append(vin);
	}
	else
	{
		VertexIn smoke;
		smoke.pos = vin.pos;
		smoke.vel = vin.vel;
		smoke.Timer = smoke_life;
		smoke.Type = Smoke;
		OutStream.Append(smoke);
	}
}
[maxvertexcount(128)]
void GS_Calc_Smoke(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
	if(vin.Timer > 0)
	{
		vin.pos += vin.vel*DeltaTime;
		vin.Timer -= DeltaTime;
		OutStream.Append(vin);
	}
}




[maxvertexcount(128)]
void GScalculate_Particles_main(point VertexIn vin[1], inout PointStream<VertexIn> OutStream)
{
	if(vin[0].Type == Missle_launcher)
	{
		GS_Calc_Launcher(vin[0],OutStream);
	}
	else if(vin[0].Type == Missle)
	{
		GS_Calc_Missle(vin[0],OutStream);
	}
	else if(vin[0].Type == Explosion)
	{
		GS_Calc_Explosion(vin[0],OutStream);
	}
	else if(vin[0].Type == Smoke)
	{
		GS_Calc_Smoke(vin[0],OutStream);
	}
	else if(vin[0].Type == Trial)
	{
		GS_Calc_Trial(vin[0],OutStream);
	}
}
