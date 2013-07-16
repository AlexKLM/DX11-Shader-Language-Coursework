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
cbuffer vars_for_render:register(b2)
{
	float4x4 worldViewProj;
	float4x4 InvViewMx;
}

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
};


BlendState AdditiveBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

cbuffer cbImmutable 
{
    float3 spark_positions[4] =
    {
        float3( -0.005, 0.005, 0 ),
        float3( 0.005, 0.005, 0 ),
        float3( -0.005, -0.005, 0 ),
        float3( 0.005, -0.005, 0 ),
    };
	float3 flame_positions[4] = 
	{
        float3( -0.15, 0.15, 0 ),
        float3( 0.15, 0.15, 0 ),
        float3( -0.15, -0.15, 0 ),
        float3( 0.15, -0.15, 0 ),
    };

    float2 g_texcoords[4] = 
    { 
        float2(0,1), 
        float2(1,1),
        float2(0,0),
        float2(1,0),
    };
};


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
#define Flame_life 2 // life of water particle
#define Emit_amount_flame 1 //number of Flame particle emitted
#define Spark_life 1.5 //life of the spark particle
#define Emit_amount_sparks 1 //num of spark emitted

//VERTEX SHADER
VertexIn VSPassthrough(VertexIn vin) //do nothin, only pass data to GS
{
	return vin;
}

VertexOut Vertex_Render(VertexIn vin)
{
	VertexOut vout;

	vout.pos = vin.pos;
	vout.radius = 2;
	vout.Type = vin.Type;

	if(vin.Type == Emitter)
	{
		vout.color = float4(1,0,0,0);
	}
	else if(vin.Type == Flame)
	{
		vout.color = float4(1,1,0,1);
		vout.color.y = (vin.Timer / Flame_life ) * 0.6;
		vout.color *= (vin.Timer / Flame_life )* 1.5;
	}
	else if(vin.Type == Sparks)
	{
		vout.color = float4(0.5,0.5,0.5,1);
		vout.color *= (vin.Timer / Spark_life ) * 2;
	}
	return vout;
}

Texture1D ramdom_tex: register(t0);
//UTILITY METHODS
float3 RandomDir(float fOffset) //use random texture to create a random direction
{
    float tCoord = (GlobalTime + fOffset) / 300.0;
    return ramdom_tex.SampleLevel( g_samPoint, tCoord, 0 );
}


//GEO SHADERS
void GS_Calc_Emitter(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
	//vin.pos = emit_pos;
	if(vin.Timer <= 0 && shouldemit >0)
	{
		VertexIn Flame_p;
		VertexIn Spark_p;
		for(int i=0; i < Emit_amount_flame ; i++)
		{
			float3 vel = normalize( RandomDir( vin.Type ) );
			vel.y = clamp(vel.y,0.5,1);
			//if(vel.y <0)
			//{
			//vel.y *= -1;
			//}
			vel.x = clamp(vel.x,-0.1,0.1);
			vel.z = clamp(vel.z,-0.1,0.1);
			Flame_p.pos = vin.pos;
			Flame_p.vel = vel;
			Flame_p.Timer = Flame_life;
			//water_drop.angle = vin.angle;
			Flame_p.Type = Flame;
			OutStream.Append(Flame_p);
		}
		for(int i = 0; i < Emit_amount_sparks; i++)
		{
			float3 vel = normalize( RandomDir( vin.Type ) );
			vel.y = clamp(vel.y,1,1.3);
			vel.x = clamp(vel.x,-0.2,0.2);
			vel.z = clamp(vel.z,-0.2,0.2);
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

void GS_Calc_Flame(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
    if(vin.Timer > 0)//only calculate particle if still alive 
	{
		vin.pos += vin.vel*DeltaTime;
		//vin.vel += gravity*DeltaTime;
		vin.Timer -= DeltaTime;
		OutStream.Append(vin); 
	}

}

void GS_Calc_Sparks(VertexIn vin, inout PointStream<VertexIn> OutStream)
{
	if(vin.Timer > 0)
	{
		vin.pos += vin.vel*DeltaTime;
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



[maxvertexcount(4)]
void GS_Render(point VertexOut vin[1], inout TriangleStream<PixelIn> OutStream)
{
	PixelIn pin;

	for(int i = 0; i < 4; i ++)
	{
		float3 position;
		if(vin[0].Type == Sparks)
		{
			position = flame_positions[i]*vin[0].radius;
		}
		else if(vin[0].Type == Flame)
		{
			position = flame_positions[i]*vin[0].radius;
		}
		position = mul(position,(float3x3)InvViewMx) + vin[0].pos;
		pin.pos = mul(float4(position,1.0), worldViewProj);

		pin.color = vin[0].color;
		pin.tex = g_texcoords[i];
		OutStream.Append(pin);
	}
}

Texture2D particle_tex: register(t1); 
//Pixel Shader
float4 PS_Render(PixelIn pin) : SV_TARGET
{
	return particle_tex.Sample( g_samLinear, pin.tex ) * pin.color;
	//return pin.color;
}


// Techniques
technique10 RenderScene {
	pass p0
	{
		SetVertexShader(CompileShader(vs_4_0,Vertex_Render()));
		SetGeometryShader(CompileShader(gs_4_0,GS_Render()));
		SetPixelShader(CompileShader(ps_4_0,PS_Render()));
		SetBlendState( AdditiveBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
	}

}

GeometryShader geo_shade  = ConstructGSWithSO(CompileShader(gs_4_0,GScalculate_Particles_main()),"POSITION.xyz;NORMAL.xyz;TIMER.x;TYPE.x;");
technique10 Calculate_Particles{

	pass p0
	{
		SetVertexShader(CompileShader(vs_4_0,VSPassthrough()));
		SetGeometryShader(geo_shade);
		SetPixelShader(NULL);

		SetDepthStencilState( DisableDepth, 0 );
	}

}

