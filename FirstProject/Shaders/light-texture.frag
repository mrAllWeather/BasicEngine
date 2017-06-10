#version 330

#define MAX_LIGHTS 4
#define MAX_MATERIALS 4

// Light details
/* Type defines the type of light this is
 * 0 - Directional Light
 * 1 - Point Light
 * 2 - Spot Light
 */
struct Light {
	bool enabled; // Is this light active
	int type;
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
	sampler2D normal;
	vec3 diffuse_color;
	float shininess;
	bool loaded;
};

struct MixedMaterial {
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

in Vert {
	vec3 Normal;

	vec2 TexCoord;
	vec3 FragPos;

	vec3 TangentLightPos [ MAX_LIGHTS ];
	vec3 TangentViewPos;
	vec3 TangentFragPos;

	mat3 TBN;
} vs_out;

out vec4 color;

uniform float time;
uniform vec3 viewPos;
uniform Light light [ MAX_LIGHTS ];
uniform Material material[ MAX_MATERIALS ];

// Height map details
uniform bool is_heightmap;
uniform sampler2D heightmap;
uniform vec2 heightmap_scale;

// - Prototypes	https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/6.multiple_lights/6.multiple_lights.fs
vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, Material material);
vec3 CalcPointLight(Light light, vec3 normal, vec3 lightPos, vec3 viewDir, Material material);
vec3 CalcSpotLight(Light light, vec3 normal, vec3 lightPos, vec3 viewDir, Material material);

// Pre-Mixed Materials
vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, MixedMaterial material);
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, MixedMaterial material);
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, MixedMaterial material);

void main()
{
	// Check to see if any of our assigned materials actually show up (alpha over 10%), else discard the fragment

	bool will_discard = true;
	for(int mat_idx = 0; mat_idx < MAX_MATERIALS; ++mat_idx)
	{
		if(material[mat_idx].loaded)
		{
			float alpha = (texture(material[mat_idx].diffuse, vs_out.TexCoord)).a;
			if(alpha >= 0.01)
			{
				will_discard = false;
			}
		}
		else
		{
			break;
		}
	}

	if(will_discard)
	{
		discard;
	}


	// Fragment Specific Values
	vec3 norm = normalize(texture(material[0].normal, vs_out.TexCoord).rgb * 2 - 1.0);

	vec3 viewDir = normalize(vs_out.TangentViewPos - vs_out.TangentFragPos);

	vec3 result = vec3(0);

	// Layered Materials (Only Heightmap for now)
	if(is_heightmap)
	{
		MixedMaterial mixed_material;
		// Determine out Material Mix
		vec2 scaleCoords = vec2((vs_out.TexCoord.x)/(heightmap_scale.x), (vs_out.TexCoord.y)/(heightmap_scale.y));
		vec4 height_details = texture(heightmap, scaleCoords); // If no lights, just show texture

		// Scale each supplied material
		float tier_0_scale = height_details.g;
		if(tier_0_scale < 0.60)
			tier_0_scale = 0;

		float tier_2_scale = 1 - height_details.g;
		if(tier_2_scale < 0.60)
			tier_2_scale = 0;

		float tier_1_scale = 1 - tier_0_scale - tier_2_scale;

		// Combine materials into a single mixed material
		mixed_material.diffuse = tier_0_scale * vec3(texture(material[0].diffuse, vs_out.TexCoord)) +
				tier_1_scale * vec3(texture(material[1].diffuse, vs_out.TexCoord)) +
				tier_2_scale * vec3(texture(material[2].diffuse, vs_out.TexCoord));

		mixed_material.specular = tier_0_scale * vec3(texture(material[0].specular, vs_out.TexCoord)) +
			tier_1_scale * vec3(texture(material[1].specular, vs_out.TexCoord)) +
			tier_2_scale * vec3(texture(material[2].specular, vs_out.TexCoord));

		mixed_material.shininess = tier_0_scale * material[0].shininess +
			tier_1_scale * material[1].shininess +
			tier_2_scale * material[2].shininess;

		for(int light_idx = 0; light_idx < MAX_LIGHTS; ++light_idx)
		{
			if(light[light_idx].enabled == false)
				continue;

			switch(light[light_idx].type)
			{
				case 0:
					result += CalcDirLight(light[light_idx], norm, viewDir, mixed_material);
					break;
				case 1:
					result += CalcPointLight(light[light_idx], norm, vs_out.FragPos, viewDir, mixed_material);
					break;
				case 2:
					result += CalcSpotLight(light[light_idx], norm, vs_out.FragPos, viewDir, mixed_material);
					break;
				default:
					break;
			}
		}
	}
	else
	{
		norm = -norm;
		for(int light_idx = 0; light_idx < MAX_LIGHTS; ++light_idx)
		{
			if(light[light_idx].enabled == false)
				continue;

			switch(light[light_idx].type)
			{
				case 0:
					result += CalcDirLight(light[light_idx], norm, viewDir, material[0]);
					break;
				case 1:
					result += CalcPointLight(light[light_idx], norm, vs_out.TangentLightPos[light_idx], viewDir, material[0]);
					break;
				case 2:
					result += CalcSpotLight(light[light_idx], norm, vs_out.TangentLightPos[light_idx], viewDir, material[0]);
					break;
				default:
					break;
			}
		}
	}

	color =  vec4(result, 1.0);

}

// calculates the color when using a directional light.
vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(vs_out.TBN * light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, vs_out.TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vs_out.TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, vs_out.TexCoord));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(Light light, vec3 normal, vec3 lightPos, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(lightPos - vs_out.TangentFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(lightPos - vs_out.TangentFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, vs_out.TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vs_out.TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, vs_out.TexCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(Light light, vec3 normal, vec3 lightPos, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(lightPos - vs_out.TangentFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(lightPos - vs_out.TangentFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(vs_out.TBN * -light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, vs_out.TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, vs_out.TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, vs_out.TexCoord));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}


// calculates the color when using a directional light.
vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, MixedMaterial material)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, MixedMaterial material)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, MixedMaterial material)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
