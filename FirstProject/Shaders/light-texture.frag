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

void main()
{
	color = texture(texture_01, TexCoord); // Will just use one texture for now
	if(lightCount > 0)
	{

		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);

		// Diffuse Calculations
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * lightColor;
		
		// Specular Calculations
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = specularStrength * spec * lightColor;

		// Ambient Calculation
		vec3 ambient = ambientStrength * lightColor;


		vec3 result = (ambient + diffuse + specular) * vec3(color.r, color.g, color.b);
		color =  vec4(result, 1.0);
	}

};
