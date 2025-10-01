#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
const float PI = 3.14159265359;

void main()
{	
    // 漫反射项只和N有关，所以我们可以直接使用WorldPos作为法线
	vec3 N = normalize(WorldPos);
	vec3 irradiance = vec3(0.0);

	// tangent space calculation from origin point
	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up         = normalize(cross(N, right));

	float sampleDelta = 0.025; // 采样间隔
	float nrSamples = 0.0;     // 采样次数
	// 对每个方向的半球面积分
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// 计算采样点，球面坐标转笛卡尔坐标（in 局部切线空间）
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// 从切线空间转换到世界空间
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	FragColor = vec4(irradiance, 1.0);
}