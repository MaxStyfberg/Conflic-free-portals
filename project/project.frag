#version 150
in vec3 Color;
out vec4 out_Color;
in vec2 TexCoord;
uniform sampler2D tex1;

void main(void)
{
	out_Color = texture(tex1, TexCoord);
}
