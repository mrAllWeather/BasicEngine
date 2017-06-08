#version 330

uniform mat4 mvp_matrix;

layout (location = 0) in vec3 tex_ver;

uniform mat4 projection;
uniform mat4 view;

out vec3 texCoords;

void main(void) {
	
	texCoords = tex_ver;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;

}
