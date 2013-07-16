//
// Constant Buffer Variables
//
#define INTERVALS 100

//Bounding box





float3 vLightDir = float3(-0.577,0.577,-0.577);


cbuffer cbPSPerObject
{
    matrix View;
	float4 viewpos;
	int type;
	float3 padding;
};

struct VS_INPUT
{
    float3 Pos          : POSITION;         //position
    float3 Norm         : NORMAL;           //normal
    float2 Tex          : TEXCOORD0;        //texture coordinate
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float2 Tex : TEXCOORD1;
    float3 ViewR : TEXCOORD2;
};



//light postions and colors




struct Ray {
	float3 o; // origin
	float3 d; // direction
};

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

//
//Ray collision detections
//

//Marching cube interset method
bool IntersectBox ( in Ray ray, in float3 minimum, in float3 maximum, out float timeIn, out float timeOut ) //return time of ray enter the box and the time when it exits
{
	float3 OMIN = ( minimum - ray.o ) / ray.d;
	float3 OMAX = ( maximum - ray.o ) / ray.d;
	float3 MAX = max ( OMAX, OMIN );
	float3 MIN = min ( OMAX, OMIN );
	timeOut = min ( MAX.x, min ( MAX.y, MAX.z ) );
	timeIn = max ( max ( MIN.x, 0.0 ), max ( MIN.y, MIN.z ) );
	return timeOut > timeIn ;
}
 
float Function ( float3 Position) //implicit function for the shape
{
	
float T = 2.5;
float levelVal = 2;
	float X=Position.x;
	float Y=Position.y;
	float Z=Position.z;
	//that number need to be replace with a value(eg(cos(X+T*Y)
	float Fun = 2.0 - cos( X + T * Y ) - cos( X - T * Y ) - cos ( Y + T * Z ) - cos ( Y - T * Z ) - cos ( Z - T * X ) - cos ( Z + T * X );
	float Fun2 = T+ pow(X,3)+pow(Y,3)+pow(Z,3) + (X+Y+Z);
	return Fun - levelVal;
}

float Function2 ( float3 Position) //implicit function for the shape
{
	
float T = 2;
float levelVal = 3;
	float X=Position.x;
	float Y=Position.y;
	float Z=Position.z;
	//that number need to be replace with a value(eg(cos(X+T*Y)
	float Fun = 0.5 *(sin(T*X)*cos(Y)*sin(Z)+sin(T*Y)*cos(Z)*sin(X)+sin(T*Z)*cos(X)*sin(Y))-0.5*(cos(T*X)*cos(T*Y)+cos(T*Y)*cos(T*Z)+cos(T*Z)*cos(T*X))-levelVal*0.1;
	//float Fun2 = T+ pow(X,3)+pow(Y,3)+pow(Z,3) + (X+Y+Z);
	return Fun;
}


bool IntersectSurface ( in Ray ray, in float start, in float final, out float val )
{
	float step = ( final - start ) / float ( INTERVALS );
	float time = start;
	float3 Position = ray.o + time * ray.d;
	float right, left = Function ( Position);
	for ( int i = 0; i < INTERVALS; ++i )
	{
		time += step;
		Position += step * ray.d;
		if(type == 0)
		{
				right = Function ( Position );
		}
		else
		{
			right = Function2 ( Position );
		}
		if ( left * right < 0.0 )
		{
			val = time + right * step / ( left - right );
			return true;
		}
		left = right;
	}
	return false;
}

#define STEP 0.01
float3 CalcNormal1 ( float3 Position ) {
	
const float3 Zero = float3 ( 0.0, 0.0, 0.0 );
const float3 Unit = float3 ( 1.0, 1.0, 1.0 );
const float3 AxisX = float3 ( 1.0, 0.0, 0.0 );
const float3 AxisY = float3 ( 0.0, 1.0, 0.0 );
const float3 AxisZ = float3 ( 0.0, 0.0, 1.0 );
	float A = Function( Position + AxisX * STEP ) - Function( Position - AxisX * STEP );
	float B = Function( Position + AxisY * STEP ) - Function( Position - AxisY * STEP );
	float C = Function( Position + AxisZ * STEP )- Function( Position - AxisZ * STEP );
	return normalize( float3 ( A, B, C ) );
}

