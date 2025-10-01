#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 WorldPos; 

uniform mat4 projection;
uniform mat4 view;

void main()
{
	WorldPos = aPos;  // directional vector for the fragment shader
	mat4 rotView = mat4(mat3(view)); // Remove translation from the view matrix
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);
	gl_Position = clipPos.xyww; // Set w component to the z component to ensure depth is 1.0
}