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
    vec3 vertices[MAX_VERTICES];
    vec3 normals[MAX_VERTICES];
    vec2 texCoords[MAX_VERTICES];
    vec3 colors[MAX_VERTICES];
    GLuint indices[MAX_INDICES];
} Room;
typedef struct {
    vec3 vertices[MAX_VERTICES];
    vec3 normals[MAX_VERTICES];
    vec2 texCoords[MAX_VERTICES];
    vec3 colors[MAX_VERTICES];
    GLuint indices[MAX_INDICES];
} Portal;

mat4 projectionMatrix;



// Function to update sphere position

// vertex array object
vec3 vertex[4];
Room rooms[MAX_ROOMS];
Portal portals[MAX_ROOMS];
int portalCount = 0;
int roomCount = 0;
GLuint program;
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
void LoadPortalsFromFile(const char* filename)
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
}
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

void init(void)
{
	// GL inits
	glClearColor(0.4,0.6,0.9,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	printError("GL inits");
    glEnable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 5000.0);

	// Load and compile shader
	program = loadShaders("project.vert", "project.frag");
	glUseProgram(program);
	printError("init shader");
	
	glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
    
    LoadTGATextureSimple("grass.tga", &tex1);
    LoadRoomsFromFile("rooms.txt");
    LoadPortalsFromFile("portals.txt");
    glUniform1i(glGetUniformLocation(program, "tex1"), 0); // Set texture unit 0
    
    
   for(int i=1; i<=portalCount; i++){
        portalmodels[i] = LoadDataToModel(portals[i].vertices, portals[i].normals, portals[i].texCoords, portals[i].colors, portals[i].indices, sizeof(portals[i].vertices), sizeof(portals[i].indices));
   
    }
    for(int i=1; i<=roomCount; i++){
        roommodels[i] = LoadDataToModel(rooms[i].vertices, rooms[i].normals, rooms[i].texCoords, rooms[i].colors, rooms[i].indices, sizeof(rooms[i].vertices), sizeof(rooms[i].indices));
   
    }
	printError("init terrain");

}

void display(void)
{
	
   
    // clear the screen
    moveCamera();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0xFF);
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
  

  

    for (int i = 1; i <=roomCount; i++)
    {
        trans = T((i-1)*70.0f, 0, 0); // Move rooms side by side
        glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, trans.m);
        DrawModel(roommodels[i], program, "in_Position", "in_Normal", "inTexCoord");
        
    }
    for (int i = 1; i <=portalCount; i++)
    {
        if(i != 3){
            trans = T((i-1)*70.0f, 0, 0); // Move rooms side by side
        }
        if(i == 4){
            trans = T((2)*70.0f, 0, 0);
        }
        glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, trans.m);
        DrawModel(portalmodels[i], program, "in_Position", "in_Normal", "inTexCoord");
        
    }
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
