#version 330 core
// Light details
struct Light {
	int active; // Is there an active light?
	int type; // TBD

	vec3 position;
	vec3 direction;

	float cut_off;
	float outer_cut_off;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float ambientStrength;
	float specularStrength;

};

// Material details
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
uniform vec3 viewPos;
uniform Light light;
uniform Material material;

void main()
{
	color = texture(material.diffuse, TexCoord); // Will just use one texture for now
	if(light.active > 0)
	{

		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(light.position - FragPos);
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);

		// Diffuse Calculations
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * light.diffuse;
		
		// Specular Calculations
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = light.specularStrength * spec * light.specular;

		// Ambient Calculation
		vec3 ambient = light.ambientStrength * light.ambient;


		vec3 result = (ambient + diffuse + specular) * vec3(color.r, color.g, color.b);
		color =  vec4(result, 1.0);
	}

};
