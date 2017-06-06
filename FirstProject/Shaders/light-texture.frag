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
	sampler2D specular;
	vec3 diffuse_color;
	float shininess;
	bool loaded;
};

in vec2 TexCoord;
in vec3 vertexColor;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform float time;
uniform vec3 viewPos;
uniform Light light;
uniform Material material[ 4 ];
uniform bool is_heightmap;
uniform sampler2D heightmap;
uniform vec2 heightmap_scale;

void main()
{
	float alpha = (texture(material[0].diffuse, TexCoord)).a;

	if(alpha < 0.1)
		discard;


	else if(light.active > 0)
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

		vec3 halfwayDir = normalize(lightDir + viewDir);

		// Diffuse value
		float diff = max(dot(norm, lightDir), 0.0);

		vec3 ambient, diffuse, specular;
		float spec;
		// Layered Materials
		int i = 0;
		if(is_heightmap)
		{

			vec2 scaleCoords = vec2((TexCoord.x)/(heightmap_scale.x), (TexCoord.y)/(heightmap_scale.y));
			vec4 height_details = texture(heightmap, scaleCoords); // If no lights, just show texture

			float tier_0_scale = height_details.g;
			if(tier_0_scale < 0.55)
				tier_0_scale = 0;

			float tier_2_scale = 1 - height_details.g;
			if(tier_2_scale < 0.55)
				tier_2_scale = 0;

			float tier_1_scale = 1 - tier_0_scale - tier_2_scale;

			vec3 map_diff = tier_0_scale * vec3(texture(material[0].diffuse, TexCoord)) +
					tier_1_scale * vec3(texture(material[1].diffuse, TexCoord)) +
					tier_2_scale * vec3(texture(material[2].diffuse, TexCoord));

			vec3 map_spec = tier_0_scale * vec3(texture(material[0].specular, TexCoord)) +
				tier_1_scale * vec3(texture(material[1].specular, TexCoord)) +
				tier_2_scale * vec3(texture(material[2].specular, TexCoord));

			float map_shine = tier_0_scale * material[0].shininess +
				tier_1_scale * material[1].shininess +
				tier_2_scale * material[2].shininess;


			// Ambient Calculation
			ambient = light.ambient * map_diff;

			// Diffuse Calculations
			diffuse = diff * light.diffuse * map_diff;

			// Specular Calculations
			spec = pow(max(dot(norm, halfwayDir), 0.0), map_shine);
			specular = light.specular * spec * map_spec;
		}
		else
		{
			// Ambient Calculation
			ambient = light.ambient * vec3(texture(material[0].diffuse, TexCoord));

			// Diffuse Calculations
			diffuse = diff * light.diffuse * vec3(texture(material[0].diffuse, TexCoord));

			// Specular Calculations
			spec = pow(max(dot(norm, halfwayDir), 0.0), material[0].shininess);
			specular = light.specular * spec * vec3(texture(material[0].specular, TexCoord));
		}

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

		color =  vec4(ambient + diffuse + specular, alpha);
	}
	else
	{
		color = texture(material[0].diffuse, TexCoord); // If no lights, just show texture
	}

}
