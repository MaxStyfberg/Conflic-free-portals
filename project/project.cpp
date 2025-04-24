// Lab 4, terrain generation

// uses framework Cocoa
// uses framework OpenGL
#define MAIN
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
    /*vec3 vertices[MAX_VERTICES];
    vec3 normals[MAX_VERTICES];
    vec2 texCoords[MAX_VERTICES];
    vec3 colors[MAX_VERTICES];
    GLuint indices[MAX_INDICES];*/
    vec3 a, b, c, d; // 4 corners
	mat4 trans;
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
    int portalCount = 0;
    Portal *portals;
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
GLuint tex1, tex2;
// Reference to shader program
float rotateX = 30;
float rotateY = 8;
float rotateZ = 0;
float sphereX = 5.0f, sphereZ = 5.0f, sphereY = 0.0f;
float velocityX = 0.02f, velocityZ = 0.01f; // Speed of movement
vec3 cameraPos = {0.0f, 5.0f, 8.0f};  // Camera's position
vec3 cameraFront = {0.0f, 0.0f, -1.0f}; // Direction camera is facing
vec3 cameraUp = {0.0f, 1.0f, 0.0f}; // Up direction

float yaw = -90.0f; // Left/right rotation
float pitch = 0.0f; // Up/down rotation
float cameraSpeed = 3.0f;

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
                rooms[roomCount].vertices[i] = vec3(x,y,z);
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
    printf("Model created for room %d\n", roomCount);
}
/*void LoadPortalsFromFile(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    char line[256];
    Room currentPortal;
    memset(&currentPortal, 0, sizeof(Portal));
    int state = 0;
    int i = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "PORTAL", 4) == 0)
        {
            portalCount++;
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
                portals[portalCount].vertices[i] = vec3(x,y,z);
                i++;
            }
            else if (state == 2 && sscanf(line, "%f %f %f", &x, &y, &z) == 3){
                portals[portalCount].normals[i] = vec3(x, y, z);
                i++;
            }
            else if (state == 3 && sscanf(line, "%f %f", &x, &y) == 2){
                portals[portalCount].texCoords[i] = vec2(x, y);
                i++;  
            }
            else if (state == 4) {
                char* token = strtok(line, " \t\n");
                while (token != NULL){ 
                    int index;
                    if (sscanf(token, "%d", &index) == 1)
                    {
                        portals[portalCount].indices[i++] = index;
                    }
                token = strtok(NULL, " \t\n");
                }
            }
        }  
    }
    fclose(file);
}*/
void moveCamera() {
    vec3 right = normalize(cross(cameraFront, cameraUp));
    float rotationSpeed = 2.0f;

    if (glutKeyIsDown('w')) // Move forward
        cameraPos = VectorAdd(cameraPos, ScalarMult(cameraFront, cameraSpeed));
    if (glutKeyIsDown('s')) // Move backward
        cameraPos = VectorSub(cameraPos, ScalarMult(cameraFront, cameraSpeed));
    if (glutKeyIsDown('a')) // Move left (strafe)
        cameraPos = VectorSub(cameraPos, ScalarMult(right, cameraSpeed));
    if (glutKeyIsDown('d')) // Move right (strafe)
        cameraPos = VectorAdd(cameraPos, ScalarMult(right, cameraSpeed));
    if (glutKeyIsDown('q')) // Move up
        cameraPos = VectorAdd(cameraPos, ScalarMult(cameraUp, cameraSpeed));
    if (glutKeyIsDown('e')) // Move down
        cameraPos = VectorSub(cameraPos, ScalarMult(cameraUp, cameraSpeed));

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
}

