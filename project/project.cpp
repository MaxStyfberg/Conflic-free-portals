// Lab 4, terrain generation

// uses framework Cocoa
// uses framework OpenGL
#define MAIN
#include "lodepng.h"
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include<math.h>
#define radians(deg) ((deg) * (M_PI / 180.0f))

#define MAX_VERTICES 1000
#define MAX_INDICES 3000
#define MAX_ROOMS 10
#define modelno 4

typedef struct {
    vec3 a, b, c, d; // 4 corners
	mat4 trans;
    mat4 move;
	mat4 rot;
	vec3 center;
	int dest; // Which cell does it lead to?
	Model *model;
} Portal;
typedef struct {
    vec3 vertices[MAX_VERTICES];
    vec3 normals[MAX_VERTICES];
    vec2 texCoords[MAX_VERTICES];
    vec3 colors[MAX_VERTICES];
    GLuint indices[MAX_INDICES];
    vec2 floorOutline[MAX_VERTICES];
    int floorVertCount;
    int portalCount = 0;
    Portal *portals;
    GLuint walltex;
    GLuint floortex;
    GLuint rooftex;
    vec3 lightPos;
} Room;



mat4 projectionMatrix;



// Function to update sphere position

// vertex array object
vec3 vertex[4];
Room rooms[MAX_ROOMS];
Portal portals[MAX_ROOMS];
//int portalCount = 0;
int roomCount = 0;
int currentCell = 1;
GLuint program,portalShader;
Model *roommodels[modelno];
Model *portalmodels[modelno];
Model *m, *m2, *tm, *sphere;
GLuint wall2,wall3,wall1,floor3, roof, floor1, floor2;
// Reference to shader program
float rotateX = 30;
float rotateY = 8;
float rotateZ = 0;
float sphereX = 5.0f, sphereZ = 5.0f, sphereY = 0.0f;
float velocityX = 0.02f, velocityZ = 0.01f; // Speed of movement
vec3 cameraPos = {10.0f, 0.0f, 5.0f};  // Camera's position
vec3 cameraFront = {0.0f, 0.0f, -1.0f}; // Direction camera is facing
vec3 cameraUp = {0.0f, 1.0f, 0.0f}; // Up direction

float yaw = -90.0f; // Left/right rotation
float pitch = 0.0f; // Up/down rotation
float cameraSpeed = 1.0f;

float lastX = 300, lastY = 300;
bool firstMouse = true;
float sensitivity = 0.1f;

void LoadRoomsFromFile(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    char line[256];
    Room currentRoom;
    memset(&currentRoom, 0, sizeof(Room));
    int state = 0;
    int i = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "ROOM", 4) == 0)
        {
            roomCount++;
        }
        else if (strncmp(line, "VERTICES", 8) == 0){
            i = 0;
            state = 1;
        }
        else if (strncmp(line, "NORMALS", 7) == 0){
            i = 0;
            state = 2;
        }  
        else if (strncmp(line, "TEXCOORDS", 9) == 0){
            i = 0;
            state = 3;
        }
        else if (strncmp(line, "INDICES", 7) == 0){
            i = 0;
            state = 4;
        }
        else
        {
            float x, y, z;
            if (state == 1 && sscanf(line, "%f %f %f", &x, &y, &z) == 3){
                if(roomCount==2){
                    rooms[roomCount].vertices[i] = vec3(x,y,z) + vec3(30,0,-20);
                }
                else if(roomCount ==3){
                    rooms[roomCount].vertices[i] = vec3(x,y,z) + vec3(80,0,-20);
                }
                else{
                rooms[roomCount].vertices[i] = vec3(x,y,z);
               
                }
                if(y == -10){
                    //rooms[roomCount].floorOutline[rooms[roomCount].floorVertCount++] = vec2(x,z);
                }
                i++;
            }
            else if (state == 2 && sscanf(line, "%f %f %f", &x, &y, &z) == 3){
                rooms[roomCount].normals[i] = vec3(x, y, z);
                i++;
            }
            else if (state == 3 && sscanf(line, "%f %f", &x, &y) == 2){
                rooms[roomCount].texCoords[i] = vec2(x, y);
                i++;  
            }
            else if (state == 4) {
                char* token = strtok(line, " \t\n");
                while (token != NULL){ 
                    int index;
                    if (sscanf(token, "%d", &index) == 1)
                    {
                        rooms[roomCount].indices[i++] = index;
                    }
                token = strtok(NULL, " \t\n");
                }
            }
        }  
    }
    fclose(file);
    printf("max(%.2f, %.2f,)\n", rooms[1].floorOutline[0].x, rooms[1].floorOutline[0].y);

    printf("Model created for room %d\n", rooms[1].floorVertCount);
}

