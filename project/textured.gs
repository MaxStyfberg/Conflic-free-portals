// Yet another use for geometry shader:
// Rescale texture coordinates depending on size
// This is done knowing that we have axis aligned boxes!
// Could I do it with arbitrary polygons? Even create proper ST axes? Bump mapping without pre-processing? Yes, I can.

#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 texCoord[3];   //[3] because this makes a triangle
in vec3 exNormal[3];
out vec2 texCoordG;
out vec3 exNormalG;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 scaling;

void main()
{
  // HOW CAN I SCALE THE TEXTURE PROPERLY?

// Irrelevant code: Make AABB for triangle.
//  float maxx, minx, maxy, miny, maxz, minz;
//  maxx = gl_in[0].gl_Position.x;
//  maxy = gl_in[0].gl_Position.y;
//  maxz = gl_in[0].gl_Position.z;
//  minx = gl_in[0].gl_Position.x;
//  miny = gl_in[0].gl_Position.y;
//  minz = gl_in[0].gl_Position.z;
//  for(int i = 1; i < gl_in.length(); i++)
//  {
//    maxx = max(maxx, gl_in[i].gl_Position.x);
//    maxy = max(maxy, gl_in[i].gl_Position.y);
//    maxz = max(maxz, gl_in[i].gl_Position.z);
//    minx = min(minx, gl_in[i].gl_Position.x);
//    miny = min(miny, gl_in[i].gl_Position.y);
//    minz = min(minz, gl_in[i].gl_Position.z);
//  }
//  float dx = maxx - minx;
//  float dy = maxy - miny;
//  float dz = maxz - minz;


  vec2 scale = vec2(1, 1);

  for(int i = 0; i < gl_in.length(); i++)
  {
    // We know that each triangle has two AA sides and one diagonal. Find the ones with only one non-zero difference.
    
    int j = (i+1) % gl_in.length();
	float dxds = (gl_in[j].gl_Position.x - gl_in[i].gl_Position.x) * (texCoord[j].s - texCoord[i].s);
	float dyds = (gl_in[j].gl_Position.y - gl_in[i].gl_Position.y) * (texCoord[j].s - texCoord[i].s);
	float dzds = (gl_in[j].gl_Position.z - gl_in[i].gl_Position.z) * (texCoord[j].s - texCoord[i].s);
	
	// Two zeroes - scale s with the third
	if (abs(dxds) < 0.1 && abs(dyds) < 0.1 && abs(dzds) > 0.5) scale.s = scaling.z; // abs(gl_in[j].gl_Position.z - gl_in[i].gl_Position.z); // zds = 1;
	if (abs(dyds) < 0.1 && abs(dzds) < 0.1 && abs(dxds) > 0.5) scale.s = scaling.x; // abs(gl_in[j].gl_Position.x - gl_in[i].gl_Position.x); // xds = 1;
	if (abs(dxds) < 0.1 && abs(dzds) < 0.1 && abs(dyds) > 0.5) scale.s = scaling.y; // abs(gl_in[j].gl_Position.y - gl_in[i].gl_Position.y); // yds = 1;
//	// Two zeroes - scale s with the third
	
	float dxdt = (gl_in[j].gl_Position.x - gl_in[i].gl_Position.x) * (texCoord[j].t - texCoord[i].t);
	float dydt = (gl_in[j].gl_Position.y - gl_in[i].gl_Position.y) * (texCoord[j].t - texCoord[i].t);
	float dzdt = (gl_in[j].gl_Position.z - gl_in[i].gl_Position.z) * (texCoord[j].t - texCoord[i].t);

  	if (abs(dxdt) < 0.1 && abs(dydt) < 0.1 && abs(dzdt) > 0.5) scale.t = scaling.z; // abs(gl_in[j].gl_Position.z - gl_in[i].gl_Position.z); // zdt = 1;
	if (abs(dydt) < 0.1 && abs(dzdt) < 0.1 && abs(dxdt) > 0.5) scale.t = scaling.x; // abs(gl_in[j].gl_Position.x - gl_in[i].gl_Position.x); // xdt = 1;
	if (abs(dxdt) < 0.1 && abs(dzdt) < 0.1 && abs(dydt) > 0.5) scale.t = scaling.y; // abs(gl_in[j].gl_Position.y - gl_in[i].gl_Position.y); // ydt = 1;
  }
  
// Uncomment for original, bad stretching.
//  scale = vec2(1, 1);
  
  for(int i = 0; i < gl_in.length(); i++)
  {
	gl_Position = projectionMatrix * modelviewMatrix * gl_in[i].gl_Position;
    texCoordG = texCoord[i] * scale;
    exNormalG = exNormal[i];
    EmitVertex();
  }
  EndPrimitive();
}
