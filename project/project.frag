#version 150
in vec3 Color;
out vec4 out_Color;
in vec2 TexCoord;
uniform sampler2D texUnit;

void main(void)
{
	out_Color = vec4(Color,1.0)*texture(texUnit, TexCoord);
}
