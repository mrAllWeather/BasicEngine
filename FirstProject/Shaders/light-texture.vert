#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;

uniform mat4 model;
uniform mat4 component;
uniform mat4 object;
uniform mat4 view;
uniform mat4 projection;

out Vert {
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;
} vs_out;

void main()
{
	vs_out.TexCoord = vec2(texCoord.x, texCoord.y);
	vs_out.Normal = mat3(transpose(inverse(object * component * model))) * normal;
	vs_out.FragPos = vec3(object * component * model * vec4(position, 1.0f));

	gl_Position = projection * view * object * component * model * vec4(position, 1);
}
