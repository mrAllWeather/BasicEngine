#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;

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
	gl_Position = vec4(position, 1);

	vertexColor = color;

	TexCoord = vec2(texCoord.x, 1-texCoord.y);

	// Normal = mat3(transpose(inverse(model * component))) * normal;
	Normal = normal;

	FragPos = position;
};
