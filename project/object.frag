#version 150
in vec3 Color;
out vec4 out_Color;
in vec2 TexCoord;
uniform sampler2D tex;
in float worldPos;
in vec3 fragment_position;
uniform vec3 lightPos;
uniform vec3 lightColour;
uniform mat4 total, view;
uniform vec3 CamDir;
void main(void)
{
     vec3 diffuse;
     vec3 spec;
     vec3 result;
     vec3 normal = normalize(Color);
     vec3 reflective;
     vec3 total2= vec3(0,0,0);
     vec3 direction;
     vec3 transform;
     vec3 camDir = mat3(view)*CamDir;
     transform = vec3(view*vec4(lightPos,1.0));
     direction = normalize(transform-fragment_position);
        
     float theta = max(dot(direction,normal),0.0);
     diffuse = 0.8*lightColour*theta;

     reflective = reflect(-direction,normal);
     vec3 viewDirection = normalize(camDir-fragment_position);

     float fi = max(dot(reflective, normalize(viewDirection)),0.0);
     spec = 0.9*lightColour*pow(fi,60.0);
            
     result = diffuse+spec;
     total2+= result;
     out_Color = vec4(total2, 1.0) * texture(tex, TexCoord);

 
}
