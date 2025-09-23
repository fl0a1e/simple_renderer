#version 330 core
out vec4 FragColor;

// temp, from camera para
float near = 0.1f;
float far = 25.f;

void main(){
	// depth is non-linear
	// 1. than inverse non-linear function [near, far]
	// 2. normalize

	float linearDepth = (2 * near * far) / (near + far - gl_FragCoord.z * (far - near));
	float normalDepth = (linearDepth - near) / (far - near);
	FragColor = vec4(vec3(normalDepth), 1.0f);
};