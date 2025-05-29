#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Texture1;
uniform sampler2D Texture2;

void main()
{
	FragColor = mix(texture(Texture1, TexCoords), texture(Texture2, TexCoords), 0.3f);
};