#version 150

out vec4 outColor;

in vec2 texCoord;
//in vec3 exNormal;
uniform sampler2D tex1;
uniform mat4 modelviewMatrix;

void main(void)
{
	// texcoord needs scaling with the scaling of modelviewMatrix! BUT it has to be in model coordinates!
//	vec4 t = texture(tex, texCoord); //  * vec2(modelviewMatrix[0][0], modelviewMatrix[1][1]));
//	vec3 n = normalize(exNormal);
//	vec3 light = vec3(0.7, 0.7, 0.7);
//	float shade = dot(n, normalize(mat3(modelviewMatrix) * light))*0.4 + 0.7;
//	if (t.a < 0.01) discard;
//	else
	float shade = sqrt(sin(texCoord.s)*sin(texCoord.t));// * sin(texCoord.s)*sin(texCoord.t);
    vec3 blue = vec3(0.0, 0.0, 1.0);
	outColor =  vec4(blue,1.0);
}