bool isInsideRoom(Room* room, vec2 point) {
    int i, j, c = 0;
    for (i = 0, j = room->floorVertCount - 1; i < room->floorVertCount; j = i++) {
        vec2 vi = room->floorOutline[i];
        vec2 vj = room->floorOutline[j];
        if (((vi.y > point.y) != (vj.y > point.y)) &&
            (point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x))
            c = !c;
    }
    return c;
}
bool intersectsPortal(Portal* p, vec3 camPos) {
    vec3 u = VectorSub(p->b, p->a); // vertical
    vec3 v = VectorSub(p->c, p->a); // horizontal
    vec3 n = normalize(cross(u, v)); // portal normal

    float d = DotProduct(n, VectorSub(camPos, p->a));
    if (fabs(d) > 1.0f) return false; // Allow small tolerance

    // Project to local 2D portal space
    vec3 ap = VectorSub(camPos, p->a);
    float u_len = Norm(u);
    float v_len = Norm(v);
    float proj_u = DotProduct(ap, normalize(u)) / u_len;
    float proj_v = DotProduct(ap, normalize(v)) / v_len;

    return proj_u >= 0.0f && proj_u <= 1.0f && proj_v >= 0.0f && proj_v <= 1.0f;
}

void moveCamera() {
    vec3 right = normalize(cross(cameraFront, cameraUp));
    float rotationSpeed = 2.0f;
    vec3 movement = {0.0f, 0.0f, 0.0f};

    // Accumulate movement input
    if (glutKeyIsDown('w'))
        movement = VectorAdd(movement, ScalarMult(cameraFront, cameraSpeed));
    if (glutKeyIsDown('s'))
        movement = VectorSub(movement, ScalarMult(cameraFront, cameraSpeed));
    if (glutKeyIsDown('a'))
        movement = VectorSub(movement, ScalarMult(right, cameraSpeed));
    if (glutKeyIsDown('d'))
        movement = VectorAdd(movement, ScalarMult(right, cameraSpeed));
    if (glutKeyIsDown('q'))
        movement = VectorAdd(movement, ScalarMult(cameraUp, cameraSpeed));
    if (glutKeyIsDown('e'))
        movement = VectorSub(movement, ScalarMult(cameraUp, cameraSpeed));

    // Store old position before applying movement
    vec3 oldPos = cameraPos;
    cameraPos = VectorAdd(cameraPos, movement);

// Lock Y position
    cameraPos.y = 0.0f;
    
    //printf("location %f\n%f\n%f\n ", cameraPos.x,cameraPos.y,cameraPos.z);
    // Only keep the move if it's within the current room's 2D boundary
    vec2 pos2D = { cameraPos.x, cameraPos.z };
    if (!isInsideRoom(&rooms[currentCell], pos2D)) {;
        cameraPos = oldPos;
    } 
 
    if (glutKeyIsDown(GLUT_KEY_LEFT))  // Rotate left
        yaw -= rotationSpeed;
    if (glutKeyIsDown(GLUT_KEY_RIGHT)) // Rotate right
        yaw += rotationSpeed;
    if (glutKeyIsDown(GLUT_KEY_UP))    // Look up
        pitch += rotationSpeed;
    if (glutKeyIsDown(GLUT_KEY_DOWN))  // Look down
        pitch -= rotationSpeed;

    // Clamp pitch to avoid flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update cameraFront vector based on yaw and pitch
    vec3 front;
    front.x = cos(radians(yaw)) * cos(radians(pitch));
    front.y = sin(radians(pitch));
    front.z = sin(radians(yaw)) * cos(radians(pitch));
    cameraFront = normalize(front);
    for (int i = 0; i < rooms[currentCell].portalCount; i++) {
        Portal* p = &rooms[currentCell].portals[i];
        if (intersectsPortal(p, cameraPos)) {
        if(currentCell != 3){
            cameraPos = cameraPos- rooms[currentCell].portals[i].center;
            cameraPos = rooms[currentCell].portals[i].rot * cameraPos;
		    cameraPos = cameraPos + rooms[currentCell].portals[i].center;
														// Then translate to portal destination
		    cameraPos = rooms[currentCell].portals[i].move * cameraPos;
		    movement = rooms[currentCell].portals[i].rot * movement;
            cameraFront = normalize(rooms[currentCell].portals[i].rot * cameraFront);
        }
        else{

        // Apply portal transform to camera position
            Portal* destPortal = &rooms[p->dest].portals[0]; // assumes it's always portal 0,change if needed

            // Get relative position to entry portal
            vec3 localOffset = VectorSub(cameraPos, p->a); // more consistent than using center

            // Transform to destination portal space
            vec3 offsetRotated = p->rot * localOffset;    // rotate from source to dest frame
            vec3 offsetMoved = p->move * offsetRotated;   // apply movement
            cameraPos = VectorAdd(destPortal->a, offsetMoved);
            }
        currentCell = p->dest;
        break;
        }
    }
}

