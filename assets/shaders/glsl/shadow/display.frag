#version 460
#extension GL_EXT_scalar_block_layout:enable

#ifdef VULKAN
#define SET(x)set=x,
#else
#define SET(x)
#endif
//layout(early_fragment_tests)in;

#define PI 3.141592653

layout(location=0)out vec4 outColor;

layout(location=0)in vec3 fs_position;
layout(location=1)in vec3 fs_normal;
layout(location=2)in vec2 fs_texcoord;

layout(SET(2)binding=1)uniform Material
{
	vec3 ambient;
	float shininess;
	vec3 diffuse;
	float ior;// index of refraction
	vec3 specular;
	float dissolve;// 1 == opaque; 0 == fully transparent
	vec3 transmittance;
	float roughness;
	vec3 emission;
	float metallic;
	float sheen;// [0, 1] default 0
	float clearcoat_thickness;// [0, 1] default 0
	float clearcoat_roughness;// [0, 1] default 0
	float anisotropy;// aniso. [0, 1] default 0
	float anisotropy_rotation;// anisor. [0, 1] default 0
	
	int ambient_tex_index;// map_Ka
	int diffuse_tex_index;// map_Kd
	int specular_tex_index;// map_Ks
	int specular_highlight_tex_index;// map_Ns
	int bump_tex_index;// map_bump, map_Bump, bump
	int displacement_tex_index;// disp
	int alpha_tex_index;// map_d
	int reflection_tex_index;// refl
	
	int roughness_tex_index;//map_Pr
	int metallic_tex_index;//map_Pm
	int sheen_tex_index;//map_Ps
	int emissive_tex_index;//map_Ke
}uboMaterial;

//only for obj
layout(SET(2)binding=0)uniform sampler2D textures[8];

layout(SET(1)binding=0)buffer UBOCamera
{
	mat4 V;
	mat4 P;
	vec3 eyePos;
	int cascadeNum;
	float cascadeSplit[];
}uboCamera;

layout(SET(3)binding=1)buffer UBOLight
{
	mat4 transformMatrix;
	vec3 color;
	int lightType;// 1 directional, 2 sphere, 3 spot, 4 tube, disable when <0
	vec3 radius;
	float rmax;
	
	float radiance;
	float cosThetaU;
	float cosThetaP;
	int vertexNum;
	
	vec4 vertices[4];
	mat4 PV[];
}uboLight;

layout(SET(4)binding=2)uniform UBOBuffer
{
	int renderMode;
	float normalBiasFactor;
	float shadowFilterRadius;
};
layout(SET(4)binding=8)uniform sampler2DArray shadowMap;
layout(SET(4)binding=9)uniform sampler2DArrayShadow shadowMap2;

const float ambient=0.;

//=============
mat3 surfaceTBN(vec3 n)
{
	vec3 UpVec=abs(n.z)<.999?vec3(0,0,1):vec3(1,0,0);
	//vec3 UpVec = abs(N.y) < 0.999 ? vec3(0,1,0):vec3(1,0,0);
	//vec3 UpVec=vec3(0,1,0);
	vec3 t=normalize(cross(UpVec,n));
	vec3 b=cross(n,t);
	return mat3(t,b,n);
}
mat4 surfaceTBN(vec3 n,vec3 o)
{
	vec3 UpVec=abs(n.z)<.999?vec3(0,0,1):vec3(1,0,0);
	//vec3 UpVec = abs(N.y) < 0.999 ? vec3(0,1,0):vec3(1,0,0);
	//vec3 UpVec=vec3(0,1,0);
	vec3 t=normalize(cross(UpVec,n));
	vec3 b=cross(n,t);
	return mat4(t,0,b,0,n,0,o,1);
}
float specBlinnPhong(float cosTheta,float shininess)
{
	return pow(cosTheta,shininess);
}
vec3 diffLambertian(vec3 albedo)
{
	return albedo/PI;
}

