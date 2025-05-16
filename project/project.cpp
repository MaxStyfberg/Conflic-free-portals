// Lab 4, terrain generation

// uses framework Cocoa
// uses framework OpenGL
#define MAIN
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "lodepng.h"
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include<math.h>
#include <time.h>
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
    vec3 positions;
    float size;
    Model* model;
    float rotationY;
    GLuint boxtex;
} Box;

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
    Box *boxes;
    int boxCount = 0;
} Room;



mat4 projectionMatrix;



// Function to update sphere position

// vertex array object
Room rooms[MAX_ROOMS];
Portal portals[MAX_ROOMS];
//int portalCount = 0;
int roomCount = 0;
int currentCell = 1;
GLuint program,portalShader,objectShader;
Model *roommodels[modelno];
Model *cube;
GLuint wall2,wall3,wall1,floor3, roof, floor1, floor2,box1,box2,box3,box4;
// Reference to shader program

vec3 cameraPos = {30.0f, 0.0f, 0.0f};  // Camera's position
vec3 cameraFront = {0.0f, 0.0f, -1.0f}; // Direction camera is facing
vec3 cameraUp = {0.0f, 1.0f, 0.0f}; // Up direction

float yaw = -90.0f; // Left/right rotation
float pitch = 0.0f; // Up/down rotation
float cameraSpeed = 1.0f;

ma_engine engine;
ma_sound backgroundSound;
ma_sound walkingSound;
double lastPlayTime = 0.0;
double soundCooldown = 0.6;
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
}
double getTimeInSeconds() {
    return glutGet(GLUT_ELAPSED_TIME) / 1000.0;
}
void initAudio() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize audio engine.\n");
    }
     if (ma_sound_init_from_file(&engine, "421701__bolkmar__atmos-horror-creepy-dark-lord-is-coming.wav", 0, NULL, NULL, &backgroundSound) != MA_SUCCESS) {
        fprintf(stderr, "Failed to load background sound.\n");
        return;
    }
    

    // Set the sound to loop automatically
    ma_sound_set_looping(&backgroundSound, MA_TRUE);
    if (ma_sound_init_from_file(&engine, "421133__giocosound__footstep_metal_2.wav", 0, NULL, NULL, &walkingSound) != MA_SUCCESS) {
        fprintf(stderr, "Failed to load walking sound.\n");
        return;
    }
}