float3 CalcNormal2 ( float3 Position ) {
	
const float3 Zero = float3 ( 0.0, 0.0, 0.0 );
const float3 Unit = float3 ( 1.0, 1.0, 1.0 );
const float3 AxisX = float3 ( 1.0, 0.0, 0.0 );
const float3 AxisY = float3 ( 0.0, 1.0, 0.0 );
const float3 AxisZ = float3 ( 0.0, 0.0, 1.0 );
	float A = Function2( Position + AxisX * STEP ) - Function2( Position - AxisX * STEP );
	float B = Function2( Position + AxisY * STEP ) - Function2( Position - AxisY * STEP );
	float C = Function2( Position + AxisZ * STEP )- Function2( Position - AxisZ * STEP );
	return normalize( float3 ( A, B, C ) );
}






//
// Vertex Shader
//


PS_INPUT VS_CanvasSetup( uint id : SV_VertexID )
{

	PS_INPUT Output;
    Output.Tex = float2((id << 1) & 2, id & 2);
    Output.Pos = float4(Output.Tex * float2(2,-2) + float2(-1,1), 0, 1);
    //return Output;

//	PS_INPUT output = (PS_INPUT)0;
	float2 pixelPos=sign(Output.Pos.xy);
	Output.Pos =float4(pixelPos, 0, 1);
	Output.Tex = pixelPos;
	return Output;
}
	
//
// Pixel Shader
//

//lighting method
float4 Phong( float3 n, float3 l, float3 v, float shininess, float4 diffuseColor, float4
specularColor)
{
	float3 lightPosition={10, 60, 10.0};
float4 lightColor = {1.0, 1.0, 1.0, 1.0};
	float NdotL = dot(n, l);
	float diff = saturate(NdotL);
	float3 r = reflect(l, n);
	float spec = pow(saturate(dot(v, r)), shininess) * (NdotL > 0.0);
	return diff*diffuseColor + spec*specularColor;
}
float4 black = {0,0,0,0};

float4 Shade_Marching(float3 Pos,float3 n, float3 v, float3 color,float shininess)
{
	float3 lightPosition={10, 60, 10.0};
	float4 lightColor = {1.0, 1.0, 1.0, 1.0};
	float4 col = float4(color,0);
	float4 specularColor = col*shininess;
	float4 result;
	float3 l = normalize(lightPosition - Pos);
	float NdotL = dot(n, l);
	float diff = saturate(NdotL);
	float3 r = reflect(l, n);
	float spec = pow(saturate(dot(v, r)), shininess) * (NdotL > 0.0);
	return diff*col + spec*specularColor;

}

float4 Raytrace(Ray ray)
{
	const float3 BoxMinimum = float3(-10,-10,0);
const float3 BoxMaximum = float3(10,10,10);

	float4 result = (float4)0;
	float start, final;
	float t;
	
	if ( IntersectBox ( ray, BoxMinimum, BoxMaximum, start, final ) )
	{
		if ( IntersectSurface ( ray, start, final, t ) )
		{
			float3 Position = ray.o + ray.d * t;
				float3 normal;
			if(type ==0)
			{
				normal = CalcNormal1 ( Position );
			}
			else
			{
				normal = CalcNormal2 ( Position );
			}

			float3 color = ( Position - BoxMinimum ) / ( BoxMaximum - BoxMinimum );
				//float4 col = float4(color,0);
				result = Shade_Marching(Position,normal,ray.d, color,5.0);
		}	
	}

	return result;
}

float4 Image( PS_INPUT input) : SV_Target
{
	float zoom = 1;
float winHeight = 800;
float winWidth = 600;

	//ray tracing stuff
float3 eyePos={0, 0, -10 }; //eye position
float nearPlane=1; //distance from eye to image plane
float farPlane=500; //distance from eye to the far plane
	//RAY TRACING
	//set up ray direction and position
	Ray eyeray;
	eyeray.o = viewpos.xyz;
	float3 dir;

	dir.xy = input.Tex.xy*float2(1.0, winHeight/winWidth);
	dir.z = zoom* nearPlane; //zoom*nearplane (add this varible later)

	//transform ray to world space
	float4x4 ViewInverse=transpose(View);
	eyeray.d = mul(float4(dir, 0.0), ViewInverse).xyz;
	eyeray.d = normalize(eyeray.d);
	Ray curay = eyeray;
	int hitobj; //which object
	bool hit; //if
	float3 n; //normal
	float4 c = {0.0, 0.0,0.0, 0.0}; //initial colour value
	float4 result = {0.0, 0.0,0.0, 0.0};

	c = Raytrace(eyeray);
	return c;

}



