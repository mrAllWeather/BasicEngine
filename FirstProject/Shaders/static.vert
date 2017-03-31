#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 component;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;
out vec2 TexCoord;
out vec4 loc;

void main()
{
	gl_Position = model * vec4(position.x, position.y, position.z, 1);
	vertexColor = vec3(1.0f, 0.0f, 0.0f);
	// TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
	TexCoord = texCoord;
	
};