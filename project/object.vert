#version 150

in  vec3 in_Position;
in vec3 in_Normal;
out vec3 Color;
uniform mat4 total, view;
uniform mat4 projectionMatrix;
in vec2 inTexCoord;
out vec2 TexCoord;
out float worldPos;
out vec3 fragment_position;

void main(void)
{
	mat3 normalMatrix = transpose(inverse(mat3(view*total)));
	Color = mat3(view)*mat3(total)*in_Normal;
    vec4 viewpos = view*total*vec4(in_Position,1.0);
    fragment_position = viewpos.xyz;
	TexCoord = inTexCoord;
    worldPos = in_Position.y;
	gl_Position = projectionMatrix*view*total*vec4(in_Position, 1.0);
}
