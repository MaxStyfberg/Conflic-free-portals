#version 150

in  vec3 in_Position;
in vec3 in_Normal;
out vec3 Color;
uniform mat4 total, view;
uniform mat4 projectionMatrix;
in vec2 inTexCoord;
out vec2 TexCoord;

void main(void)
{
	mat3 normalMatrix1 = mat3(total);
	mat3 normalMatrix3 = mat3(view);
	vec3 transformedNormal = normalMatrix3 *normalMatrix1 * in_Normal;
	const vec3 light = vec3(0.58,0.58,0.58);
	float shade;
 	shade = dot(normalize(transformedNormal),light);
	TexCoord = inTexCoord;
	Color = vec3(shade);
	gl_Position = projectionMatrix*view*total*vec4(in_Position, 1.0);
}
