#version 330 core
in vec2 TexCoord;
in vec3 vertexColor;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform float time;

// Light details
uniform int lightCount;
uniform float ambientStrength;
uniform float specularStrength;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

// Texture samplers (We will revisit to increase texture ranges)
uniform int texture_count;
uniform sampler2D texture_01;
uniform sampler2D texture_02;
uniform sampler2D texture_03;

// View Mode
uniform int view_mode;

void main()
{

	color = vec4(vertexColor, 1.0);
};
