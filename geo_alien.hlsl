//--------------------------------------------------------------------------------------
// File: Tutorial13.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D g_txDiffuse:register(t0);
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

TextureCube g_txEnvMap;
SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};



cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 time; //x = time, y = explode value for debug z = explode
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir;
	float		g_fAmbient;
	float3 Eye;
	float padding;
};

struct VS_INPUT
{
    float3 Pos          : POSITION;         
    float3 Norm         : NORMAL;           
    float2 Tex          : TEXCOORD0;        
};

struct VS_explode_input
{
	float3 pos : POSITION; 
    float3 vel : NORMAL;
	float Timer : TIMER;
};

struct VS_explode_out { 
    float3 pos : POSITION; 
    float4 color : COLOR; 
};

struct PS_explode
{
	float4 pos : SV_Position;
	float2 tex : TEXTURE0;
	float4 color : COLOR0;
};

struct GSPS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 PosW : POSITION0; 
	float4 PosV : POSITION1;
    float3 Norm : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};
#define life 20 
//--------------------------------------------------------------------------------------
// DepthStates
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
VS_explode_input VS_pass(VS_explode_input input)
{
	return input;
}

VS_explode_out VS_explode( VS_explode_input input)
{
	VS_explode_out output;
	output.pos = input.pos;
	output.color = float4(1,1,1,1);
	output.color.z = clamp((input.Timer / life ) * 1.2,0.0001,0.1);
	output.color.y = clamp((input.Timer / life ) * 1.2,0.0001,1);
	output.color.x = clamp((input.Timer / life ) * 0.6,0.0001,1);
	output.color.a = (input.Timer / life ) * 0.5;
	return output;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
GSPS_INPUT VS( VS_INPUT input )
{
    GSPS_INPUT output = (GSPS_INPUT)0;
	//output.Pos.xyz = input.Pos;


	float neck = smoothstep(150,170,input.Pos.z);
	//neck
	int value1 = step(174,input.Pos.z);
	int value2 = step(input.Pos.z,373 );
	//chest+arm 85 to 155 z axis
	//arm right 85 to 155 z axis, -190 to -45 x axis
	//arm left  85 to 155 z axis, 45 to 190 x axis
	//rest of body -265 to 163 z axis
	//head 170 to 255 z axis
	//lower leg -279 to -89 z axis
	int lowerleg_min = step(-279,input.Pos.z);
	int lowerleg_max = step(input.Pos.z,-89);
	float lowerleg = smoothstep(-109,-59,input.Pos.z);
	int bodystep = step(input.Pos.z,170);

	int arm_right_min = step(-190,input.Pos.x);
	int arm_right_max = step(input.Pos.x,-45);

	int arm_left_min = step(45,input.Pos.x);
	int arm_left_max = step(input.Pos.x,190);

	int chestmin = step(85,input.Pos.z);
	int chestmax = step(input.Pos.z, 155);

	//right hand = 80 to 208 x axis
	int hand_right_min = step(80,input.Pos.x);
	int hand_right_max = step(input.Pos.x,208);

	int hand_left_min = step(-208,input.Pos.x);
	int hand_left_max = step(input.Pos.x,-80);

	int lefthandstep =0;
	if(hand_left_min ==1&& hand_left_max ==1)
	{
		lefthandstep = 1;
	}



	int righthandstep =0;
	if(hand_right_min ==1&& hand_right_max ==1)
	{
		righthandstep = 1;
	}

	int rightramstep =0;
	if(arm_right_min ==1&& arm_right_max ==1&&chestmin==1 && chestmax ==1)
	{
		rightramstep =1;
	}

	int leftarmstep = 0;
	if(arm_left_min == 1 && arm_left_max == 1 && chestmin==1 && chestmax ==1)
	{
		leftarmstep =1;
	}
	//float body = smoothstep(g_min,g_max,input.Pos.z);
	int neck_inrange = 0;
	int lowerleg_inrange =0;
	if(lowerleg_min == 1&& lowerleg_max ==1)
	{
		lowerleg_inrange = 1;
	}
	if(value1 ==1 && value2 ==1)
	{
		neck_inrange = 1;
	}
	//float value = smoothstep(g_min, g_max, input.Pos.z);
	//
	//float armtest = smoothstep(60,110, input.Pos.z) * (1-smoothstep(110+50,110 +60,input.Pos.z))*smoothstep(g_min,g_max, input.Pos.x) * (1-smoothstep(g_min2,g_max2 ,input.Pos.x)) ;
	float rotation = radians(0)*bodystep;
	float c = cos(rotation);
	float s = sin(rotation);
		
	float armrotation = radians(45)*rightramstep;
	float arm_c = cos(armrotation);
	float arm_s = sin(armrotation);

	float armrotation2 = radians(300)*leftarmstep;
	float arm_c2 = cos(armrotation2);
	float arm_s2 = sin(armrotation2);

	//sin wave animation
	float angle=(time.x%360)*2;
    float freqx = 1.0f+sin(time.x)*4.0f;
    float freqy = 1.0f+sin(time.x*1.3f)*4.0f;
    float freqz = 1.0f+sin(time.x*1.1f)*4.0f;
    float amp = 1.0f+sin(time.x*1.4)*30.0f;
    
  
		
	matrix rotmat = (c,s,0,0,-s,c,0,0,0,0,1,0,0,0,0,1);

	output.Pos = float4(input.Pos,1);
	
	if(rightramstep)
	{
	output.Pos.x = input.Pos.x*arm_c - input.Pos.y*arm_s;

	output.Pos.y = input.Pos.x*arm_s + input.Pos.y*arm_c;

	}
	else
	{
	output.Pos.x = input.Pos.x*arm_c2 - input.Pos.y*arm_s2;

	output.Pos.y = input.Pos.x*arm_s2 + input.Pos.y*arm_c2;
	}

	////long neck
	output.Pos.z = output.Pos.z + (80*neck);
	output.Pos.y = output.Pos.y - (80*lowerleg);

	output.Pos.x = output.Pos.x -(50*rightramstep);
	output.Pos.x = output.Pos.x +(50*leftarmstep);

	//output.Pos.y = output.Pos.y - (180*neck);



	float f = sin(input.Norm.x*freqx + time.x) * sin(input.Norm.y*freqy + time.x) * sin(input.Norm.z*freqz + time.x)*righthandstep;
	float f2 = sin(input.Norm.z*freqz+time.x)*lefthandstep*50;
	output.Pos.z += input.Norm*f2;
	output.Pos.xyz += input.Norm*amp*f;

	output.Pos.xyz += 20*input.Norm*sin(10)*bodystep;
	output.Pos.xyz /=2;
	if(time.z == 0)
	{
		
	}

	//output.Pos.xyz /=2;


	output.PosW = output.Pos;
    output.Pos = mul( float4(output.Pos.xyz,1), World );
	output.Pos = mul(output.Pos,View);
	output.Pos = mul(output.Pos,Projection);
	
	//output.Pos.xyz *=2;
	
	//output.PosV = input.Pos;
	output.Norm = input.Norm;
    //output.Norm = mul( input.Norm, (float3x3)World );
    output.Tex = input.Tex;
    
    return output;
}

[maxvertexcount(128)]
void GS_EXPLODE_FIRSTPASS(point GSPS_INPUT input[1], inout PointStream<VS_explode_input> OutStream)
{
	VS_explode_input output;
	output.pos = input[0].PosW.xyz;
	output.vel = normalize(mul(float4(input[0].Norm,1),World).xyz)*10;
	output.vel = input[0].Norm*30;
	output.Timer = 10;
	OutStream.Append(output); 
}
[maxvertexcount(128)]
void GS_EXPLODE_Calc(point VS_explode_input input[1], inout PointStream<VS_explode_input> OutStream)
{
	VS_explode_input output = input[0];
	if(input[0].Timer > 0)
	{
		output.pos += input[0].vel*time.y;
		output.vel += float3(0,0,-10)*time.y;
		output.Timer -= time.y;
		OutStream.Append(output); 
	}
}

[maxvertexcount(4)]
void GS_Render(point VS_explode_out vin[1], inout TriangleStream<PS_explode> OutStream)
{
	PS_explode pin;

	float3 quad[4] = 
	{
        float3( -5, 5, 0 ),
        float3( 5, 5, 0 ),
        float3( -5, -5, 0 ),
        float3( 5, -5, 0 ),
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
		
		float4 inpos = mul(float4(vin[0].pos.xyz,1),World);
		position = quad[i];
		position.xyz=mul(position.xyz, invView);
		position +=inpos;
		//position = mul(position,(float3x3)InvViewMx) + vin[0].pos;
		//pin.pos = mul(float4(position,1.0), worldViewProj);
		pin.pos = mul(float4(position,1), View);
		pin.pos = mul(pin.pos, Projection);
		pin.color = vin[0].color;
		pin.tex = g_texcoords[i];
		OutStream.Append(pin);
	}
}

//Lab geometry shader
[maxvertexcount(12)]
void GS_AKLM(triangle GSPS_INPUT input[3], inout TriangleStream<GSPS_INPUT> TriStream)
{
	GSPS_INPUT output;
	//
    // Calculate the face normal
    //
    float3 faceEdgeA = input[1].PosW.xyz - input[0].PosW.xyz;
    float3 faceEdgeB = input[2].PosW.xyz - input[0].PosW.xyz;
    float3 faceNormal = normalize( cross(faceEdgeA, faceEdgeB) );
    //float3 ExplodeAmt = faceNormal*Explode;
    
    //
    // Calculate the face center
    //
    float3 centerPos = (input[0].PosW.xyz + input[1].PosW.xyz + input[2].PosW.xyz)/3.0;
    float2 centerTex = (input[0].Tex + input[1].Tex + input[2].Tex)/3.0;
    centerPos += (faceNormal*50*clamp(sin(time.x),0.0001,10))*smoothstep(115,175,centerPos.z);

	//add new triangles to inout stream
	for( int i=0; i<3; i++ )
    {
		int NextId = (i +1)%3;
		float3 V1 = centerPos - input[i].PosW;
		float3 V2 = centerPos - input[NextId].PosW;
		float3 N = normalize(cross(V1,V2));

        output.Pos = input[i].PosW;
		output.PosV = output.Pos;
		output.Pos = mul( output.Pos, World );
		output.PosW = output.Pos;
        output.Pos = mul( output.Pos, View );
        output.Pos = mul( output.Pos, Projection );
        output.Norm = input[i].Norm + faceNormal*0.2;
		output.Norm = mul(  output.Norm, (float3x3)World );
        output.Tex = input[i].Tex;

        TriStream.Append( output );
        
        //int iNext = (i+1)%3;
        output.Pos = input[NextId].PosW;
		output.PosV = output.Pos;
		output.Pos = mul( output.Pos, World );
		output.PosW = output.Pos;
        output.Pos = mul( output.Pos, View );
        output.Pos = mul( output.Pos, Projection );
        output.Norm = input[NextId].Norm + faceNormal*0.2;
		output.Norm = mul(  output.Norm, (float3x3)World );
        output.Tex = input[NextId].Tex;

        TriStream.Append( output );
        
		output.Pos = float4(centerPos,1);
		output.PosV = output.Pos;
		output.Pos = mul( output.Pos, World );
		output.PosW = output.Pos;
        output.Pos = mul( output.Pos, View );
        output.Pos = mul( output.Pos, Projection );
        output.Norm = faceNormal + faceNormal*0.2;
		output.Norm = mul(  output.Norm, (float3x3)World );

        output.Tex = centerTex;

        TriStream.Append( output );
        
    }
}


[maxvertexcount(4)]
void GS2Quad_AKLM(point GSPS_INPUT input[1], inout TriangleStream<GSPS_INPUT> TriStream)
{
	
	
	
float3 quad_pos[4] =
    {
        float3( -1 , 1 , 0 ),
        float3( 1, 1 , 0 ),
        float3( -1 , -1 , 0 ),
        float3( 1 , -1, 0 ),
    };
	GSPS_INPUT output;
	float3x3 invView=(float3x3)transpose(View);
	//add new triangles to inout stream
	for( int i=0; i<4; i++ )
    {
		float4 inpos = mul(float4(input[0].PosW.xyz,1),World);
		float3 position = quad_pos[i]*5;
		//position += float3(200,0,0);
		float4 PP=float4(position, 1.0);
		//PP = mul( PP, World );
		
		PP.xyz=mul(PP.xyz, invView);
		//PP.xyz = mul( PP, World );
		PP +=inpos;

		
		output.Pos = PP;
		output.PosV = output.Pos;
		//output.Pos.xyz += float3(200,0,0);
		//output.Pos= mul( PP, World );
		output.Pos = mul(output.Pos, View);
		output.Pos = mul(output.Pos, Projection);
		output.Norm = input[0].Norm;
		output.PosW = output.Pos;
        output.Tex = input[0].Tex; 
        TriStream.Append( output );       
    }



	//complete the triangle strip:
	TriStream.RestartStrip();
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( GSPS_INPUT input) : SV_Target
{


	float3 N=normalize(input.Norm);
	float3 L=normalize(g_vLightDir- input.PosW);

	float3 R=reflect(-L, N);
	float3 V=normalize(Eye-input.PosV);
	float specular = max(0.2, pow(dot(R, V), 1));
	float fLighting = saturate( dot( input.Norm, L ) );
	//chest+arm 85 to 155 z axis
	//arm right 85 to 155 z axis, -190 to -45 x axis
	//arm left  85 to 155 z axis, 45 to 190 x axis
	//rest of body -265 to 163 z axis
	//head 1770 to 255 z axis
	//lower leg -279 to -89 z axis
	//smoothstep chest+arm 60 to 110 z axis -239 to -89 xaxis?
	int chestmin = step(85,input.PosV.z);
	int chestmax = step(input.PosV.z, 155);


	int arm_right_min = step(-190,input.PosV.x);
	int arm_right_max = step(input.PosV.x,-45);

	int arm_left_min = step(45,input.PosV.x);
	int arm_left_max = step(input.PosV.x,190);

	//int value1 = step(g_min,input.PosV.z);
	//int value2 = step(input.PosV.z,g_max );
	/*float test = smoothstep(60,110, input.PosV.z) * (1-smoothstep(110+50,110 +60,input.PosV.z))*smoothstep(g_min,g_max, input.PosV.x) * (1-smoothstep(g_min+50,g_max +60,input.PosV.x)) ;
	float testsmooth = smoothstep(g_min,g_max,input.PosV.z);*/
	//int inrange = 0;
	//if(value1 ==1 && value2 ==1)
	//{
	//	inrange = 1;
	//}
	//float4 diffcol = (float4(1,0,0,1)*test);
	float neck = smoothstep(174,175,input.PosV.z);
	//float4 diffcol = (float4(1,0,0,1)*test);
	float4 outputColor = (g_txDiffuse.Sample( samLinear, input.Tex )) * (0.2*fLighting + specular);

    // Calculate lighting assuming light color is <1,1,1,1>
    //float fLighting = saturate( dot( input.Norm, vLightDir ) );
    //float4 outputColor = g_txDiffuse.Sample( samLinear, input.Tex ) * fLighting;
    outputColor.a = 1;
    return outputColor;
}
Texture2D g_txPart:register(t1);
float4 PS_Render(PS_explode pin) : SV_TARGET
{
	//return pin.color;
	return g_txPart.Sample( samLinear, pin.tex ) * pin.color;
	//return pin.color;
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
//technique10 Render
//{
//    pass P0
//    {
//        SetVertexShader( CompileShader( vs_4_0, VS() ) );
//        SetGeometryShader( CompileShader( gs_4_0, GS_AKLM() ) );
//        SetPixelShader( CompileShader( ps_4_0, PS() ) );
//        
//        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
//        SetDepthStencilState( EnableDepth, 0 );
//    }
//}


