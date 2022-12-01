#version 460
layout(location=0) in vec2 inTexcoord;
layout(location=0) out vec4 fragColor;
layout(binding=0) uniform sampler2D tex;

#ifdef VULKAN
#define SET(x) set=x,
#define PUSH_CONSTANT push_constant
#else
#define SET(x)
#extension GL_EXT_scalar_block_layout: enable
#define PUSH_CONSTANT std430,binding=1
#endif
layout(PUSH_CONSTANT) uniform uPushConstant{
	int channel;
	int mode;
	float saturationFactor;
	float exposure;
	float factor0;
	float factor1;
	int intfactor0;
	int acesFilm;
};

void f_convertToCMY(inout vec3 col)
{
	col=1.-col;
}
void f_convertToHSI(inout vec3 col)
{
	vec3 ret;
	float a=col.r-col.g;
	float b=col.r-col.b;
	float theta=acos(0.5*(a+b)/sqrt(a*a+b*(col.g-col.b)));
	ret.r=col.b<=col.g?theta:(2*3.141592653-theta);
	ret.g=1.-3*min(min(col.r,col.g),col.b)/(col.r+col.g+col.b);
	ret.b=(col.r+col.g+col.b)/3;
	col=ret;
}
void f_log(inout vec3 col)
{
	col=log(1+col);
}
void f_power(inout vec3 col)
{
	col=pow(col,vec3(factor0));
}
void f_constrast_smoothstep(inout vec3 col)
{
	col=smoothstep(vec3(0),vec3(1),col);
}
void f_intensity_slice(inout vec3 col)
{
	float a=col.r;
	int channel=intfactor0;
	if(channel==1)
		a=col.g;
	else if(channel==2)
		a=col.b;
	if(a>=factor0&&a<=factor1)
		col=vec3(0,1,0);
}
vec3 ACESFilm(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e),0,1);
}

//RGB
float grayScale(vec3 x)
{
	return dot(vec3(0.2126,0.7152,0.0722),x);
}
vec3 saturation(vec3 x,float factor)
{
	return mix(x,vec3(grayScale(x)),factor);
}

void main()
{
	vec4 col=texture(tex,inTexcoord);

	switch(mode)
	{
		case 1:
		f_convertToCMY(col.rgb);	
		break;
		case 2:
		f_convertToHSI(col.rgb);	
		break;
		case 3:
		f_log(col.rgb);
		break;
		case 4:
		f_power(col.rgb);
		break;
		case 5:
		f_constrast_smoothstep(col.rgb);
		break;
		case 6:
		f_intensity_slice(col.rgb);
		break;
	}

	switch(channel)
	{
		case 1:
		col.rgb=vec3(col.r);
		break;
		case 2:
		col.rgb=vec3(col.g);
		break;
		case 3:
		col.rgb=vec3(col.b);
		break;
		case 4:
		col.rgb=vec3(col.a);
		break;
	}
	//col.rgb=grayScale(col.rgb);
	col.rgb=saturation(col.rgb,saturationFactor);
	col.rgb*=exposure;
	if(bool(acesFilm))
		col.rgb=ACESFilm(col.rgb);
	fragColor=vec4(col.rgb,1);
}