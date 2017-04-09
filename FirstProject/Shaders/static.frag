#version 330 core
in vec3 vertexColor;
in vec2 TexCoord;
in vec4 loc;

out vec4 color;

uniform float time;

// Light details
uniform float ambientStrength;
uniform vec3 lightColor;

// Texture samplers (We will revisit to increase texture ranges)
uniform int texture_count;
uniform sampler2D texture_01;
uniform sampler2D texture_02;
uniform sampler2D texture_03;

void main()
{
	color = texture(texture_01, TexCoord); // Will just use one texture for now

	vec3 ambient = ambientStrength * lightColor;

	color = color * vec4(ambient, 1.0);

};
