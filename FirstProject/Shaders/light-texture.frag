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
	if(light.active > 0)
	{
		vec3 lightDir;
		if(light.type == 0)
		{
			lightDir = normalize(-light.direction);
		}
		else
		{
			lightDir = normalize(light.position - FragPos);
		}
		
		vec3 norm = normalize(Normal);	
		
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		
		// Ambient Calculation
		vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
			
		// Diffuse Calculations
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoord));

		// Specular Calculations
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

		if(light.type == 2) // If Spotlight, soften our edges
		{
			float theta = dot(lightDir, normalize(-light.direction)); 
			float epsilon = (light.cut_off - light.outer_cut_off);
			float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
			diffuse  *= intensity;
			specular *= intensity;
		}
		
		if(light.type == 1 || light.type == 2) // If Point or Spotlight, Apply attenuation
		{
			float distance    = length(light.position - FragPos);
			float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

			ambient  *= attenuation;  
			diffuse  *= attenuation;
			specular *= attenuation;   
		}

		color =  vec4(ambient + diffuse + specular, 1.0);
	}
	else
	{
		color = texture(material.diffuse, TexCoord); // If no lights, just show texture
	}

};