float lightDist(float r,float rmax)
{
	float a=r/rmax;
	float b=max(1-a*a,0);
	return b*b;
}
float lightWin(float r,float rmax)
{
	float a=r/rmax;
	float b=max(1-pow(a,4),0);
	return b*b;
}
float lightFactor(float r,float r0,float rmax)
{
	return lightWin(r,rmax)*(r0*r0/(r*r+.0001));
}
float lightFactorSpot(float r,float rmax,float cosTheta,float cosThetaU,float cosThetaP)
{
	float t=clamp((cosTheta-cosThetaU)/(cosThetaP-cosThetaU),0,1);
	return lightWin(r,rmax)*t*t;
}

float rand(vec2 co){
	return fract(sin(dot(co,vec2(12.9898,78.233)))*43758.5453);
}

mat2 rotate(float angle)
{
	float sinAngle=sin(angle);
	float cosAngle=cos(angle);
	return mat2(cosAngle,sinAngle,-sinAngle,cosAngle);
}

float textureProj(vec4 shadowCoord,vec2 offset,uint cascadeIndex)
{
	if(shadowCoord.z>0.&&shadowCoord.z<1.&&shadowCoord.w>0)
	{
		float dist=texture(shadowMap,vec3(shadowCoord.st+offset,cascadeIndex)).r;
		return float(dist>=shadowCoord.z);
	}
	return 1.;
}
float occDepthAvg(vec3 shadowCoord,vec2 ds,uint cascadeIndex)
{
	float dist=0.;
	//mat2 rotM=rotate(rand(shadowCoord.xy*2*PI));
	int count=0;
	int range=1;
	for(int x=-range;x<=range;x++){
		for(int y=-range;y<=range;y++){
			float a=texture(shadowMap,vec3(shadowCoord.st+ds*vec2(x,y),cascadeIndex)).r;
			if(a<shadowCoord.z)
			{
				dist+=a;
				++count;
			}
		}
	}
	return count==0?0:dist/count;
}
float filterHard(vec3 p,vec3 N,int cascadeIndex)
{
	vec3 shadowPos=p+N*normalBiasFactor;
	vec4 shadowCoord=uboLight.PV[cascadeIndex*2]*uboLight.PV[cascadeIndex*2+1]*vec4(shadowPos,1);
	shadowCoord.xyz/=shadowCoord.w;
	shadowCoord.xy=shadowCoord.xy*.5+.5;
	//return textureProj(shadowCoord,vec2(0),cascadeIndex);
	return texture(shadowMap2,vec4(shadowCoord.xy,cascadeIndex,shadowCoord.z));
}
float filterPCF(vec3 p,vec3 N,uint cascadeIndex,float scale)
{
	vec3 shadowPos=p+N*normalBiasFactor;
	vec4 shadowCoord=uboLight.PV[cascadeIndex*2]*uboLight.PV[cascadeIndex*2+1]*vec4(shadowPos,1);
	shadowCoord.xyz/=shadowCoord.w;
	shadowCoord.xy=shadowCoord.xy*.5+.5;
	
	ivec2 texDim=textureSize(shadowMap,0).xy;
	vec2 ds=scale*1./texDim.xy;
	mat2 rotM=rotate(rand(shadowCoord.xy)*2*PI);
	
	float shadowFactor=0;
	
	int count=0;
	int range=2;
	for(int x=-range;x<=range;x++){
		for(int y=-range;y<=range;y++){
			shadowFactor+=textureProj(shadowCoord,rotM*ds*vec2(x,y),cascadeIndex);
			count++;
		}
	}
	shadowFactor/=count;
	return shadowFactor;
}
float filterPCF_HW(vec3 p,vec3 N,uint cascadeIndex,float scale)
{
	vec3 shadowPos=p+N*normalBiasFactor;
	vec4 shadowCoord=uboLight.PV[0]*uboLight.PV[1]*vec4(shadowPos,1);
	shadowCoord.xyz/=shadowCoord.w;
	shadowCoord.xy=shadowCoord.xy*.5+.5;
	
	ivec2 texDim=textureSize(shadowMap,0).xy;
	vec2 ds=scale*1./texDim.xy;
	mat2 rotM=rotate(rand(shadowCoord.xy)*2*PI);
	
	float shadowFactor=0;
	
	int count=0;
	int range=2;
	for(int x=-range;x<=range;x++){
		for(int y=-range;y<=range;y++){
			shadowFactor+=texture(shadowMap2,vec4(shadowCoord.xy+rotM*ds*vec2(x,y),cascadeIndex,shadowCoord.z));
			count++;
		}
	}
	shadowFactor/=count;
	return shadowFactor;
}
float filterPCSS(vec3 p,vec3 N,uint cascadeIndex)
{
	vec3 shadowPos=p+N*normalBiasFactor;
	vec4 shadowCoord=uboLight.PV[cascadeIndex*2]*uboLight.PV[cascadeIndex*2+1]*vec4(shadowPos,1);
	shadowCoord.xyz/=shadowCoord.w;
	shadowCoord.xy=shadowCoord.xy*.5+.5;
	
	ivec2 texDim=textureSize(shadowMap,0).xy;
	vec2 ds=1./vec2(texDim);
	
	//blocker search
	float scale=2*shadowCoord.z*shadowFilterRadius;
	float d_occ=occDepthAvg(shadowCoord.xyz,ds*scale,cascadeIndex);
	if(d_occ==0)
		return 1.;
	float radius=(shadowCoord.z-d_occ)/shadowCoord.z*uboLight.rmax*shadowFilterRadius;
	//=======================
	//do pcf
	float shadowFactor=0.;
	mat2 rotM=rotate(rand(shadowCoord.xy)*2*PI);
	
	int count=0;
	int range=2;
	for(int x=-range;x<=range;x++){
		for(int y=-range;y<=range;y++){
			shadowFactor+=textureProj(shadowCoord,rotM*radius*ds*vec2(x,y),cascadeIndex);
			count++;
		}
	}
	shadowFactor/=count;
	return shadowFactor;
}
float filterPCSS_HW(vec3 p,vec3 N,uint cascadeIndex)
{
	vec3 shadowPos=p+N*normalBiasFactor;
	vec4 shadowCoord=uboLight.PV[cascadeIndex*2]*uboLight.PV[cascadeIndex*2+1]*vec4(shadowPos,1);
	shadowCoord.xyz/=shadowCoord.w;
	shadowCoord.xy=shadowCoord.xy*.5+.5;
	
	ivec2 texDim=textureSize(shadowMap,0).xy;
	vec2 ds=1./vec2(texDim);
	
	//blocker search
	float scale=2*shadowCoord.z*shadowFilterRadius;
	float d_occ=occDepthAvg(shadowCoord.xyz,ds*scale,cascadeIndex);
	if(d_occ==0)
		return 1.;
	float radius=(shadowCoord.z-d_occ)/shadowCoord.z*uboLight.rmax*shadowFilterRadius;
	//=======================
	//do pcf
	float shadowFactor=0.;
	mat2 rotM=rotate(rand(shadowCoord.xy)*2*PI);
	
	int count=0;
	int range=2;
	for(int x=-range;x<=range;x++){
		for(int y=-range;y<=range;y++){
			shadowFactor+=texture(shadowMap2,vec4(shadowCoord.xy+rotM*radius*ds*vec2(x,y),cascadeIndex,shadowCoord.z));
			count++;
		}
	}
	shadowFactor/=count;
	return shadowFactor;
}

