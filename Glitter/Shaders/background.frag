#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{
	vec3 envColor = texture(environmentMap, WorldPos).rgb; // in linear space

	// hdr tonemap and gamma correct
	vec3 mapped = envColor / (envColor + vec3(1.0));
	envColor = pow(mapped, vec3(1.0/2.2));

	FragColor = vec4(envColor, 1.0);
}