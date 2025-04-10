#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;
out vec2 texCoord;
out vec3 Normal;
out float height;
out vec2 Pos;

// NY
uniform mat4 projMatrix;
uniform mat4 mdlMatrix;

void main(void)
{
    mat3 normalMatrix1 = mat3(mdlMatrix);
    texCoord = inTexCoord;
    gl_Position = projMatrix * mdlMatrix * vec4(inPosition, 1.0);
    Normal = inNormal;
    height = inPosition.y;
    Pos = inPosition.xz;
}
