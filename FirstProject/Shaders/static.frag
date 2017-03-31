#version 330 core
in vec3 vertexColor;
in vec2 TexCoord;
in vec4 loc;

out vec4 color;

uniform float time;

// Texture samplers (We will revisit to increase texture ranges)
uniform sampler2D texture_01;
uniform sampler2D texture_02;
uniform sampler2D texture_03;

void main()
{
	color = texture(texture_01, TexCoord); // Will just use one texture for now
	// color = mix(texture2D(texture_01, TexCoord), texture2D(texture_02, TexCoord), 0.2); // We will ignore outColor for now
};