#version 150

in  vec3 inPosition;
//in  vec3 inColor;
in  vec3 inNormal;
in vec2 inTexCoord;
//uniform vec3 scale;

//out vec3 exColor; // Gouraud
out vec3 exNormal; // Phong
out vec2 texCoord;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

void main(void)
{
	texCoord = inTexCoord;
	exNormal = mat3(modelviewMatrix) * inNormal;
	
//	gl_Position = projectionMatrix * modelviewMatrix * vec4(inPosition, 1.0);
	gl_Position = vec4(inPosition, 1.0);
}
