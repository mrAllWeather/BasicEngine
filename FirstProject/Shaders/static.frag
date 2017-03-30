#version 330 core
out vec3 vertexColor;
out vec2 TexCoord;
out vec4 loc;

out vec4 color;

uniform float time;

// Texture samplers (We will revisit to increase texture ranges)
uniform sampler2D texture_01;
uniform sampler2D texture_02;
uniform sampler2D texture_03;

void main()
{
	color = texture2D(texture_01, TexCoord); // Will just use one texture for now
	// color = mix(texture2D(texture_01, TexCoord), texture2D(texture_02, TexCoord), 0.2); // We will ignore outColor for now
};