void addPortal(int cell, int toCell, vec3 a, vec3 b, vec3 c, vec3 d, mat4 trans, mat4 rot, mat4 move)
{
   
	int portalCount = rooms[cell].portalCount;
	if (rooms[cell].portals == NULL)
		rooms[cell].portals = (Portal *) malloc(sizeof(Portal));
	else
		rooms[cell].portals = (Portal *) realloc(rooms[cell].portals, (portalCount+1)*sizeof(Portal));
		
	rooms[cell].portals[portalCount].a = a;
	rooms[cell].portals[portalCount].b = b;
	rooms[cell].portals[portalCount].c = c;
	rooms[cell].portals[portalCount].d = d;
	rooms[cell].portals[portalCount].trans = trans;
	rooms[cell].portals[portalCount].rot = rot;
    rooms[cell].portals[portalCount].move = move;
	rooms[cell].portals[portalCount].center = vec3((a.x+b.x+c.x+d.x)/4, 0, (a.z + b.z + c.z + d.z)/4);

	rooms[cell].portals[portalCount].dest = toCell;


	// Create portal for drawing
	vec3 verticesp[] = {rooms[cell].portals[portalCount].a, rooms[cell].portals[portalCount].b, rooms[cell].portals[portalCount].d, rooms[cell].portals[portalCount].c};
	GLfloat texcoordp[] = {	M_PI, M_PI,
						0.0f, M_PI,
						0.0f, 0.0f,
						M_PI, 0.0f};
	GLuint indicesp[] = {0,2,1, 2, 0, 3};
	rooms[cell].portals[portalCount].model = LoadDataToModel((vec3 *)verticesp, NULL, (vec2 *)texcoordp, NULL,
			indicesp, 4, 6);


	 rooms[cell].portalCount += 1;
     printf("Portalcount %d\n", rooms[cell].portalCount);
}

