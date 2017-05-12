#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 component;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 vertexColor;
out vec3 Normal;
out vec3 FragPos;

void main()
{
	gl_Position = projection * view * model * component * vec4(position, 1);
	vertexColor = vec3(1.0f, 0.0f, 0.0f);
	TexCoord = vec2(texCoord.x, 1-texCoord.y);
	Normal = mat3(transpose(inverse(model * component))) * normal;
	FragPos = vec3(model * component * vec4(position, 1.0f));
};