float filterVSM(vec3 p,vec3 N,uint cascadeIndex)
{
	vec3 shadowPos=p+N*normalBiasFactor;
	vec4 shadowCoord=uboLight.PV[cascadeIndex*2]*uboLight.PV[cascadeIndex*2+1]*vec4(shadowPos,1);
	shadowCoord.xyz/=shadowCoord.w;
	shadowCoord.xy=shadowCoord.xy*.5+.5;
	
	ivec2 texDim=textureSize(shadowMap,0).xy;
	vec2 ds=1./vec2(texDim);

	//blocker search
	float scale=2*shadowCoord.z*shadowFilterRadius;
	float d_occ=occDepthAvg(shadowCoord.xyz,ds*scale,cascadeIndex);
	if(d_occ==0)
		return 1.;

	float radius=(shadowCoord.z-d_occ)/shadowCoord.z*uboLight.rmax*shadowFilterRadius;
	//====================
	mat2 rotM=rotate(rand(shadowCoord.xy)*2*PI);
	float M1=0.,M2=0;
	
	int count=0;
	int range=2;
	for(int x=-range;x<=range;x++){
		for(int y=-range;y<=range;y++){
			float a=texture(shadowMap,vec3(shadowCoord.xy+rotM*radius*ds*vec2(x,y),cascadeIndex)).r;
			M1+=a;
			M2+=a*a;
			++count;
		}
	}
	M1/=count;
	M2/=count;
	
	if(M1>shadowCoord.z)
	return 1.;
	
	float var=abs(M2-M1*M1);
	float b=shadowCoord.z-M1;
	float pmax=var/(var+b*b);
	pmax=smoothstep(0.2,1,pmax);
	return pmax;
}

