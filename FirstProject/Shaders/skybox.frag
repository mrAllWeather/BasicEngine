#version 330

in vec3 texCoords;

uniform samplerCube texMap;

out vec4 fragColour;

void main(void) {

	fragColour = texture(texMap, texCoords);
}
