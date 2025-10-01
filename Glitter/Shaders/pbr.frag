#version 330 core

// 反射和入射，所以只需要考虑反射，反射方程
// 反射方程：三个部分，能量注入（积分），BRDF定义表面属性计算可以往某个方向辐射出多少能量（几何、菲涅尔、法线分布函数），得到出射能量
// 最常用 Cook-Torrance BRDF

// t b n 三个向量都应该在 fs 中进行一次 normalize 再合并 tbn 矩阵，不然光栅化的时候插值运算可能会导致 vs 传过来的 t b n 三个向量不是单位长度。

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform vec3 camPos;

uniform samplerCube irradianceMap; // IBL
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// lights
const int NR_LIGHTS = 20;
uniform vec3 lightPositions[NR_LIGHTS];
uniform vec3 lightColors[NR_LIGHTS];

const float PI = 3.14159265359;

// 菲涅尔方程的近似解
// 计算了反射光和入射光的比例
// F0是垂直入射时的反射率
// vec3是因为不同波长的光反射率不同-RGB
vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   
// ----------------------------------------------------------------------------

// 法线分布函数
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// 几何遮蔽函数
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
    vec3 N = normalize(Normal); 
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N); 

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);
    
    // 直接光照，光源位置确定，直接迭代
    for(int i = 0; i < NR_LIGHTS; i++){
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColors[i] * attenuation;
       
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);

        vec3 nominator    = NDF * G * F; // DFG项
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
        vec3 specular     = nominator / denominator;  

        vec3 kS = F; // 菲涅尔方程计算了反射光和入射光的比例
        vec3 kD = vec3(1.0) - kS; // 折射光比例
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;
    
    color = color / (color + vec3(1.0)); // HDR tonemapping，Lo可能大于1.0
    color = pow(color, vec3(1.0/2.2)); // gamma校正, PBR要求在线性空间计算光照，要映射回去
    
    FragColor = vec4(color, 1.0);
}







