//
// Constant Buffer Variables
//
#define INTERVALS 100

//Bounding box

float3 vLightDir = float3(-0.577,0.577,-0.577);


cbuffer cbPSPerObject
{
    matrix View;
	float4 viewpos; //the w value of viewpos determine which object to show
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

float DistanceEstimator(float3 input)
{
	float d = (length(input-float3(0.0,0.0,0.0))-1.0); // ball

	d = max(d,- (length(input.xy-float2(0.0,0.0))-0.4)); // tube1 (xy) 

	d = max(d,- (length(input.yz-float2(0.0,0.0))-0.4)); // tube2(yz)

    d = max(d,- (length(input.xz-float2(0.0,0.0))-0.4)); // tube3(xz)

	return d;
}

float DE_fract1(float3 input)
{

	float3 a1 = float3(1,1,1);
	float3 a2 = float3(-1,-1,1);
	float3 a3 = float3(1,-1,-1);
	float3 a4 = float3(-1,1,-1);
	float3 c;
	int n = 0;
	float dist, d;
	float Iterations = 8;
	float Scale = 2;
	while (n < Iterations) {
		 c = a1; dist = length(input-a1);
	        d = length(input-a2); if (d < dist) { c = a2; dist=d; }
		 d = length(input-a3); if (d < dist) { c = a3; dist=d; }
		 d = length(input-a4); if (d < dist) { c = a4; dist=d; }
		input = Scale*input-c*(Scale-1.0);
		n++;
	}

	return length(input) * pow(Scale, float(-n));
}

float DE_fract2(float3 input)
{
	float3 pos = input;
	float dr = 1.0;
	float r = 0.0;
	float Bailout = 30;
	float Iterations = 8;
	float Power = 6;
	for (int i = 0; i < Iterations ; i++) {
		r = length(pos);
		if (r>Bailout) break;
		
		// convert to polar coordinates
		float theta = acos(pos.z/r);
		float phi = atan2(pos.y,pos.x);
		dr =  pow( r, Power-1.0)*Power*dr + 1.0;
		
		// scale and rotate the point
		float zr = pow( r,Power);
		theta = theta*Power;
		phi = phi*Power;
		
		// convert back to cartesian coordinates
		pos = zr*float3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		pos+=input;
	}
	return 0.5*log(r)*r/dr;
}



#define STEP 0.01
float3 CalcNormal1 ( float3 Position ) {
	
const float3 Zero = float3 ( 0.0, 0.0, 0.0 );
const float3 Unit = float3 ( 1.0, 1.0, 1.0 );
const float3 AxisX = float3 ( 1.0, 0.0, 0.0 );
const float3 AxisY = float3 ( 0.0, 1.0, 0.0 );
const float3 AxisZ = float3 ( 0.0, 0.0, 1.0 );
	float A = DistanceEstimator( Position + AxisX * STEP ) - DistanceEstimator( Position - AxisX * STEP );
	float B = DistanceEstimator( Position + AxisY * STEP ) - DistanceEstimator( Position - AxisY * STEP );
	float C = DistanceEstimator( Position + AxisZ * STEP )- DistanceEstimator( Position - AxisZ * STEP );
	return normalize( float3 ( A, B, C ) );
}

float3 CalcNormal2 ( float3 Position ) {
	
const float3 Zero = float3 ( 0.0, 0.0, 0.0 );
const float3 Unit = float3 ( 1.0, 1.0, 1.0 );
const float3 AxisX = float3 ( 1.0, 0.0, 0.0 );
const float3 AxisY = float3 ( 0.0, 1.0, 0.0 );
const float3 AxisZ = float3 ( 0.0, 0.0, 1.0 );
	float A = DE_fract1( Position + AxisX * STEP ) - DE_fract1( Position - AxisX * STEP );
	float B = DE_fract1( Position + AxisY * STEP ) - DE_fract1( Position - AxisY * STEP );
	float C = DE_fract1( Position + AxisZ * STEP )- DE_fract1( Position - AxisZ * STEP );
	return normalize( float3 ( A, B, C ) );
}

float3 CalcNormal3 ( float3 Position ) {
	
const float3 Zero = float3 ( 0.0, 0.0, 0.0 );
const float3 Unit = float3 ( 1.0, 1.0, 1.0 );
const float3 AxisX = float3 ( 1.0, 0.0, 0.0 );
const float3 AxisY = float3 ( 0.0, 1.0, 0.0 );
const float3 AxisZ = float3 ( 0.0, 0.0, 1.0 );
	float A = DE_fract2( Position + AxisX * STEP ) - DE_fract2( Position - AxisX * STEP );
	float B = DE_fract2( Position + AxisY * STEP ) - DE_fract2( Position - AxisY * STEP );
	float C = DE_fract2( Position + AxisZ * STEP )- DE_fract2( Position - AxisZ * STEP );
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
	
	float4 col = float4(color,1)*0.5;
	float4 specularColor = col*shininess;
	float3 view = normalize(viewpos - Pos);
	float4 result;
	float4 amb = float4(color,1);
	float3 emissive =  float4(0.1,0.1,0.1,0.1);
	float3 l = normalize(viewpos - Pos);
	float3 halfway = normalize(l + view);

	float NdotL = dot(n, l);
	float4 diff = float4(1,1,1,1)*saturate(NdotL);
	float3 r = reflect(l, n);
	float spec = float4(0.5,0.5,0.5,1)*pow(saturate(dot(v, r)), shininess) * (NdotL > 0.0);
	float3 specular = pow(saturate(dot(n, halfway)), shininess) * (NdotL > 0.0);

	float3 output = (saturate(amb + diff) *color+ specular)* float4(1,1,1,1) ;

	return float4(output,1);
	return col + amb+diff+spec;

}



float4 trace(Ray ray)
{
    float3 BoxMinimum = float3(-30,-30,-30);
	float3 BoxMaximum = float3(30,30,30);

	float4 output = {0.0, 0.0,0.0, 1.0}; 
	//float result = 0;
	//return result;
	float3 from = ray.o;
	float3 direction = ray.d;
	float totalDistance = 0.0;
	float MinimumDistance = 0.01;
	//float MaxRaySteps = 50;
	int steps;
	int MaximumRaySteps = 50;
	if(type ==1)
	{
		MaximumRaySteps = 30;
		MinimumDistance = 0.05;
	}
	

	float start, final;
	float t;
	//int depth = 3;
	if ( IntersectBox ( ray, BoxMinimum, BoxMaximum, start, final ) )
	{
		for (steps=0; steps < MaximumRaySteps; steps++) {
			float3 p = from + totalDistance * direction;
				float distance;
			if(type == 0)
			{
				distance = DistanceEstimator(p);
			}
			else if(type == 1)
			{
				distance = DE_fract1(p);
			}
			else
			{
				distance = DE_fract2(p);
			}


			totalDistance += distance;
			if (distance < MinimumDistance)
			{
				float4 col = float4(0.5,0.5,0.5,1.0);
					float3 n;
				if(type == 0)
				{
					n = CalcNormal1(p);
				}
				else if(type == 1)
				{
					n = CalcNormal2(p);
				}
				else
				{
					n = CalcNormal3(p);
				}

				output = Shade_Marching(p,n,ray.d, col,16);
				return output;
			}
		}
	}
	//return 1.0-float(steps)/float(MaxRaySteps);
	return output;
}

float4 Image( PS_INPUT input) : SV_Target
{
	float zoom = 1;
float winHeight = 800;
float winWidth = 600;

	//ray tracing stuff
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

	c = trace(eyeray);
	return c;

}



