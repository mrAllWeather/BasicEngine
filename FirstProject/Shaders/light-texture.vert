#version 330 core

#define MAX_LIGHTS 16

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

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

uniform mat4 model;
uniform mat4 component;
uniform mat4 object;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;
uniform Light light [ MAX_LIGHTS ];

out Vert {
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;

	vec3 TangentLightPos [ MAX_LIGHTS ];
	vec3 TangentViewPos;
	vec3 TangentFragPos;
} vs_out;



void main()
{
	vs_out.TexCoord = vec2(texCoord.x, texCoord.y);
	vs_out.Normal = mat3(transpose(inverse(object * component * model))) * normal;
	vs_out.FragPos = vec3(object * component * model * vec4(position, 1.0f));

	// https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping
	mat3 normalMatrix = transpose(inverse(mat3(object * component * model)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		vs_out.TangentLightPos[i] = TBN * light[i].position;
	}

	gl_Position = projection * view * object * component * model * vec4(position, 1);
}
