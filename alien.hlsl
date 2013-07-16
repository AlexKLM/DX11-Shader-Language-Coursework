//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	matrix		g_mWorldViewProjection;
	matrix		g_mWorld;
	float4		time;//x = time
};

cbuffer cbPerFrame : register( b1 )
{
	float3		g_vLightDir;
	float		g_fAmbient;
	float3 Eye;
	float padding;
};

//--------------------------------------------------------------------------------------
// DepthStates/BlendStates
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

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Pos          : POSITION;         //position
    float3 Norm         : NORMAL;           //normal
    float2 Tex          : TEXCOORD0;        //texture coordinate
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 PosV: POSITION0;
	float3 WorldPos: POSITION1;
    float3 Norm : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};

//TUTORIAL CODE


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    
    

	
	//float rotation = smoothstep(min,max,input.Pos.z);

	//head 150 to 170 z axis
	//neck 367 to 373 z axis
	//chest+arm 85 to 155 z axis
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

	//sin wave animation
	float angle=(time.x%360)*2;
    float freqx = 1.0f+sin(time.x)*4.0f;
    float freqy = 1.0f+sin(time.x*1.3f)*4.0f;
    float freqz = 1.0f+sin(time.x*1.1f)*4.0f;
    float amp = 1.0f+sin(time.x*1.4)*30.0f;
    
  
		
	matrix rotmat = (c,s,0,0,-s,c,0,0,0,0,1,0,0,0,0,1);

	output.Pos = float4(input.Pos,1);
	
	/*output.Pos.y = input.Pos.y*c - input.Pos.z*s;

	output.Pos.z = input.Pos.y*s + input.Pos.z*c;
*/
	
	/*output.Norm.x = input.Pos.x*c - input.Pos.y*s;

	output.Norm.y = input.Pos.x*s + input.Pos.y*c;*/
		output.Pos.x = input.Pos.x*arm_c - input.Pos.y*arm_s;

	output.Pos.y = input.Pos.x*arm_s + input.Pos.y*arm_c;

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
    //Pos.z += N.z * amp * f;
    //Pos.x += N.x * amp * f;
    //Pos.y += N.y * amp * f;

	output.Pos.xyz += 20*input.Norm*sin(10)*bodystep;
	//output.Pos.xyz += -20*input.Norm*sin(10)*armtest;

	//output.Pos.xyz /=2;
	output.PosV = output.Pos;
	output.WorldPos = mul(output.Pos, g_mWorld);
	//output.Pos.z *=5;
	output.Pos = mul( output.Pos, g_mWorldViewProjection );
	
	
	
   /* output.Pos = mul( output.Pos, View );

	

    output.Pos = mul( output.Pos, Projection );*/

	
	float3 test = normalize(mul(input.Norm ,output.Pos));

    output.Norm = mul( input.Norm, g_mWorld );



    output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{

	float3 N=normalize(input.Norm);
	float3 L=normalize(g_vLightDir- input.WorldPos);

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
	float4 outputColor = (g_txDiffuse.Sample( g_samLinear, input.Tex )) * (0.2*fLighting + specular);

    // Calculate lighting assuming light color is <1,1,1,1>
    //float fLighting = saturate( dot( input.Norm, vLightDir ) );
    //float4 outputColor = g_txDiffuse.Sample( samLinear, input.Tex ) * fLighting;
    outputColor.a = 1;
    return outputColor;

}
