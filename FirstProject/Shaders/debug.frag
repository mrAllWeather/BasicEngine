#version 330 core
// Material
struct Material {
	sampler2D diffuse;
	vec3 diffuse_color;
	sampler2D specular;
	float shininess;
};

in vec2 TexCoord;
in vec3 vertexColor;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform float time;

uniform Material material[4];

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
		color = vec4(material[0].diffuse_color, 1.0);
	}
	else // Default to white model
	{
		color =  vec4(1.0);
	}

}