void init(void)
{
	// GL inits
	glClearColor(0.4,0.6,0.9,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	program = loadShaders("project.vert", "project.frag");
    portalShader = loadShaders("portal.vert", "portal.frag");
	glUseProgram(program);
	printError("init shader");
	
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "tex1"), 1); // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "tex2"), 2); // Texture unit 0

	glUseProgram(portalShader);
	glUniformMatrix4fv(glGetUniformLocation(portalShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(portalShader, "tex"), 0); // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "tex1"), 1); // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "tex2"), 2); // Texture unit 0

	LoadTGATextureSimple("ConcreteWallPainted-HR_64.tga", &wall2);
	LoadTGATextureSimple("ConcreteWall-Hazard-Full-01_64.tga", &wall3);
	LoadTGATextureSimple("ConcreteWallPainted-H2FB_64.tga", &wall1);
	LoadTGATextureSimple("ConcreteFloorPainted-H2FY_64.tga", &floor3);
    LoadTGATextureSimple("ConcreteFloorPainted-Cb16x16B_64.tga", &roof);
    LoadTGATextureSimple("ConcreteFloor-Hazard-Full-01_64.tga", &floor1);
    LoadTGATextureSimple("ConcreteFloorPainted-HR_64.tga", &floor2);

	glBindTexture(GL_TEXTURE_2D, wall2);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, wall3);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);
    

    LoadRoomsFromFile("rooms.txt");

   
    
    for(int i=1; i<=roomCount; i++){
        roommodels[i] = LoadDataToModel(rooms[i].vertices, rooms[i].normals, rooms[i].texCoords, rooms[i].colors, rooms[i].indices, sizeof(rooms[i].vertices), sizeof(rooms[i].indices));
   
    }
    rooms[1].walltex = wall1;
    rooms[1].floortex = floor1;
    rooms[2].walltex = wall2;
    rooms[2].floortex = floor2;
    rooms[3].walltex = wall3;
    rooms[3].floortex = floor3;
    rooms[1].floorVertCount = 6;
    rooms[1].floorOutline[0] = vec2 (0,0);
    rooms[1].floorOutline[1] = vec2 (0,20);
    rooms[1].floorOutline[2] = vec2 (40,20);
    rooms[1].floorOutline[3] = vec2 (40,-20);
    rooms[1].floorOutline[4] = vec2 (-30,-20);
    rooms[1].floorOutline[5] = vec2 (-30,0);
    rooms[2].floorVertCount = 6;
    rooms[2].floorOutline[0] = vec2 (30,-20);
    rooms[2].floorOutline[1] = vec2 (90,-20);
    rooms[2].floorOutline[2] = vec2 (90,-60);
    rooms[2].floorOutline[3] = vec2 (20, -60);
    rooms[2].floorOutline[4] = vec2 (20, -50);
    rooms[2].floorOutline[5] = vec2 (30, -50);
    rooms[3].floorVertCount = 6;
    rooms[3].floorOutline[0] = vec2 (80,-20);
    rooms[3].floorOutline[1] = vec2 (80, 10);
    rooms[3].floorOutline[2] = vec2 (90, 10);
    rooms[3].floorOutline[3] = vec2 (90, 30);
    rooms[3].floorOutline[4] = vec2 (170, 30);
    rooms[3].floorOutline[5] = vec2 (170, -20);
    rooms[1].lightPos = vec3(20, 9,0);
    rooms[2].lightPos = vec3(55, 9,-40);
    rooms[3].lightPos = vec3(125,9,0);
    addPortal(1,	// Add to this cell
		2,			// Destination cell
		vec3(30,-10, -20), // Four corners of portal. Must be a square.
		vec3(30, 10, -20),
		vec3(40,-10, -20),
		vec3(40, 10, -20),
		T(0,0,0), // Translation part of portal transformation
		IdentityMatrix(),
        T(0,0,-1.5));

   addPortal(2,	// Add to this cell
		1,			// Destination cell
		vec3(30,-10, -20), // Four corners of portal. Must be a square.
		vec3(30, 10, -20),
		vec3(40,-10, -20),
		vec3(40, 10, -20),
		T(0,0,0), // Translation part of portal transformation
		Ry(M_PI),
        T(0,0, 1.5));
    
    addPortal(2,	// Add to this cell
		3,			// Destination cell
		vec3(90,-10, -20), // Four corners of portal. Must be a square.
		vec3(90, 10, -20),
		vec3(80,-10, -20),
		vec3(80, 10, -20),
		T(0, 0, 0), // Translation part of portal transformation
		IdentityMatrix(),
        T(0,0,1.5));

    addPortal(3,	// Add to this cell
		2,			// Destination cell
		vec3(90,-10, -20), // Four corners of portal. Must be a square.
		vec3(90, 10, -20),
		vec3(80,-10, -20),
		vec3(80, 10, -20),
		T(50,0,-20), // Translation part of portal transformation
		Ry(M_PI),
        T(50,0,0));
 

    currentCell = 1;
	printError("init terrain");

}



