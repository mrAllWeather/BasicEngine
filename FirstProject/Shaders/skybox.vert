#version 330

layout (location = 0) in vec3 tex_ver;

uniform mat4 projection;
uniform mat4 view;

out vec3 texCoords;

void main(void) {
	
	texCoords = tex_ver;
    vec4 pos = projection * view * vec4(tex_ver, 1.0);
    gl_Position = pos.xyww;

}