void playCollisionSound() {
    double now = getTimeInSeconds();
    if (now - lastPlayTime >= soundCooldown) {
        ma_engine_play_sound(&engine, "793441__penguinnom__collision-sound1.wav", NULL);
        lastPlayTime = now;
    }
}
void playPortalSound() {
    double now = getTimeInSeconds();
    if (now - lastPlayTime >= soundCooldown) {
        ma_engine_play_sound(&engine, "178350__andromadax24__s_teleport_05.wav", NULL);
        lastPlayTime = now;
    }
}
void playWalkingSound() {
    if (!ma_sound_is_playing(&walkingSound)) {
        ma_sound_start(&walkingSound);
    }
}
void playBackgroundSound() {
    if (!ma_sound_is_playing(&backgroundSound)) {
        ma_sound_start(&backgroundSound);
    }
}
void shutdownAudio() {
    ma_engine_uninit(&engine);
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
bool isCollidingWithBox(Box* box, vec3 pos) {
    float halfSize = box->size;

    return (pos.x > box->positions.x - halfSize && pos.x < box->positions.x + halfSize &&
            pos.z > box->positions.z - halfSize && pos.z < box->positions.z + halfSize);
}
void moveCamera() {
    vec3 right = normalize(cross(cameraFront, cameraUp));
    float rotationSpeed = 2.0f;
    vec3 movement = {0.0f, 0.0f, 0.0f};

    // Accumulate movement input
   bool isMoving = false;

    if (glutKeyIsDown('w')) {
        movement = VectorAdd(movement, ScalarMult(cameraFront, cameraSpeed));
        isMoving = true;
     }
    if (glutKeyIsDown('s')) {
        movement = VectorSub(movement, ScalarMult(cameraFront, cameraSpeed));
        isMoving = true;
    }
    if (glutKeyIsDown('a')) {
        movement = VectorSub(movement, ScalarMult(right, cameraSpeed));
        isMoving = true;
    }
    if (glutKeyIsDown('d')) {
        movement = VectorAdd(movement, ScalarMult(right, cameraSpeed));
        isMoving = true;
    }
   

// Only play sound if a movement key was pressed
if (isMoving) {
    playWalkingSound();
}

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
        playCollisionSound();
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
        playPortalSound();
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
   for (int i = 0; i < rooms[currentCell].boxCount; i++) {
        if (isCollidingWithBox(&rooms[currentCell].boxes[i], cameraPos)) {
            cameraPos = oldPos;
            playCollisionSound();
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
    
}

void addBox(int cell, vec3 pos, int size, Model *model, float rotation, GLint boxtex )
{
   
	int boxCount = rooms[cell].boxCount;
	if (rooms[cell].boxes == NULL)
		rooms[cell].boxes = (Box *) malloc(sizeof(Box));
	else
		rooms[cell].boxes = (Box *) realloc(rooms[cell].boxes, (boxCount+1)*sizeof(Box));
    
    rooms[cell].boxes[boxCount].positions = pos;
    rooms[cell].boxes[boxCount].size = size;
    rooms[cell].boxes[boxCount].model = model;
    rooms[cell].boxes[boxCount].rotationY = rotation;
    rooms[cell].boxes[boxCount].boxtex = boxtex;
    rooms[cell].boxCount +=1;
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
    objectShader = loadShaders("object.vert", "object.frag");
	glUseProgram(program);
	printError("init shader");
	
	glUseProgram(program);
    
    cube = LoadModel("cube.obj");
	glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "tex1"), 1); // Texture unit 0
    glUniform1i(glGetUniformLocation(program, "tex2"), 2); // Texture unit 0

	glUseProgram(portalShader);
	glUniformMatrix4fv(glGetUniformLocation(portalShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(portalShader, "tex"), 0); // Texture unit 0
    glUniform1i(glGetUniformLocation(portalShader, "tex1"), 1); // Texture unit 0
    glUniform1i(glGetUniformLocation(portalShader, "tex2"), 2); // Texture unit 0

    glUseProgram(objectShader);
	glUniformMatrix4fv(glGetUniformLocation(objectShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(objectShader, "tex"), 0); // Texture unit 0

	LoadTGATextureSimple("ConcreteWallPainted-HR_64.tga", &wall2);
	LoadTGATextureSimple("ConcreteWall-Hazard-Full-01_64.tga", &wall3);
	LoadTGATextureSimple("ConcreteWallPainted-H2FB_64.tga", &wall1);
	LoadTGATextureSimple("ConcreteFloorPainted-H2FY_64.tga", &floor3);
    LoadTGATextureSimple("ConcreteFloorPainted-Cb16x16B_64.tga", &roof);
    LoadTGATextureSimple("ConcreteFloor-Hazard-Full-01_64.tga", &floor1);
    LoadTGATextureSimple("ConcreteFloorPainted-HR_64.tga", &floor2);
    LoadTGATextureSimple("Metal-Panel-003_Section-001.tga", &box1);
    LoadTGATextureSimple("Panel-001-2_Base-001.tga", &box2);
    LoadTGATextureSimple("Grid-002-9_Base-004.tga", &box3);
    LoadTGATextureSimple("Vent-003_Base-003.tga", &box4);
    
   
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
 
    addBox(1, vec3(10,-6, 0), 4, cube, 20, box1);

    addBox(1, vec3(37, -8, 15), 2, cube, 20, box2);

    addBox(1, vec3(32, -8, 15), 2, cube, 30, box2);

    addBox(1, vec3(34, -8, 10), 2, cube, 10, box2);

    addBox(1, vec3(34, -4, 14), 2, cube, 40, box3);

    addBox(1, vec3(-32, -7, -10), 3, cube, 1, box4);

    addBox(2, vec3(55,-6, -30), 4, cube, 10,box3);
     
    addBox(2, vec3(85,-6, -55), 4, cube, 10,box3);
    
    addBox(2, vec3(85, 1, -54), 3, cube, 30,box1);

    addBox(2, vec3(55, -7, -62), 3, cube, 1, box4);

    addBox(3, vec3(100,-6, 20), 4, cube, 40,box2);
    
    addBox(3, vec3(160,-6, -10), 4, cube,1,box1);
    
    addBox(3, vec3(160,-6, 0), 4, cube, 30, box1);

    addBox(3, vec3(160, 1, -5), 3, cube, 30, box2);

    addBox(3, vec3(160, -4, 20), 6, cube, 30, box2);

    addBox(3, vec3(120, -7, -22), 3, cube, 1, box4);

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
    
    glUseProgram(objectShader);
    for (int i = 0; i < rooms[currentCell].boxCount; i++) {
        glActiveTexture(GL_TEXTURE0);
        Box* b = &rooms[currentCell].boxes[i];
        glBindTexture(GL_TEXTURE_2D, b->boxtex);
        glUniform3fv(glGetUniformLocation(objectShader, "lightPos"), 1, &rooms[currentCell].lightPos.x);
        glUniform3fv(glGetUniformLocation(objectShader, "lightColour"), 1, &lightcolour.x);
        glUniform3fv(glGetUniformLocation(objectShader, "CamDir"), 1, &cameraPos.x);
        mat4 modelMatrix = T(b->positions.x, b->positions.y, b->positions.z)
                     * Ry(b->rotationY * (M_PI / 180.0f))  // Convert degrees to radians if needed
                     * S(b->size, b->size, b->size);
        glUniformMatrix4fv(glGetUniformLocation(program, "total"), 1, GL_TRUE, modelMatrix.m);
        DrawModel(b->model, objectShader, "in_Position", "in_Normal", "inTexCoord");
    }   
    
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

    glUseProgram(objectShader);

    glUniformMatrix4fv(glGetUniformLocation(portalShader, "view"), 1, 
     GL_TRUE, worldToView.m);

    glUseProgram(program);
	modelToWorld = IdentityMatrix();
	total = worldToView * modelToWorld;
    DrawCell(currentCell, -1, worldToView, IdentityMatrix(), 0); // Draw cells recursively! Needed for the semitransparent portal!
 
	printError("display 2");
	
	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
    initAudio();
    playBackgroundSound();
     atexit(shutdownAudio);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH );
	glutInitContextVersion(3, 2);
	glutInitWindowSize (600, 600);
	glutCreateWindow ("TSBK07 Lab 4");
	glutDisplayFunc(display);
	init ();
	glutRepeatingTimer(20);
	


	glutMainLoop();
    shutdownAudio();
	exit(0);
}