void main()
{
	vec3 V=normalize(uboCamera.eyePos-fs_position);
	
	vec3 N=fs_normal;
	mat4 TBN=surfaceTBN(N,fs_position);
	
	vec4 albedo=vec4(uboMaterial.diffuse,1);
	
	if(uboMaterial.bump_tex_index>=0)
	{
		N+=mat3(TBN)*normalize(texture(textures[uboMaterial.bump_tex_index],fs_texcoord).xyz*2-1);
		N=normalize(N);
	}
	
	if(uboMaterial.diffuse_tex_index>=0)
	albedo=texture(textures[uboMaterial.diffuse_tex_index],fs_texcoord);
	//if(uboMaterial.alpha_tex_index>=0)
	//	alpha=texture(textures[uboMaterial.alpha_tex_index],fs_texcoord).a;
	
	//=========================================
	vec3 fd=diffLambertian(albedo.rgb);
	vec3 fs=vec3(1.);
	
	vec3 Lo=ambient*albedo.rgb;
	
	vec3 R=reflect(-V,N);
	vec3 lightPos=uboLight.transformMatrix[3].xyz;
	vec3 l=lightPos-fs_position;
	vec3 L_dir=mat3(uboLight.transformMatrix)*vec3(0,0,-1);
	vec3 L=normalize(l);
	vec3 H;
	float NdotL=0;
	
	float phongCostheta=1.;
	NdotL=max(dot(N,L),0);
	H=normalize(L+V);
	
	//============
	float f_light=lightFactorSpot(length(l),uboLight.rmax,dot(-L,L_dir),uboLight.cosThetaU,uboLight.cosThetaP);
	if(f_light>0)
	{
		phongCostheta=clamp(dot(N,H),0,1);
		fs=uboMaterial.specular*specBlinnPhong(phongCostheta,uboMaterial.shininess);
		
		float shadowFactor=1.;
		switch(abs(renderMode))
		{
			case 1:
			shadowFactor=filterHard(fs_position,N,0);
			break;
			case 2:
			shadowFactor=filterPCF(fs_position,N,0,shadowFilterRadius);
			break;
			case 3:
			shadowFactor=filterPCSS(fs_position,N,0);
			break;
			case 4:
			shadowFactor=filterPCF_HW(fs_position,N,0,shadowFilterRadius);
			break;
			case 5:
			shadowFactor=filterPCSS_HW(fs_position,N,0);
			break;
			case 6:
			shadowFactor=filterVSM(fs_position,N,0);
			break;
		}
		if(renderMode>0)
		Lo+=((fd*PI+fs)*uboLight.radiance*uboLight.color*NdotL)*shadowFactor*f_light;
		else
		Lo=vec3(shadowFactor);
	}
	//Lo+=albedo.rgb*0.1;
	outColor=vec4(Lo,albedo.a);
}