void addPortal(int cell, int toCell, vec3 a, vec3 b, vec3 c, vec3 d, mat4 trans, mat4 rot)
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
	rooms[cell].portals[portalCount].center = vec3((a.x+b.x+c.x+d.x)/4, 0, (a.z + b.z + c.z + d.z)/4);

	rooms[cell].portals[portalCount].dest = toCell;


	// Create portal for drawing
	vec3 verticesp[] = {rooms[cell].portals[portalCount].a, rooms[cell].portals[portalCount].b, rooms[cell].portals[portalCount].d, rooms[cell].portals[portalCount].c};
	GLfloat texcoordp[] = {	M_PI, M_PI,
						0.0f, M_PI,
						0.0f, 0.0f,
						M_PI, 0.0f};
	GLuint indicesp[] = {0,2,1, 2,0, 3};
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
	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 5000.0);

	// Load and compile shader
	program = loadShaders("project.vert", "project.frag");
    portalShader = loadShaders("portal.vert", "portal.frag");
	glUseProgram(program);
	printError("init shader");
	
	glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
    
    LoadTGATextureSimple("grass.tga", &tex1);
    glUniform1i(glGetUniformLocation(program, "tex1"), 0); // Set texture unit 0
	glUseProgram(portalShader);
	glUniformMatrix4fv(glGetUniformLocation(portalShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(portalShader, "tex2"), 0); // Texture unit 0

    LoadRoomsFromFile("rooms.txt");
   // LoadPortalsFromFile("portals.txt");
    


    
   /*for(int i=1; i<=portalCount; i++){
        portalmodels[i] = LoadDataToModel(portals[i].vertices, portals[i].normals, portals[i].texCoords, portals[i].colors, portals[i].indices, sizeof(portals[i].vertices), sizeof(portals[i].indices));
   
    }*/
    
    for(int i=1; i<=roomCount; i++){
        roommodels[i] = LoadDataToModel(rooms[i].vertices, rooms[i].normals, rooms[i].texCoords, rooms[i].colors, rooms[i].indices, sizeof(rooms[i].vertices), sizeof(rooms[i].indices));
   
    }
    addPortal(1,	// Add to this cell
		2,			// Destination cell
		vec3(30,-10, -20), // Four corners of portal. Must be a square.
		vec3(30, 10, -20),
		vec3(40,-10, -20),
		vec3(40, 10, -20),
		T(70,0,-20), // Translation part of portal transformation
		IdentityMatrix());

    addPortal(2,	// Add to this cell
		1,			// Destination cell
		vec3(10,-10, 0), // Four corners of portal. Must be a square.
		vec3(10, 10, 0),
		vec3(0,-10, 0),
		vec3(0, 10, 0),
		T(70,0,0), // Translation part of portal transformation
		IdentityMatrix());

    
    addPortal(2,	// Add to this cell
		3,			// Destination cell
		vec3(60,-10, -10), // Four corners of portal. Must be a square.
		vec3(60, 10, -10),
		vec3(50,-10, 0),
		vec3(50, 10, 0),
		T(70,0,0), // Translation part of portal transformation
		IdentityMatrix());
 
    addPortal(3,	// Add to this cell
		2,			// Destination cell
		vec3(0,-10, 0), // Four corners of portal. Must be a square.
		vec3(0, 10, 0),
		vec3(10,-10, -10),
		vec3(10, 10, -10),
		T(140,0,0), // Translation part of portal transformation
		IdentityMatrix());

    currentCell = 1;
	printError("init terrain");

}

void DrawCell(int currentCell, int fromCell, mat4 worldToView, mat4 modelToWorld, int count) // Draw cells recursively! Needed for the semitransparent portal!
{
	mat4 m,trans;
	
	if (count < 1)
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

			DrawCell(rooms[currentCell].portals[i].dest, currentCell, worldToView, Mult(modelToWorld, m), count+1);
		}
	}
		
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	printf("currentCell %d\n", currentCell);
   
	
        trans = T(rooms[currentCell].portals[0].center.x, rooms[currentCell].portals[0].center.y, rooms[currentCell].portals[0].center.z); // Move rooms side by side
        glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, trans.m);
        DrawModel(roommodels[currentCell], program, "in_Position", "in_Normal", "inTexCoord");
    
        
    
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

    glUseProgram(portalShader);
	glUniformMatrix4fv(glGetUniformLocation(portalShader, "modelviewMatrix"), 1, GL_TRUE, Mult(worldToView, modelToWorld).m);
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
	modelToWorld = IdentityMatrix();
	total = worldToView * modelToWorld;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex1);
    DrawCell(currentCell, -1, worldToView, IdentityMatrix(), 0); // Draw cells recursively! Needed for the semitransparent portal!
  
    /*for (int i = 1; i <=roomCount; i++)
    {
        trans = T((i-1)*70.0f, 0, 0); // Move rooms side by side
        glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, trans.m);
        DrawModel(roommodels[i], program, "in_Position", "in_Normal", "inTexCoord");
        
    }*/
    /*for (int i = 1; i <=portalCount; i++)
    {
        if(i != 3){
            trans = T((i-1)*70.0f, 0, 0); // Move rooms side by side
        }
        if(i == 4){
            trans = T((2)*70.0f, 0, 0);
        }
        glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, trans.m);
        DrawModel(portalmodels[i], program, "in_Position", "in_Normal", "inTexCoord");
        
    }*/
    /*glUniform1i(useTex2Loc, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, rooms[1].portals[0].trans.m);
    DrawModel(rooms[1].portals[0].model, program, "in_Position", "in_Normal", "inTexCoord");
    glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, rooms[2].portals[0].trans.m);
    DrawModel(rooms[2].portals[0].model, program, "in_Position", "in_Normal", "inTexCoord");
    glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, rooms[2].portals[1].trans.m);
    DrawModel(rooms[2].portals[1].model, program, "in_Position", "in_Normal", "inTexCoord");
	glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, rooms[3].portals[0].trans.m);
    DrawModel(rooms[3].portals[0].model, program, "in_Position", "in_Normal", "inTexCoord");*/
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
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

