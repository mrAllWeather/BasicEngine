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

// Materials
uniform vec3 diffuseColor;

// View Mode
uniform int view_mode;

void main()
{
	if(view_mode == 0) // Wireframe Mode (We expect glPolygonMode to be called, so present default color)
	{
		color =  vec4(1.0);
	}
	else if(view_mode == 1) // Vertex Normal Mode
	{
		vec3 norm = normalize(Normal);
		color =  vec4(norm, 1.0);	
	}
	else if(view_mode == 2) // Diffuse Color Mode
	{
		color = vec4(diffuseColor, 1.0);
	}
	else // Default to white model
	{
		color =  vec4(1.0);
	}
};
