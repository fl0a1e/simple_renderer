#version 330 core
#extension GL_ARB_shading_language_420pack : enable
out vec4 FragColor;
in vec2 TexCoords;

// 要采样的3个贴图
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

// 定义光源结构体和UBO
struct Light {
    vec3 Position;
    float padding1;  // 填充以满足16字节对齐
    vec3 Color;
    float Linear;    // 线性衰减系数
    float Quadratic; // 二次衰减系数
    vec2 padding2;   // 填充以满足16字节对齐
};

const int NR_LIGHTS = 800;
// 使用UBO存储光源数组 (std140布局确保跨平台对齐一致)
layout(std140, binding = 0) uniform LightBuffer {
    Light lights[NR_LIGHTS];  // 现在可以支持更多个光源
};

uniform vec3 viewPos;

void main(){
	// Retrieve data from gbuffer
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

	// Then calculate lighting as usual
	vec3 lighting = Diffuse * 0.1; // hard-coded ambient component
	vec3 viewDir = normalize(viewPos - FragPos);
	for(int i = 0; i < NR_LIGHTS; i++){
		// Diffuse
		vec3 lightDir = normalize(lights[i].Position - FragPos);
		vec3 diffuse = max(dot(Normal, lightDir), 0.0f) * Diffuse * lights[i].Color; 
		// Spec
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(Normal, halfwayDir), 0.0f), 16.0f);
		vec3 specular = lights[i].Color * spec * Specular;
		// Attenuation
        float distance = length(lights[i].Position - FragPos);
		float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
		diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
	}
	FragColor = vec4(lighting, 1.0);
}