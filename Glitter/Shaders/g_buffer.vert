#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // g_buffer需要世界坐标系下的信息
    vec4 worldPos = model * vec4(position, 1.0f);
    FragPos = worldPos.xyz;
    gl_Position = projection * view * worldPos;
    TexCoords = texCoords;
    Normal = transpose(inverse(mat3(model))) * normal; // M^-1^T * normal
};