void DrawCell(int currentCell, int fromCell, mat4 worldToView, mat4 modelToWorld, int count) // Draw cells recursively! Needed for the semitransparent portal!
{
	mat4 m,trans;
    bool visited[MAX_ROOMS] = { false };
	
    if(count <2){
	    if (!visited[currentCell]){
        visited[currentCell] = true;
	for (int i = 0; i < rooms[currentCell].portalCount; i++)
	{
		if (rooms[currentCell].portals[i].dest != fromCell) // Do not go back!
		{
			vec3 c2 = rooms[currentCell].portals[i].center; // Current portal center
			c2 = rooms[currentCell].portals[i].trans * c2; // Translate to next
			
			mat4 tm = T(-c2.x, -c2.y, -c2.z); // Translation portal destination to origin
			mat4 m = inverse(rooms[currentCell].portals[i].rot) * tm; // Apply rotation
			
			// Then translate to center
			vec3 c1 = rooms[currentCell].portals[i].center; // Nuvarande portal
			mat4 tf = T(c1.x, c1.y, c1.z); // Translation portal destination to current
			m = tf * m;
            mat4 newModelToWorld = Mult(modelToWorld, m);
            printf("dest %d\n ", rooms[currentCell].portals[i].dest );
            printf("location %f\n ", rooms[currentCell].portals[i].center.x );
            printf("count %d\n", count);
			DrawCell(rooms[currentCell].portals[i].dest, currentCell, worldToView, newModelToWorld, count+1);
		}
	}
}
}
		
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	  
	glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rooms[currentCell].walltex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rooms[currentCell].floortex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, roof);
    vec3 lightcolour = vec3(1.0f,1.0f,1.0f);
    glUniform3fv(glGetUniformLocation(program, "lightPos"), 1, &rooms[currentCell].lightPos.x);
    glUniform3fv(glGetUniformLocation(program, "lightColour"), 1, &lightcolour.x);
    glUniform3fv(glGetUniformLocation(program, "CamDir"), 1, &cameraPos.x);
    trans = T(0,0,0);//rooms[currentCell].portals[0].center.x, rooms[currentCell].portals[0].center.y, rooms[currentCell].portals[0].center.z); // Move rooms side by side
    glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, trans.m);
    DrawModel(roommodels[currentCell], program, "in_Position", "in_Normal", "inTexCoord");
    
        
    
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

    glUseProgram(portalShader);
    glUniformMatrix4fv(glGetUniformLocation(portalShader, "total"), 1, GL_TRUE, modelToWorld.m);


//	glUniformMatrix4fv(glGetUniformLocation(portalShader, "modelviewMatrix"), 1, GL_TRUE, Mult(worldToView, modelToWorld).m);
	for (int i = 0; i < rooms[currentCell].portalCount; i++)
	{
		DrawModel(rooms[currentCell].portals[i].model, portalShader, "in_Position", "in_Normal", "inTexCoord");
	}

}

void display(void)
{
	
   
    // clear the screen
    moveCamera();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mat4 total, modelToWorld, worldToView, trans;
	
	printError("pre display");
	
	glUseProgram(program);

	// Build matrix
	
	vec3 cam = vec3(0, 5, 8);
	vec3 lookAtPoint = vec3(2, 0, 2);
	worldToView = lookAt(cameraPos.x, cameraPos.y, cameraPos.z,
                         cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z,
                         cameraUp.x, cameraUp.y, cameraUp.z);
     glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, 
     GL_TRUE, worldToView.m);

    glUseProgram(portalShader);

    glUniformMatrix4fv(glGetUniformLocation(portalShader, "view"), 1, 
     GL_TRUE, worldToView.m);

    glUseProgram(program);
	modelToWorld = IdentityMatrix();
	total = worldToView * modelToWorld;
    DrawCell(currentCell, -1, worldToView, IdentityMatrix(), 0); // Draw cells recursively! Needed for the semitransparent portal!
  
 
	printError("display 2");
	
	glutSwapBuffers();
}

void mouse(int x, int y) {
    /*if (firstMouse) { // Prevent sudden jump when starting
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float dx = x - lastX;
    float dy = lastY - y; // Invert Y-axis for natural feel
    lastX = x;
    lastY = y;

    yaw += dx * sensitivity;
    pitch += dy * sensitivity;

    // Clamp pitch to avoid flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update cameraFront vector based on yaw and pitch
    vec3 front;
    front.x = cos(radians(yaw)) * cos(radians(pitch));
    front.y = sin(radians(pitch));
    front.z = sin(radians(yaw)) * cos(radians(pitch));
    cameraFront = normalize(front);*/
}
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH );
	glutInitContextVersion(3, 2);
	glutInitWindowSize (600, 600);
	glutCreateWindow ("TSBK07 Lab 4");
	glutDisplayFunc(display);
	init ();
	glutRepeatingTimer(20);
	
	glutPassiveMotionFunc(mouse);

	glutMainLoop();
	exit(0);
}

