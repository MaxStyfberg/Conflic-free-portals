#version 150
in vec3 Color;
out vec4 out_Color;
in vec2 TexCoord;
uniform sampler2D tex;

void main(void)
{
	float shade = sqrt(sin(TexCoord.s)*sin(TexCoord.t));// * sin(texCoord.s)*sin(texCoord.t);
	out_Color = vec4(1.0, shade, shade, 1-shade);
}
