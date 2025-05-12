#version 150
in vec3 Color;
out vec4 out_Color;
in vec2 TexCoord;
uniform sampler2D tex;
uniform sampler2D tex1;
uniform sampler2D tex2;
in float worldPos;
void main(void)
{
    if(worldPos <= -9.99f){
        out_Color = texture(tex1, TexCoord);
    }
    else if(worldPos >= 9.99f){
        out_Color = texture(tex2, TexCoord);
    }
    else{
        out_Color = texture(tex, TexCoord);
	}
}
