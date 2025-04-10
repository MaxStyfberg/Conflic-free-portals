#version 150

out vec4 outColor;
in vec2 texCoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
in vec3 Normal;
in float height;
in vec2 Pos;

const vec2 craterCenter = vec2(95.0, 150.0);
const float craterRadius = 56.0;
const float lakeThreshold = 2.0;

void main(void)
{
    const vec3 light = vec3(0.58,0.58,0.58);
    float shade;
    shade = dot(normalize(Normal),light);

    const float fadeWidth = 15.0;  // Controls the smooth edge width

    float maxHeight = 10.0;
    float minHeight = 5.0;
    float blendFactor = clamp((height - minHeight) / (maxHeight - minHeight), 0.0, 1.0);
    vec4 texColor1 = texture(tex1, texCoord);
    vec4 texColor2 = texture(tex2, texCoord);

    vec2 craterOffset = Pos - craterCenter;
    float distance = length(craterOffset);
    float fadeFactor = smoothstep(craterRadius, craterRadius - fadeWidth, distance);
    if (height < 2.0 && distance < craterRadius) {
    vec4 waterColor = vec4(0.0, 0.3, 0.7, 1.0);
    outColor = mix(texColor1, waterColor, fadeFactor);
    } else {
    outColor = mix(texColor1, texColor2, blendFactor)*vec4(shade,shade,shade,1.0);
    }
}
