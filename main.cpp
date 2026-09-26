#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <cstring>
#include <initializer_list>

// ---------------------- STB Image (single header) ----------------------
// Download stb_image.h from https://github.com/nothings/stb/blob/master/stb_image.h
// and place it in the same folder as this .cpp file.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Some older/MinGW OpenGL headers (common on Windows + Code::Blocks) don't
// define this OpenGL 1.2 constant even though the driver supports it.
// Defining it manually here avoids a "not declared in this scope" error.
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifdef _WIN32
#include <windows.h>
#endif

// ---------------------- Mansion Interior ----------------------
bool mansionDoorOpen = false;
float mansionDoorAngle = 0.0f;

const float MANSION_HW = 9.0f;
const float MANSION_HD = 6.0f;
const float MANSION_H  = 10.0f;
const float MANSION_DOOR_W = 3.0f;
const float MANSION_DOOR_H = 4.5f;
const float MANSION_HALL_DEPTH = 4.0f;

// ---------------------- Shopping Mall System (Simple) ----------------------
#define NUM_SHOPS 2
float shopX[NUM_SHOPS]         = { 10.0f, 80.0f };
float shopZ[NUM_SHOPS]         = {  80.0f,  16.0f };
float shopRotY[NUM_SHOPS]      = { 180.0f, -90.0f };
bool  shopDoorOpen[NUM_SHOPS]  = { false, false };
float shopDoorAngle[NUM_SHOPS] = { 0.0f, 0.0f };

const float SHOP_W = 16.0f, SHOP_D = 12.0f, SHOP_H = 4.5f;
const float SHOP_DOOR_W = 2.4f, SHOP_DOOR_H = 2.8f;

// ---------------------- Fish Jump System (no struct) ----------------------
#define MAX_FISH 20

float fishX[MAX_FISH];
float fishZ[MAX_FISH];
float fishJumpT[MAX_FISH];
bool  fishJumping[MAX_FISH];
float fishWaitTimer[MAX_FISH];
float fishJumpDuration[MAX_FISH];
float fishJumpHeight[MAX_FISH];
float fishSize[MAX_FISH];
float fishRotY[MAX_FISH];

// ---------------------- Day/Night System ----------------------
bool isNight = false;
float dayNightBlend = 0.0f;
const float DAYNIGHT_SPEED = 0.25f;

// ---------------------- Boat System ----------------------
float boatX = 0.0f, boatZ = -76.0f, boatAngle = 0.0f;
bool inBoat = false;
const float boatSpeed = 9.5f;
const float boatTurnSpeed = 65.0f;
const float BOAT_MARGIN = 5.0f;

const float BOAT_LEN    = 9.0f;
const float BOAT_HALF_W = 1.5f;
const float BOAT_WALL_H = 0.85f;
const float BOAT_DECK_Y = 0.55f;

// ---------------------- river System ----------------------
const float RIVER_CENTER_Z   = -110.0f;
const float RIVER_HALF_DEPTH = 34.0f;
const float RIVER_HALF_WIDTH = 60.0f;

// ---------------------- Railway ----------------------
const float RAIL_X = 95.0f;
const float RAIL_Z_START = 110.0f;
const float RAIL_Z_END   = -165.0f;
const float RAIL_TOTAL_LEN = RAIL_Z_START - RAIL_Z_END;

const float RIVER_Z1 = RIVER_CENTER_Z - RIVER_HALF_DEPTH;
const float RIVER_Z2 = RIVER_CENTER_Z + RIVER_HALF_DEPTH;
const float BRIDGE_MARGIN = 15.0f;
const float BRIDGE_HEIGHT = 5.0f;

bool trainRunning = true;
float trainDist = 0.0f;
const float trainSpeed = 14.0f;
const int NUM_CARRIAGES = 4;
const float CARRIAGE_LEN = 6.0f;
const float CARRIAGE_GAP = 1.0f;

// ---------------------- Waterfall Particle System ----------------------
struct WFParticle { float t, speed, xJitter, size, alpha; };
#define MAX_WF_PARTICLES 120
WFParticle wfParticles[MAX_WF_PARTICLES];

struct WFMist { float x, y, z, size, alpha, drift; };
#define MAX_WF_MIST 30
WFMist wfMist[MAX_WF_MIST];

const float WF_X       = 0.0f;
const float WF_TOP_Y   = 38.0f, WF_BOTTOM_Y = 0.0f;
const float WF_TOP_Z   = -145.0f, WF_BOTTOM_Z = -100.0f;
const float WF_TOP_HALFW = 0.6f, WF_BOTTOM_HALFW = 2.8f;

void waterfallPathAt(float t, float &x, float &y, float &z, float &halfW) {
    y = WF_TOP_Y + (WF_BOTTOM_Y - WF_TOP_Y) * t;
    z = WF_TOP_Z + (WF_BOTTOM_Z - WF_TOP_Z) * t;

    float bend = sinf(t * 3.0f) * 1.4f + sinf(t * 7.3f + 1.7f) * 0.5f;
    x = WF_X + bend * (1.0f - t * 0.4f);

    halfW = WF_TOP_HALFW + (WF_BOTTOM_HALFW - WF_TOP_HALFW) * t;
}

float waterfallOffset = 0.0f;
float waveOffset = 0.0f;

const float PI = 3.14159265f;

// ============ Camera (third-person, follows player) ============
float camDist   = 8.0f;
float camHeight = 7.0f;

const float GROUND_SIZE = 170.0f;

// ============ Barricade settings ============
const float BARRIER = 22.0f;
const float BARRIER_HEIGHT = 1.3f;
const float POST_SPACING = 4.0f;
const float GATE_HALF_WIDTH = 3.0f;

// ============ Loop road (village ring road) ============
const float LOOP_R = 55.0f;
const float ROAD_WIDTH = 4.0f;
float loopCorners[4][2] = { {-LOOP_R,-LOOP_R}, {LOOP_R,-LOOP_R}, {LOOP_R,LOOP_R}, {-LOOP_R,LOOP_R} };
float segLen = 2.0f * LOOP_R;
float totalLoopLen = 4.0f * segLen;

// ============ Guards ============
struct Guard { float x, z, angle; };
Guard guards[2] = { { 3.0f, 10.0f, 180.0f }, { 0.0f, BARRIER + 4.0f, 180.0f } };
int selectedGuard = 0;
const float guardSpeed = 6.0f;
const float guardTurnSpeed = 90.0f;

// ============ Player ============
struct Player { float x, z, angle; };
Player player = { 5.0f, 15.0f, 0.0f };
const float playerSpeed = 6.0f;
const float playerTurnSpeed = 100.0f;

// ============ Houses (scattered randomly) ============
struct House {
    float x, z, rotY;
    int type;
    float hw, hd, doorW, doorH;
    bool doorOpen;
    float doorAngle;
};
const int NUM_HOUSES = 20;
House houses[NUM_HOUSES];

// ============ Fruit Trees ============
enum FruitType { MANGO=0, JACKFRUIT, JAM, LEMON, GUAVA, PAPAYA, BOROI, NUM_FRUIT_TYPES };
const int MAX_FRUITS = 6;
struct FruitTree {
    float x, z, rotY;
    float leanDeg, leanAxisX, leanAxisZ;
    int type;
    float swayPhase;
    float fruitLocalX[MAX_FRUITS], fruitLocalY[MAX_FRUITS], fruitLocalZ[MAX_FRUITS];
    bool fruitPicked[MAX_FRUITS];
    int fruitCount;
};
const int NUM_FRUIT_TREES = 55;
FruitTree fruitTrees[NUM_FRUIT_TREES];
int fruitsCollected = 0;

// ============ Pedestrians ============
struct Ped { float t, speed, laneOffset; int variant; };
const int NUM_PEDS = 24;
Ped peds[NUM_PEDS];

// ============ Vehicles ============
struct Vehicle { float t, speed, laneOffset; int variant; };
const int NUM_VEHICLES = 16;
Vehicle vehicles[NUM_VEHICLES];

// ============ Held-key state ============
bool w_down=false, a_down=false, s_down=false, d_down=false;
bool up_down=false, down_down=false, left_down=false, right_down=false;

const float TIMER_DT = 0.03f;
float flagTime = 0.0f;

// ---------------------------------------------------------------
// ================== TEXTURE / BANNER SYSTEM =====================
// ---------------------------------------------------------------
// Global texture handles. If a file fails to load, the handle stays 0
// and drawTexturedQuad() simply skips drawing (falls back to the plain
// colored board that is drawn underneath it), so the program never
// crashes even if an image is missing.
GLuint texMansionSign      = 0;
GLuint texSchoolBanner     = 0;
GLuint texUniversityBanner = 0;
GLuint texHospitalBanner   = 0;
GLuint texShopBanner       = 0;

// Actual pixel dimensions of each loaded image, so we can preserve its
// aspect ratio instead of stretching it to fit a fixed box.
int texMansionSignW = 0, texMansionSignH = 0;
int texSchoolBannerW = 0, texSchoolBannerH = 0;
int texUniversityBannerW = 0, texUniversityBannerH = 0;
int texHospitalBannerW = 0, texHospitalBannerH = 0;
int texShopBannerW = 0, texShopBannerH = 0;

GLuint loadTexture(const char* filename, int* outW = nullptr, int* outH = nullptr) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); // OpenGL expects bottom-left origin
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4); // force RGBA
    if (!data) {
        printf("[texture] WARNING: failed to load \"%s\" - a plain board will show instead.\n", filename);
        return 0;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    printf("[texture] loaded \"%s\" (%dx%d)\n", filename, width, height);
    if (outW) *outW = width;
    if (outH) *outH = height;
    return texID;
}

// Draws a flat textured rectangle of size w x h, centered at the current
// origin, facing +Z (same orientation drawText3D content used to face).
// Call this after glTranslatef-ing to the desired world position.
void drawTexturedQuad(GLuint texID, float w, float h) {
    if (texID == 0) return; // no texture loaded -> draw nothing extra

    // Draw the banner fully bright regardless of scene lighting/day-night,
    // same as the bitmap text it replaces (glutBitmapCharacter always draws
    // at full brightness too, ignoring GL_LIGHTING).
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float hw = w / 2.0f, hh = h / 2.0f;
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( hw, -hh, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( hw,  hh, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw,  hh, 0.0f);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

// Draws texID so that it fits INSIDE a maxW x maxH box while keeping the
// image's own aspect ratio (same idea as CSS "object-fit: contain") -
// this stops different-shaped banner images from looking stretched.
void drawTexturedQuadFit(GLuint texID, int texW, int texH, float maxW, float maxH) {
    if (texID == 0 || texW <= 0 || texH <= 0) return;

    float texAspect = (float)texW / (float)texH;
    float boxAspect = maxW / maxH;

    float w, h;
    if (texAspect > boxAspect) {
        // image is relatively wider than the box -> width is the limit
        w = maxW;
        h = maxW / texAspect;
    } else {
        // image is relatively taller than the box -> height is the limit
        h = maxH;
        w = maxH * texAspect;
    }
    drawTexturedQuad(texID, w, h);
}

// Code::Blocks (and many IDEs) run the exe with its "working directory"
// set to somewhere other than your project folder (often bin\Debug, but
// it can vary by project setup). Guessing at the working directory is
// fragile, so instead we find out where the .exe itself actually lives
// (which never changes) and search for "images" starting from there.
void getExeDir(char* outDir, size_t outSize) {
    outDir[0] = '\0';
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        for (int i = (int)len - 1; i >= 0; i--) {
            if (buf[i] == '\\' || buf[i] == '/') {
                buf[i + 1] = '\0';
                break;
            }
        }
        strncpy(outDir, buf, outSize - 1);
        outDir[outSize - 1] = '\0';
    }
#endif
}

GLuint loadTextureSmart(const char* relPath, int* outW = nullptr, int* outH = nullptr) {
    char fullPath[1024];
    const char* upPrefixes[] = { "", "../", "../../", "../../../" };

    // 1) Try relative to the current working directory first (covers the
    //    case where the IDE's working directory IS the project root).
    for (int i = 0; i < 4; i++) {
        snprintf(fullPath, sizeof(fullPath), "%s%s", upPrefixes[i], relPath);
        FILE* f = fopen(fullPath, "rb");
        if (f) { fclose(f); printf("[texture] found \"%s\" via cwd search\n", fullPath); return loadTexture(fullPath, outW, outH); }
    }

    // 2) Try relative to the folder the .exe actually lives in. This is
    //    the reliable path: bin\Debug\..\..\images\x.png == <project>\images\x.png
    char exeDir[512];
    getExeDir(exeDir, sizeof(exeDir));
    if (exeDir[0] != '\0') {
        for (int i = 0; i < 4; i++) {
            snprintf(fullPath, sizeof(fullPath), "%s%s%s", exeDir, upPrefixes[i], relPath);
            FILE* f = fopen(fullPath, "rb");
            if (f) { fclose(f); printf("[texture] found \"%s\" via exe-dir search\n", fullPath); return loadTexture(fullPath, outW, outH); }
        }
    }

    printf("[texture] WARNING: could not find \"%s\". exeDir=\"%s\". "
           "Double-check the \"images\" folder name/spelling and that it sits "
           "next to main.cpp (project root).\n", relPath, exeDir);
    return 0;
}

void loadAllBannerTextures() {
    texMansionSign      = loadTextureSmart("images/gram_panchayet.png", &texMansionSignW, &texMansionSignH);
    texSchoolBanner      = loadTextureSmart("images/school.png", &texSchoolBannerW, &texSchoolBannerH);
    texUniversityBanner  = loadTextureSmart("images/university.png", &texUniversityBannerW, &texUniversityBannerH);
    texHospitalBanner    = loadTextureSmart("images/hospital.png", &texHospitalBannerW, &texHospitalBannerH);
    texShopBanner        = loadTextureSmart("images/supermarket.png", &texShopBannerW, &texShopBannerH);
}

// ---------------------------------------------------------------
void drawBox(float w, float h, float d) {
    glPushMatrix();
    glScalef(w, h, d);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawRoadSegment(float x1, float z1, float x2, float z2, float width) {
    float dx = x2 - x1, dz = z2 - z1;
    float length = sqrt(dx * dx + dz * dz);
    float midx = (x1 + x2) / 2.0f, midz = (z1 + z2) / 2.0f;
    float angle = atan2(dx, dz) * 180.0f / PI;
    float y = 0.02f + width * 0.002f;

    glPushMatrix();
    glTranslatef(midx, y, midz);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    glDisable(GL_LIGHTING);
    glColor3f(0.33f, 0.33f, 0.33f);
    glBegin(GL_QUADS);
        glVertex3f(-width / 2.0f, 0.0f, -length / 2.0f);
        glVertex3f( width / 2.0f, 0.0f, -length / 2.0f);
        glVertex3f( width / 2.0f, 0.0f,  length / 2.0f);
        glVertex3f(-width / 2.0f, 0.0f,  length / 2.0f);
    glEnd();
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void drawGround() {
    glDisable(GL_LIGHTING);          // <-- যোগ করো
    glColor3f(0.25f, 0.55f, 0.2f);
    glBegin(GL_QUADS);
        glVertex3f(-GROUND_SIZE, 0.0f, -GROUND_SIZE);
        glVertex3f(-GROUND_SIZE, 0.0f,  GROUND_SIZE);
        glVertex3f( GROUND_SIZE, 0.0f,  GROUND_SIZE);
        glVertex3f( GROUND_SIZE, 0.0f, -GROUND_SIZE);
    glEnd();
    glEnable(GL_LIGHTING);           // <-- যোগ করো
}

// ---------------------- Mansion ----------------------
void drawColumn(float x, float z, float height) {
    glPushMatrix();
    glTranslatef(x, height / 2.0f, z);
    glColor3f(0.9f, 0.9f, 0.85f);
    drawBox(1.0f, height, 1.0f);
    glPopMatrix();
}

void drawSteps() {
    glColor3f(0.62f, 0.62f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.06f, 9.0f);
    drawBox(17.0f, 0.12f, 4.0f);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.48f);
    glPushMatrix();
    glTranslatef(0.0f, 0.01f, 9.0f);
    drawBox(17.3f, 0.03f, 4.3f);
    glPopMatrix();
}

void drawPediment() {
    glColor3f(0.85f, 0.8f, 0.7f);
    glBegin(GL_TRIANGLES);
        glVertex3f(-9.0f, 10.0f, 6.0f); glVertex3f(9.0f, 10.0f, 6.0f); glVertex3f(0.0f, 14.0f, 6.0f);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex3f(-9.0f, 10.0f, 2.0f); glVertex3f(9.0f, 10.0f, 2.0f); glVertex3f(0.0f, 14.0f, 2.0f);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(-9.0f, 10.0f, 6.0f); glVertex3f(0.0f, 14.0f, 6.0f); glVertex3f(0.0f, 14.0f, 2.0f); glVertex3f(-9.0f, 10.0f, 2.0f);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(9.0f, 10.0f, 6.0f); glVertex3f(0.0f, 14.0f, 6.0f); glVertex3f(0.0f, 14.0f, 2.0f); glVertex3f(9.0f, 10.0f, 2.0f);
    glEnd();
}

void drawText3D(float x, float y, float z, const char* text) {
    glRasterPos3f(x, y, z);
    for (const char* c = text; *c != '\0'; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

void drawSimpleDesk(float w, float d, float h) {
    glColor3f(0.55f, 0.38f, 0.2f);
    glPushMatrix(); glTranslatef(0.0f, h, 0.0f); drawBox(w, 0.12f, d); glPopMatrix();
    glColor3f(0.4f, 0.26f, 0.12f);
    float lx = w/2.0f - 0.1f, lz = d/2.0f - 0.1f;
    float legPos[4][2] = { {-lx,-lz},{lx,-lz},{-lx,lz},{lx,lz} };
    for (auto &lp : legPos) {
        glPushMatrix(); glTranslatef(lp[0], h/2.0f, lp[1]); drawBox(0.1f, h, 0.1f); glPopMatrix();
    }
}

void drawOfficeChair(bool hasBack) {
    glColor3f(0.15f, 0.15f, 0.18f);
    glPushMatrix(); glTranslatef(0.0f, 0.9f, 0.0f); drawBox(0.9f, 0.1f, 0.9f); glPopMatrix();
    if (hasBack) {
        glPushMatrix(); glTranslatef(0.0f, 1.45f, -0.42f); drawBox(0.85f, 1.0f, 0.1f); glPopMatrix();
    }
    glColor3f(0.2f, 0.2f, 0.22f);
    glPushMatrix(); glTranslatef(0.0f, 0.45f, 0.0f); drawBox(0.12f, 0.9f, 0.12f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.05f, 0.0f); drawBox(0.9f, 0.08f, 0.15f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.05f, 0.0f); drawBox(0.15f, 0.08f, 0.9f); glPopMatrix();
}

void drawComputer() {
    glColor3f(0.1f,0.1f,0.1f);
    glPushMatrix(); glTranslatef(0.0f, 0.08f, 0.0f); drawBox(0.3f,0.16f,0.2f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.25f, 0.0f); drawBox(0.05f,0.35f,0.05f); glPopMatrix();
    glColor3f(0.05f,0.05f,0.05f);
    glPushMatrix(); glTranslatef(0.0f, 0.55f, 0.0f); drawBox(0.7f,0.45f,0.05f); glPopMatrix();
    glColor3f(0.3f,0.6f,0.9f);
    glPushMatrix(); glTranslatef(0.0f, 0.55f, 0.03f); drawBox(0.6f,0.36f,0.02f); glPopMatrix();
    glColor3f(0.85f,0.85f,0.85f);
    glPushMatrix(); glTranslatef(0.0f, 0.02f, 0.35f); drawBox(0.5f,0.03f,0.18f); glPopMatrix();
    glColor3f(0.85f,0.85f,0.88f);
    glPushMatrix(); glTranslatef(0.5f, -0.5f, 0.05f); drawBox(0.25f,1.0f,0.45f); glPopMatrix();
}

void drawBench(float length) {
    glColor3f(0.4f, 0.28f, 0.14f);
    glPushMatrix(); glTranslatef(0.0f, 0.5f, 0.0f); drawBox(length, 0.1f, 0.45f); glPopMatrix();
    float lx = length/2.0f - 0.2f;
    float legZ[2] = {-0.18f, 0.18f};
    for (float sx : {-lx, lx})
        for (float sz : legZ) {
            glPushMatrix(); glTranslatef(sx, 0.25f, sz); drawBox(0.08f,0.5f,0.08f); glPopMatrix();
        }
}

void drawBookshelf(float w, float h) {
    glColor3f(0.42f, 0.28f, 0.14f);
    glPushMatrix(); glTranslatef(0.0f, h/2.0f, 0.0f); drawBox(w, h, 0.35f); glPopMatrix();
    float bookColors[6][3] = {{0.8f,0.2f,0.2f},{0.2f,0.5f,0.8f},{0.9f,0.75f,0.1f},{0.2f,0.6f,0.3f},{0.6f,0.3f,0.7f},{0.85f,0.5f,0.15f}};
    int shelves = 3;
    for (int s = 0; s < shelves; s++) {
        float sy = 0.35f + s * (h - 0.5f) / shelves;
        int nbooks = 10;
        for (int b = 0; b < nbooks; b++) {
            float bw = w / nbooks;
            float bx = -w/2.0f + bw*b + bw/2.0f;
            glColor3f(bookColors[b%6][0], bookColors[b%6][1], bookColors[b%6][2]);
            glPushMatrix();
            glTranslatef(bx, sy, 0.14f);
            drawBox(bw*0.85f, 0.32f, 0.22f);
            glPopMatrix();
        }
    }
}

void drawPottedPlant() {
    glColor3f(0.6f, 0.35f, 0.2f);
    glPushMatrix(); glTranslatef(0.0f, 0.25f, 0.0f);
    GLUquadric* q = gluNewQuadric();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(q, 0.35f, 0.25f, 0.5f, 10, 6);
    gluDeleteQuadric(q);
    glPopMatrix();
    glColor3f(0.12f, 0.5f, 0.15f);
    glPushMatrix(); glTranslatef(0.0f, 0.9f, 0.0f); glutSolidSphere(0.5f, 10, 8); glPopMatrix();
    glPushMatrix(); glTranslatef(0.15f, 1.25f, 0.1f); glutSolidSphere(0.32f, 8, 6); glPopMatrix();
}

void drawFilingCabinet() {
    glColor3f(0.5f, 0.52f, 0.55f);
    glPushMatrix(); glTranslatef(0.0f, 0.75f, 0.0f); drawBox(0.8f, 1.5f, 0.6f); glPopMatrix();
    glColor3f(0.3f,0.3f,0.32f);
    for (int i = 0; i < 3; i++) {
        glPushMatrix(); glTranslatef(0.0f, 0.3f + i*0.5f, 0.31f); drawBox(0.65f, 0.4f, 0.03f); glPopMatrix();
    }
}

// ---------------------- Name plate (now with optional banner image) ----------------------
void drawFixedNamePlate() {
    glPushMatrix();
    glColor3f(0.1f, 0.1f, 0.1f);
    glTranslatef(0.0f, 10.6f, 6.05f);
    drawBox(10.0f, 1.2f, 0.1f);
    glPopMatrix();

    if (texMansionSign != 0) {
        // Banner image drawn just in front of the board.
        glPushMatrix();
        glTranslatef(0.0f, 10.6f, 6.11f);
        drawTexturedQuadFit(texMansionSign, texMansionSignW, texMansionSignH, 9.6f, 1.0f);
        glPopMatrix();
    } else {
        // Fallback: original text label if no image was loaded.
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText3D(-4.6f, 10.3f, 6.2f, "GRAM PANCHAYET OFFICE");
    }
}

void drawFlag() {
    glColor3f(0.4f, 0.3f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 17.0f, 4.0f);
    drawBox(0.2f, 6.0f, 0.2f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.5f, 19.2f, 4.0f);

    glColor3f(0.0f, 0.4f, 0.15f);
    int segs = 10;
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segs; i++) {
        float u = (float)i / segs;
        float wave = sin(flagTime * 4.0f + u * 6.0f) * 0.15f * u;
        glVertex3f(u * 3.0f, 0.6f, wave);
        glVertex3f(u * 3.0f, -0.6f, wave);
    }
    glEnd();

    glPopMatrix();
    glPopMatrix();
}

void drawMansionShell();
void drawOfficeRoomFurniture();
void drawMeetingRoomFurniture();

void drawMansion() {
    drawMansionShell();
    drawOfficeRoomFurniture();
    drawMeetingRoomFurniture();

    float colXs[] = {-7.5f, -4.5f, -1.5f, 1.5f, 4.5f, 7.5f};
    for (float cx : colXs) drawColumn(cx, 6.5f, 9.0f);

    drawSteps();
    drawPediment();
    drawFixedNamePlate();
    drawFlag();
}

void drawTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.38f, 0.24f, 0.12f);
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.35f, 0.22f, 2.6f, 10, 8);
    gluDeleteQuadric(quad);
    glPopMatrix();

    glColor3f(0.13f, 0.44f, 0.14f);
    struct Blob { float dx, dy, dz, r; };
    Blob blobs[] = {
        {  0.0f, 3.2f,  0.0f, 1.5f },
        {  0.9f, 2.8f,  0.4f, 1.1f },
        { -0.9f, 2.9f, -0.3f, 1.15f },
        {  0.3f, 3.9f, -0.6f, 1.0f },
        { -0.5f, 3.7f,  0.7f, 1.05f },
        {  0.6f, 2.4f, -0.9f, 0.9f },
        { -0.7f, 2.5f,  0.8f, 0.9f },
    };
    for (auto &b : blobs) {
        float shade = 0.85f + 0.15f * sinf(b.dx * 3.1f + b.dz * 2.7f);
        glColor3f(0.10f * shade, 0.42f * shade, 0.13f * shade);
        glPushMatrix();
        glTranslatef(b.dx, b.dy, b.dz);
        glutSolidSphere(b.r, 10, 8);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawTreesAroundMansion() {
    float positions[][2] = { {-14.0f,10.0f},{14.0f,10.0f},{-14.0f,-8.0f},{14.0f,-8.0f},{-10.0f,-12.0f},{10.0f,-12.0f} };
    for (auto& p : positions) drawTree(p[0], p[1]);
}

// ---------------------- Barricade ----------------------
void drawPost(float x, float z) {
    glColor3f(0.9f, 0.35f, 0.0f);
    glPushMatrix();
    glTranslatef(x, BARRIER_HEIGHT / 2.0f, z);
    drawBox(0.25f, BARRIER_HEIGHT, 0.25f);
    glPopMatrix();
}

void drawBarricadeLine(float x1, float z1, float x2, float z2, bool hasGate) {
    float dx = x2 - x1, dz = z2 - z1;
    float length = sqrt(dx * dx + dz * dz);
    int steps = (int)(length / POST_SPACING);
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        float px = x1 + dx * t, pz = z1 + dz * t;
        if (hasGate && fabs(px) < GATE_HALF_WIDTH && fabs(x1 - x2) > fabs(z1 - z2)) continue;
        if (hasGate && fabs(pz) < GATE_HALF_WIDTH && fabs(z1 - z2) > fabs(x1 - x2)) continue;
        drawPost(px, pz);
    }
    glColor3f(1.0f, 0.8f, 0.0f);
    float railY = BARRIER_HEIGHT * 0.75f;
    if (!hasGate) {
        glPushMatrix();
        glTranslatef((x1 + x2) / 2.0f, railY, (z1 + z2) / 2.0f);
        if (fabs(dx) > fabs(dz)) drawBox(length, 0.15f, 0.15f); else drawBox(0.15f, 0.15f, length);
        glPopMatrix();
    } else {
        float half = length / 2.0f - GATE_HALF_WIDTH;
        if (fabs(dx) > fabs(dz)) {
            float dir = (dx > 0) ? 1.0f : -1.0f;
            glPushMatrix(); glTranslatef(x1 + dir * half / 2.0f, railY, z1); drawBox(half, 0.15f, 0.15f); glPopMatrix();
            glPushMatrix(); glTranslatef(x2 - dir * half / 2.0f, railY, z2); drawBox(half, 0.15f, 0.15f); glPopMatrix();
        } else {
            float dir = (dz > 0) ? 1.0f : -1.0f;
            glPushMatrix(); glTranslatef(x1, railY, z1 + dir * half / 2.0f); drawBox(0.15f, 0.15f, half); glPopMatrix();
            glPushMatrix(); glTranslatef(x2, railY, z2 - dir * half / 2.0f); drawBox(0.15f, 0.15f, half); glPopMatrix();
        }
    }
}

void drawBarricade() {
    drawBarricadeLine(-BARRIER, BARRIER, BARRIER, BARRIER, true);
    drawBarricadeLine(-BARRIER, -BARRIER, BARRIER, -BARRIER, false);
    drawBarricadeLine(-BARRIER, -BARRIER, -BARRIER, BARRIER, false);
    drawBarricadeLine(BARRIER, -BARRIER, BARRIER, BARRIER, false);
}

// ---------------------- Humanoid figure ----------------------
void drawHumanoid(float x, float z, float angle, float bodyR, float bodyG, float bodyB, bool marker) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix(); glTranslatef(-0.25f, 0.75f, 0.0f); drawBox(0.35f, 1.5f, 0.35f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.25f, 0.75f, 0.0f); drawBox(0.35f, 1.5f, 0.35f); glPopMatrix();

    glColor3f(bodyR, bodyG, bodyB);
    glPushMatrix(); glTranslatef(0.0f, 1.9f, 0.0f); drawBox(1.0f, 1.2f, 0.6f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.65f, 1.9f, 0.0f); drawBox(0.3f, 1.1f, 0.3f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.65f, 1.9f, 0.0f); drawBox(0.3f, 1.1f, 0.3f); glPopMatrix();

    glColor3f(0.85f, 0.65f, 0.5f);
    glPushMatrix(); glTranslatef(0.0f, 2.75f, 0.0f); glutSolidSphere(0.35f, 12, 12); glPopMatrix();

    if (marker) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glPushMatrix(); glTranslatef(0.0f, 3.6f, 0.0f); glutSolidCone(0.25f, 0.5f, 8, 8); glPopMatrix();
    }
    glPopMatrix();
}

void drawSecurityGuard(float x, float z, float angle, bool highlight) {
    drawHumanoid(x, z, angle, highlight ? 0.4f : 0.05f, 0.1f, highlight ? 0.4f : 0.35f, highlight);
}

void drawAllGuards() {
    for (int i = 0; i < 2; i++) drawSecurityGuard(guards[i].x, guards[i].z, guards[i].angle, i == selectedGuard);
}

void drawPlayer() {
    drawHumanoid(player.x, player.z, player.angle, 0.7f, 0.05f, 0.05f, false);
}

// ---------------------- Houses ----------------------
void drawGableRoof(float w, float d, float h) {
    float hw = w / 2.0f, hd = d / 2.0f;
    glBegin(GL_TRIANGLES); glVertex3f(-hw,0,-hd); glVertex3f(hw,0,-hd); glVertex3f(0,h,-hd); glEnd();
    glBegin(GL_TRIANGLES); glVertex3f(-hw,0,hd); glVertex3f(hw,0,hd); glVertex3f(0,h,hd); glEnd();
    glBegin(GL_QUADS); glVertex3f(-hw,0,-hd); glVertex3f(0,h,-hd); glVertex3f(0,h,hd); glVertex3f(-hw,0,hd); glEnd();
    glBegin(GL_QUADS); glVertex3f(hw,0,-hd); glVertex3f(0,h,-hd); glVertex3f(0,h,hd); glVertex3f(hw,0,hd); glEnd();
}

void drawWindows(float w, float wallH, float halfD, float winSize, bool twoFloors) {
    glColor3f(0.6f, 0.85f, 0.9f);
    float winY = wallH * 0.4f;
    glPushMatrix(); glTranslatef(-(w/2.0f-0.9f), winY, halfD+0.03f); drawBox(winSize,winSize,0.05f); glPopMatrix();
    glPushMatrix(); glTranslatef((w/2.0f-0.9f), winY, halfD+0.03f); drawBox(winSize,winSize,0.05f); glPopMatrix();
    if (twoFloors) {
        float upperY = wallH - 1.3f;
        glPushMatrix(); glTranslatef(-(w/2.0f-0.9f), upperY, halfD+0.03f); drawBox(winSize,winSize,0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f, upperY, halfD+0.03f); drawBox(winSize,winSize,0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef((w/2.0f-0.9f), upperY, halfD+0.03f); drawBox(winSize,winSize,0.05f); glPopMatrix();
    }
}

void drawDoor(float doorW, float doorH, float halfD, float doorAngle) {
    glColor3f(0.05f, 0.05f, 0.05f);
    glPushMatrix();
    glTranslatef(0.0f, doorH / 2.0f, halfD - 0.05f);
    drawBox(doorW - 0.1f, doorH - 0.1f, 0.05f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-doorW / 2.0f, 0.0f, halfD);
    glRotatef(doorAngle, 0.0f, 1.0f, 0.0f);
    glColor3f(0.3f, 0.18f, 0.05f);
    glPushMatrix();
    glTranslatef(doorW / 2.0f, doorH / 2.0f, 0.0f);
    drawBox(doorW, doorH, 0.08f);
    glPopMatrix();
    glPopMatrix();
}

void drawHouse(int type, float doorAngle, bool interior) {
    switch (type) {
        case 0: { float w=6.0f,d=5.0f,h=3.2f,roofH=1.8f,doorW=1.0f,doorH=1.9f;
            glColor3f(0.72f,0.28f,0.18f);
            glPushMatrix(); glTranslatef(0,h/2,0); drawBox(w,h,d); glPopMatrix();
            if (!interior) {
                glColor3f(0.5f,0.15f,0.08f);
                glPushMatrix(); glTranslatef(0,h,0); drawGableRoof(w+0.4f,d+0.4f,roofH); glPopMatrix();
            }
            drawWindows(w,h,d/2.0f,0.8f,false);
            drawDoor(doorW,doorH,d/2.0f,doorAngle);
            break; }
        case 1: { float w=6.5f,d=5.5f,h=6.5f,doorW=1.1f,doorH=2.0f;
            glColor3f(0.85f,0.83f,0.78f);
            glPushMatrix(); glTranslatef(0,h/2,0); drawBox(w,h,d); glPopMatrix();
            if (!interior) {
                glColor3f(0.55f,0.55f,0.55f);
                glPushMatrix(); glTranslatef(0,h+0.15f,0); drawBox(w+0.4f,0.3f,d+0.4f); glPopMatrix();
            }
            drawWindows(w,h,d/2.0f,0.85f,true);
            drawDoor(doorW,doorH,d/2.0f,doorAngle);
            break; }
        case 2: { float w=5.0f,d=4.5f,h=2.6f,roofH=1.6f,doorW=0.9f,doorH=1.7f;
            glColor3f(0.6f,0.45f,0.3f);
            glPushMatrix(); glTranslatef(0,h/2,0); drawBox(w,h,d); glPopMatrix();
            if (!interior) {
                glColor3f(0.75f,0.65f,0.25f);
                glPushMatrix(); glTranslatef(0,h,0); drawGableRoof(w+0.5f,d+0.5f,roofH); glPopMatrix();
            }
            drawWindows(w,h,d/2.0f,0.6f,false);
            drawDoor(doorW,doorH,d/2.0f,doorAngle);
            break; }
        case 3: { float w=5.5f,d=4.5f,h=2.8f,roofH=1.5f,doorW=0.9f,doorH=1.8f;
            glColor3f(0.45f,0.3f,0.15f);
            glPushMatrix(); glTranslatef(0,h/2,0); drawBox(w,h,d); glPopMatrix();
            if (!interior) {
                glColor3f(0.75f,0.78f,0.8f);
                glPushMatrix(); glTranslatef(0,h,0); drawGableRoof(w+0.4f,d+0.4f,roofH); glPopMatrix();
            }
            drawWindows(w,h,d/2.0f,0.65f,false);
            drawDoor(doorW,doorH,d/2.0f,doorAngle);
            break; }
    }
}

bool isPlayerInsideHouse(int i) {
    House &h = houses[i];
    float dx = player.x - h.x, dz = player.z - h.z;
    float rad = -h.rotY * PI / 180.0f;
    float lx = dx * cos(rad) - dz * sin(rad);
    float lz = dx * sin(rad) + dz * cos(rad);
    return fabs(lx) < h.hw && fabs(lz) < h.hd;
}
bool isPlayerInsideMansion() {
    return fabs(player.x) < MANSION_HW && fabs(player.z) < MANSION_HD;
}

void drawMansionShell() {
    float hw = MANSION_HW, hd = MANSION_HD, h = MANSION_H;

    glColor3f(0.82f, 0.78f, 0.68f);
    glBegin(GL_QUADS);
        glVertex3f(-hw, 0.02f, -hd); glVertex3f(hw, 0.02f, -hd);
        glVertex3f(hw, 0.02f, hd);   glVertex3f(-hw, 0.02f, hd);
    glEnd();

    glColor3f(0.95f, 0.93f, 0.85f);
    glPushMatrix(); glTranslatef(0.0f, h/2.0f, -hd); drawBox(2*hw, h, 0.3f); glPopMatrix();
    glPushMatrix(); glTranslatef(-hw, h/2.0f, 0.0f); drawBox(0.3f, h, 2*hd); glPopMatrix();
    glPushMatrix(); glTranslatef(hw, h/2.0f, 0.0f); drawBox(0.3f, h, 2*hd); glPopMatrix();

    float sideW = (2*hw - MANSION_DOOR_W) / 2.0f;
    glPushMatrix(); glTranslatef(-(MANSION_DOOR_W/2.0f + sideW/2.0f), h/2.0f, hd); drawBox(sideW, h, 0.3f); glPopMatrix();
    glPushMatrix(); glTranslatef((MANSION_DOOR_W/2.0f + sideW/2.0f), h/2.0f, hd); drawBox(sideW, h, 0.3f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, h - (h-MANSION_DOOR_H)/2.0f, hd); drawBox(MANSION_DOOR_W, h - MANSION_DOOR_H, 0.3f); glPopMatrix();

    if (!isPlayerInsideMansion()) {
        glColor3f(0.85f, 0.83f, 0.75f);
        glPushMatrix(); glTranslatef(0.0f, h, 0.0f); drawBox(2*hw, 0.3f, 2*hd); glPopMatrix();
    }

    float dividerZEnd = hd - MANSION_HALL_DEPTH;
    float dividerLen = dividerZEnd - (-hd);
    glColor3f(0.9f, 0.88f, 0.82f);
    glPushMatrix();
    glTranslatef(0.0f, h/2.0f, -hd + dividerLen/2.0f);
    drawBox(0.2f, h, dividerLen);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-MANSION_DOOR_W/2.0f, 0.0f, hd);
    glRotatef(mansionDoorAngle, 0.0f, 1.0f, 0.0f);
    glColor3f(0.35f, 0.2f, 0.08f);
    glPushMatrix(); glTranslatef(MANSION_DOOR_W/2.0f, MANSION_DOOR_H/2.0f, 0.0f); drawBox(MANSION_DOOR_W - 0.1f, MANSION_DOOR_H - 0.1f, 0.08f); glPopMatrix();
    glPopMatrix();
}

void drawOfficeRoomFurniture() {
    glPushMatrix(); glTranslatef(-5.0f, 0.0f, -3.6f); drawSimpleDesk(2.2f, 1.0f, 0.75f);
        glPushMatrix(); glTranslatef(0.0f, 0.75f, -0.15f); drawComputer(); glPopMatrix();
    glPopMatrix();

    glPushMatrix(); glTranslatef(-5.0f, 0.0f, -2.2f); glRotatef(180.0f,0,1,0); drawOfficeChair(true); glPopMatrix();

    glPushMatrix(); glTranslatef(-5.0f, 0.0f, -4.6f); glScalef(0.9f,0.9f,0.9f);
        drawHumanoid(0.0f, 0.0f, 0.0f, 0.15f, 0.3f, 0.55f, false);
    glPopMatrix();

    glPushMatrix(); glTranslatef(-8.3f, 0.0f, -1.0f); glRotatef(90.0f,0,1,0); drawBench(3.0f); glPopMatrix();

    glPushMatrix(); glTranslatef(-6.5f, 0.0f, -5.6f); drawBookshelf(3.0f, 2.4f); glPopMatrix();

    glPushMatrix(); glTranslatef(-1.0f, 0.0f, -5.5f); drawPottedPlant(); glPopMatrix();

    glPushMatrix(); glTranslatef(-0.9f, 0.0f, -2.0f); glRotatef(-90.0f,0,1,0); drawFilingCabinet(); glPopMatrix();
}

void drawMeetingRoomFurniture() {
    int rows = 2, cols = 3;
    float startX = 2.0f, startZ = -4.5f;
    float spacingX = 2.3f, spacingZ = 2.5f;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            float x = startX + c * spacingX;
            float z = startZ + r * spacingZ;
            glPushMatrix(); glTranslatef(x, 0.0f, z); drawSimpleDesk(1.3f, 0.7f, 0.72f); glPopMatrix();
            glPushMatrix(); glTranslatef(x, 0.0f, z + 0.9f); drawOfficeChair(true); glPopMatrix();
        }
    }

    glPushMatrix(); glTranslatef(8.0f, 0.0f, -5.5f); drawPottedPlant(); glPopMatrix();
}

void drawVillageHouses() {
    for (int i = 0; i < NUM_HOUSES; i++) {
        glPushMatrix();
        glTranslatef(houses[i].x, 0.0f, houses[i].z);
        glRotatef(houses[i].rotY, 0.0f, 1.0f, 0.0f);
        drawHouse(houses[i].type, houses[i].doorAngle, isPlayerInsideHouse(i));
        glPopMatrix();
    }
}

bool isPlayerInsideShop(int i) {
    float dx = player.x - shopX[i], dz = player.z - shopZ[i];
    float rad = -shopRotY[i] * PI / 180.0f;
    float lx = dx * cos(rad) - dz * sin(rad);
    float lz = dx * sin(rad) + dz * cos(rad);
    return fabs(lx) < SHOP_W/2.0f && fabs(lz) < SHOP_D/2.0f;
}

int findNearestShop(float px, float pz) {
    int best = -1; float bestDist = 1e9f;
    for (int i = 0; i < NUM_SHOPS; i++) {
        float dx = px - shopX[i], dz = pz - shopZ[i];
        float dist = sqrt(dx*dx+dz*dz);
        if (dist < bestDist) { bestDist = dist; best = i; }
    }
    if (bestDist < 12.0f) return best;
    return -1;
}

bool checkMansionCollision(float px, float pz) {
    const float margin = 0.5f;
    if (fabs(px) < MANSION_HW && fabs(pz) < MANSION_HD) {
        bool fullyInside = (fabs(px) < MANSION_HW - margin) && (fabs(pz) < MANSION_HD - margin);
        bool inFrontDoorGap = (pz > MANSION_HD - margin) && (fabs(px) < MANSION_DOOR_W/2.0f + 0.3f) && (mansionDoorAngle < -40.0f);
        if (!fullyInside && !inFrontDoorGap) return true;

        float dividerZEnd = MANSION_HD - MANSION_HALL_DEPTH;
        if (fabs(px) < 0.3f && pz < dividerZEnd) return true;
    }
    return false;
}

bool checkShopCollision(float px, float pz) {
    const float margin = 0.6f;
    for (int i = 0; i < NUM_SHOPS; i++) {
        float dx = px - shopX[i], dz = pz - shopZ[i];
        float rad = -shopRotY[i] * PI / 180.0f;
        float lx = dx * cos(rad) - dz * sin(rad);
        float lz = dx * sin(rad) + dz * cos(rad);
        float hw = SHOP_W/2.0f, hd = SHOP_D/2.0f;
        if (fabs(lx) < hw && fabs(lz) < hd) {
            bool fullyInside = (fabs(lx) < hw - margin) && (fabs(lz) < hd - margin);
            bool inDoorGap = (lz > hd - margin) && (fabs(lx) < SHOP_DOOR_W/2.0f + 0.3f) && (shopDoorAngle[i] < -40.0f);
            if (!fullyInside && !inDoorGap) return true;
        }
    }
    return false;
}

void drawSimpleShelf(float cx, float cz, float rotY, float w, float h, int colorIdx) {
    float colors[5][3] = { {0.85f,0.2f,0.2f},{0.2f,0.55f,0.85f},{0.95f,0.75f,0.15f},{0.25f,0.7f,0.3f},{0.8f,0.4f,0.7f} };
    glPushMatrix();
    glTranslatef(cx, 0.0f, cz);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    glColor3f(0.6f, 0.6f, 0.62f);
    glPushMatrix(); glTranslatef(0.0f, h/2.0f, 0.0f); drawBox(w, h, 0.5f); glPopMatrix();

    glColor3f(colors[colorIdx%5][0], colors[colorIdx%5][1], colors[colorIdx%5][2]);
    for (int s = 0; s < 3; s++) {
        glPushMatrix();
        glTranslatef(0.0f, 0.5f + s * (h-0.5f)/3.0f, 0.28f);
        drawBox(w * 0.9f, 0.35f, 0.15f);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawSimpleCounter(float cx, float cz, float rotY) {
    glPushMatrix();
    glTranslatef(cx, 0.0f, cz);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);
    glColor3f(0.85f, 0.85f, 0.88f);
    glPushMatrix(); glTranslatef(0.0f, 0.5f, 0.0f); drawBox(2.5f, 1.0f, 0.8f); glPopMatrix();
    glColor3f(0.15f, 0.15f, 0.15f);
    glPushMatrix(); glTranslatef(0.0f, 1.15f, 0.0f); drawBox(0.5f, 0.3f, 0.4f); glPopMatrix();
    glPopMatrix();
}

void drawShopShell(float w, float d, float h, float doorAngle) {
    float hw = w/2.0f, hd = d/2.0f;

    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        glVertex3f(-hw,0.02f,-hd); glVertex3f(hw,0.02f,-hd);
        glVertex3f(hw,0.02f,hd); glVertex3f(-hw,0.02f,hd);
    glEnd();

    glColor3f(0.92f, 0.92f, 0.9f);
    glPushMatrix(); glTranslatef(0.0f, h/2.0f, -hd); drawBox(w, h, 0.2f); glPopMatrix();
    glPushMatrix(); glTranslatef(-hw, h/2.0f, 0.0f); drawBox(0.2f, h, d); glPopMatrix();
    glPushMatrix(); glTranslatef(hw, h/2.0f, 0.0f); drawBox(0.2f, h, d); glPopMatrix();

    float sideW = (w - SHOP_DOOR_W) / 2.0f;
    glPushMatrix(); glTranslatef(-(SHOP_DOOR_W/2.0f + sideW/2.0f), h/2.0f, hd); drawBox(sideW, h, 0.2f); glPopMatrix();
    glPushMatrix(); glTranslatef((SHOP_DOOR_W/2.0f + sideW/2.0f), h/2.0f, hd); drawBox(sideW, h, 0.2f); glPopMatrix();

    glColor3f(0.5f, 0.15f, 0.15f);
    glPushMatrix(); glTranslatef(0.0f, h + 0.15f, 0.0f); drawBox(w + 0.6f, 0.3f, d + 0.6f); glPopMatrix();

    drawDoor(SHOP_DOOR_W, SHOP_DOOR_H, hd, doorAngle);
}

// ---------------------- Shop banner (now with optional banner image) ----------------------
void drawShopBanner(float w, float h, const char* text) {
    glPushMatrix();
    glColor3f(0.1f, 0.35f, 0.15f);
    glTranslatef(0.0f, h + 0.55f, SHOP_D/2.0f + 0.05f);
    drawBox(w * 0.75f, 0.9f, 0.1f);
    glPopMatrix();

    if (texShopBanner != 0) {
        glPushMatrix();
        glTranslatef(0.0f, h + 0.55f, SHOP_D/2.0f + 0.15f);
        drawTexturedQuadFit(texShopBanner, texShopBannerW, texShopBannerH, w * 0.7f, 0.8f);
        glPopMatrix();
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
        float textWidth = strlen(text) * 0.55f;
        drawText3D(-textWidth/2.0f, h + 0.35f, SHOP_D/2.0f + 0.15f, text);
    }
}

void drawShop(int i) {
    glPushMatrix();
    glTranslatef(shopX[i], 0.0f, shopZ[i]);
    glRotatef(shopRotY[i], 0.0f, 1.0f, 0.0f);

    drawShopShell(SHOP_W, SHOP_D, SHOP_H, shopDoorAngle[i]);
    drawShopBanner(SHOP_W, SHOP_H, "SUPERMARKET");
    drawSimpleShelf(0.0f, -SHOP_D/2.0f + 0.6f, 0.0f, SHOP_W - 4.0f, SHOP_H - 1.5f, 0);
    drawSimpleShelf(-SHOP_W/2.0f + 0.6f, 0.0f, 90.0f, SHOP_D - 4.0f, SHOP_H - 1.5f, 1);
    drawSimpleShelf(SHOP_W/2.0f - 0.6f, 0.0f, 90.0f, SHOP_D - 4.0f, SHOP_H - 1.5f, 2);
    drawSimpleCounter(0.0f, SHOP_D/2.0f - 2.0f, 0.0f);

    glPopMatrix();
}

void drawAllShops() {
    for (int i = 0; i < NUM_SHOPS; i++) drawShop(i);
}

// ---------------------- Roads ----------------------
void nearestPointOnRect(float x, float z, float R, float &ox, float &oz) {
    float dRight = R - x, dLeft = x + R, dTop = R - z, dBottom = z + R;
    float minD = dRight;
    ox = R; oz = z;
    if (dLeft < minD) { minD = dLeft; ox = -R; oz = z; }
    if (dTop < minD)  { minD = dTop;  ox = x; oz = R; }
    if (dBottom < minD){ minD = dBottom; ox = x; oz = -R; }
}

float distPointToSegment(float px, float pz, float x1, float z1, float x2, float z2) {
    float dx = x2 - x1, dz = z2 - z1;
    float len2 = dx*dx + dz*dz;
    float t = 0.0f;
    if (len2 > 0.0001f) t = ((px-x1)*dx + (pz-z1)*dz) / len2;
    if (t < 0) t = 0; if (t > 1) t = 1;
    float cx = x1 + t*dx, cz = z1 + t*dz;
    float ddx = px-cx, ddz = pz-cz;
    return sqrt(ddx*ddx + ddz*ddz);
}

float distToLoopRoad(float x, float z) {
    float minD = 1e9f;
    for (int i = 0; i < 4; i++) {
        int j = (i+1) % 4;
        float d = distPointToSegment(x, z, loopCorners[i][0], loopCorners[i][1], loopCorners[j][0], loopCorners[j][1]);
        if (d < minD) minD = d;
    }
    return minD;
}

// z অনুযায়ী রেললাইনের উচ্চতা: সাধারণ জমিতে নিচু, নদীর উপর ব্রিজ-উচ্চতায়
float railHeightAt(float z) {
    if (z <= RIVER_Z2 + BRIDGE_MARGIN && z > RIVER_Z2) {
        float t = (RIVER_Z2 + BRIDGE_MARGIN - z) / BRIDGE_MARGIN;
        return 0.15f + t * (BRIDGE_HEIGHT - 0.15f);
    }
    if (z <= RIVER_Z2 && z >= RIVER_Z1) {
        return BRIDGE_HEIGHT;
    }
    if (z < RIVER_Z1 && z >= RIVER_Z1 - BRIDGE_MARGIN) {
        float t = (z - (RIVER_Z1 - BRIDGE_MARGIN)) / BRIDGE_MARGIN;
        return 0.15f + t * (BRIDGE_HEIGHT - 0.15f);
    }
    return 0.15f;
}

void drawVillageRoads() {
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) % 4;
        drawRoadSegment(loopCorners[i][0], loopCorners[i][1], loopCorners[j][0], loopCorners[j][1], ROAD_WIDTH);
    }
    drawRoadSegment(0.0f, BARRIER, 0.0f, LOOP_R, ROAD_WIDTH);
    for (int i = 0; i < NUM_HOUSES; i++) {
        float ox, oz;
        nearestPointOnRect(houses[i].x, houses[i].z, LOOP_R, ox, oz);
        drawRoadSegment(houses[i].x, houses[i].z, ox, oz, 2.0f);
    }
}

// ---------------------- Loop-path helper ----------------------
void getLoopPoint(float t, float &x, float &z, float &heading) {
    while (t < 0) t += totalLoopLen;
    t = fmod(t, totalLoopLen);
    int seg = (int)(t / segLen);
    float frac = (t - seg * segLen) / segLen;
    int i0 = seg, i1 = (seg + 1) % 4;
    float dx = loopCorners[i1][0] - loopCorners[i0][0];
    float dz = loopCorners[i1][1] - loopCorners[i0][1];
    x = loopCorners[i0][0] + dx * frac;
    z = loopCorners[i0][1] + dz * frac;
    heading = atan2(dx, dz) * 180.0f / PI;
}

void drawPedestrian(const Ped &p) {
    float x, z, heading;
    getLoopPoint(p.t, x, z, heading);
    float headingRad = heading * PI / 180.0f;
    float dirX = sin(headingRad), dirZ = cos(headingRad);
    float perpX = dirZ, perpZ = -dirX;
    x += perpX * p.laneOffset;
    z += perpZ * p.laneOffset;

    float colors[3][3] = { {0.8f,0.2f,0.2f}, {0.2f,0.4f,0.8f}, {0.9f,0.7f,0.1f} };
    int v = p.variant % 3;
    drawHumanoid(x, z, heading, colors[v][0], colors[v][1], colors[v][2], false);
}

void drawAllPedestrians() {
    for (int i = 0; i < NUM_PEDS; i++) drawPedestrian(peds[i]);
}

void drawVehicle(const Vehicle &v) {
    float x, z, heading;
    getLoopPoint(v.t, x, z, heading);
    float headingRad = heading * PI / 180.0f;
    float dirX = sin(headingRad), dirZ = cos(headingRad);
    float perpX = dirZ, perpZ = -dirX;
    x += perpX * v.laneOffset;
    z += perpZ * v.laneOffset;

    float bodyColors[2][3] = { {0.8f,0.1f,0.1f}, {0.1f,0.3f,0.7f} };
    int variant = v.variant % 2;
    float len = (variant == 0) ? 2.4f : 3.2f;
    float wid = (variant == 0) ? 1.2f : 1.5f;
    float ht  = (variant == 0) ? 0.8f : 1.1f;

    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(heading, 0.0f, 1.0f, 0.0f);

    glColor3f(bodyColors[variant][0], bodyColors[variant][1], bodyColors[variant][2]);
    glPushMatrix(); glTranslatef(0.0f, ht/2.0f + 0.3f, 0.0f); drawBox(wid, ht, len); glPopMatrix();

    glColor3f(0.7f, 0.85f, 0.9f);
    glPushMatrix(); glTranslatef(0.0f, ht + 0.55f, -0.2f); drawBox(wid*0.8f, 0.5f, len*0.45f); glPopMatrix();

    glColor3f(0.05f, 0.05f, 0.05f);
    float wheelX = wid/2.0f - 0.05f, wheelZ = len/2.0f - 0.4f;
    float wPos[4][2] = { {-wheelX,-wheelZ}, {wheelX,-wheelZ}, {-wheelX,wheelZ}, {wheelX,wheelZ} };
    for (auto &wp : wPos) {
        glPushMatrix();
        glTranslatef(wp[0], 0.3f, wp[1]);
        drawBox(0.25f, 0.5f, 0.5f);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawAllVehicles() {
    for (int i = 0; i < NUM_VEHICLES; i++) drawVehicle(vehicles[i]);
}

// ---------------------- Fruit Trees ----------------------
void drawFruitTree(FruitTree &t) {
    glPushMatrix();
    glTranslatef(t.x, 0.0f, t.z);
    glRotatef(t.rotY, 0.0f, 1.0f, 0.0f);
    glRotatef(t.leanDeg, t.leanAxisX, 0.0f, t.leanAxisZ);

    float trunkH, trunkR, canopyR, canopyH;
    float trunkColor[3], canopyColor[3];

    switch (t.type) {
        case MANGO:     trunkH=3.5f; trunkR=0.28f; canopyR=1.8f; canopyH=3.0f; trunkColor[0]=0.35f;trunkColor[1]=0.22f;trunkColor[2]=0.1f; canopyColor[0]=0.08f;canopyColor[1]=0.45f;canopyColor[2]=0.12f; break;
        case JACKFRUIT: trunkH=3.0f; trunkR=0.38f; canopyR=1.9f; canopyH=2.6f; trunkColor[0]=0.3f;trunkColor[1]=0.2f;trunkColor[2]=0.1f;  canopyColor[0]=0.05f;canopyColor[1]=0.4f;canopyColor[2]=0.1f; break;
        case JAM:       trunkH=3.2f; trunkR=0.22f; canopyR=1.6f; canopyH=2.4f; trunkColor[0]=0.32f;trunkColor[1]=0.2f;trunkColor[2]=0.12f; canopyColor[0]=0.1f;canopyColor[1]=0.35f;canopyColor[2]=0.15f; break;
        case LEMON:     trunkH=1.6f; trunkR=0.14f; canopyR=1.1f; canopyH=1.6f; trunkColor[0]=0.4f;trunkColor[1]=0.28f;trunkColor[2]=0.14f; canopyColor[0]=0.12f;canopyColor[1]=0.5f;canopyColor[2]=0.15f; break;
        case GUAVA:     trunkH=2.2f; trunkR=0.18f; canopyR=1.4f; canopyH=2.0f; trunkColor[0]=0.5f;trunkColor[1]=0.4f;trunkColor[2]=0.25f; canopyColor[0]=0.15f;canopyColor[1]=0.5f;canopyColor[2]=0.18f; break;
        case PAPAYA:    trunkH=3.8f; trunkR=0.16f; canopyR=1.3f; canopyH=1.4f; trunkColor[0]=0.45f;trunkColor[1]=0.42f;trunkColor[2]=0.25f; canopyColor[0]=0.1f;canopyColor[1]=0.45f;canopyColor[2]=0.15f; break;
        default:        trunkH=1.4f; trunkR=0.13f; canopyR=1.0f; canopyH=1.3f; trunkColor[0]=0.38f;trunkColor[1]=0.25f;trunkColor[2]=0.12f; canopyColor[0]=0.15f;canopyColor[1]=0.42f;canopyColor[2]=0.12f; break;
    }

    glColor3f(trunkColor[0], trunkColor[1], trunkColor[2]);
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, trunkR, trunkR * 0.7f, trunkH, 8, 6);
    gluDeleteQuadric(q);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, trunkH, 0.0f);
    float sway = sin(flagTime * 1.5f + t.swayPhase) * 4.0f;
    glRotatef(sway, 0.0f, 0.0f, 1.0f);
    glRotatef(sway * 0.6f, 1.0f, 0.0f, 0.0f);

    glColor3f(canopyColor[0], canopyColor[1], canopyColor[2]);
    if (t.type == PAPAYA) {
        for (int i = 0; i < 6; i++) {
            glPushMatrix();
            glRotatef(i * 60.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.7f, 0.0f, 0.0f);
            glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
            drawBox(1.1f, 0.08f, 0.35f);
            glPopMatrix();
        }
    } else {
        glutSolidSphere(canopyR, 10, 8);
        glPushMatrix();
        glTranslatef(0.0f, canopyH * 0.5f, 0.0f);
        glutSolidSphere(canopyR * 0.7f, 8, 6);
        glPopMatrix();
    }

    for (int k = 0; k < t.fruitCount; k++) {
        if (t.fruitPicked[k]) continue;
        glPushMatrix();
        glTranslatef(t.fruitLocalX[k], t.fruitLocalY[k], t.fruitLocalZ[k]);
        switch (t.type) {
            case MANGO:     glColor3f(0.85f, 0.55f, 0.1f); glutSolidSphere(0.22f, 8, 6); break;
            case JACKFRUIT: glColor3f(0.55f, 0.42f, 0.1f); glutSolidSphere(0.5f, 8, 6); break;
            case JAM:       glColor3f(0.25f, 0.05f, 0.3f); glutSolidSphere(0.12f, 6, 6); break;
            case LEMON:     glColor3f(0.9f, 0.9f, 0.2f); glutSolidSphere(0.15f, 6, 6); break;
            case GUAVA:     glColor3f(0.75f, 0.85f, 0.4f); glutSolidSphere(0.18f, 6, 6); break;
            case PAPAYA:    glColor3f(0.3f, 0.55f, 0.15f); glutSolidSphere(0.28f, 8, 6); break;
            default:        glColor3f(0.7f, 0.15f, 0.1f); glutSolidSphere(0.13f, 6, 6); break;
        }
        glPopMatrix();
    }

    glPopMatrix();
    glPopMatrix();
}

void drawAllFruitTrees() {
    for (int i = 0; i < NUM_FRUIT_TREES; i++) drawFruitTree(fruitTrees[i]);
}

// ---------------------- Village Flower Beds ----------------------
void drawVillageFlowers() {
    for (int i = 0; i < NUM_HOUSES; i++) {
        for (int k = 0; k < 6; k++) {
            float ang = k * 60.0f * PI / 180.0f + i * 0.35f;
            float rx = houses[i].x + cosf(ang) * (houses[i].hw + 1.3f);
            float rz = houses[i].z + sinf(ang) * (houses[i].hd + 1.3f);
            float bob = sinf(flagTime * 2.0f + i * 1.7f + k) * 0.05f;

            float petalR = (k % 2) ? 0.95f : 0.85f;
            float petalG = (i % 3 == 0) ? 0.15f : 0.75f;
            float petalB = (k % 2) ? 0.65f : 0.15f;

            glColor3f(0.12f, 0.5f, 0.15f);
            glPushMatrix(); glTranslatef(rx, 0.1f, rz); drawBox(0.05f, 0.24f, 0.05f); glPopMatrix();

            glColor3f(petalR, petalG, petalB);
            glPushMatrix(); glTranslatef(rx, 0.24f + bob, rz); glutSolidSphere(0.14f, 8, 6); glPopMatrix();
        }
    }
}

// ====================== ORIANA CAMPUS SYSTEM ======================
enum CampusKind { CAMPUS_SCHOOL=0, CAMPUS_UNIVERSITY=1, CAMPUS_HOSPITAL=2 };

struct Campus {
    float x, z;
    float w, d;
    int floors;
    int kind;
    const char* name;
    bool gateOpen;
    float gateAngle;
};

Campus campuses[3] = {
    { -112.0f, 82.0f, 34.0f, 24.0f, 5,  CAMPUS_SCHOOL,     "ORIANA SCHOOL & COLLEGE",       false, 0.0f },
    { 0.0f, 130.0f, 50.0f, 28.0f, 5,  CAMPUS_UNIVERSITY, "ORIANA UNIVERSITY OF SCIENCE AND TECHNOLOGY", false, 0.0f },
    {  142.0f, 70.0f, 42.0f, 30.0f, 10, CAMPUS_HOSPITAL,   "ORIANA HUMAN CARE HOSPITAL",    false, 0.0f }
};
const float CAMPUS_WALL_H = 2.2f;
const float CAMPUS_GATE_W = 6.0f;
const float CAMPUS_ROAD_W = 6.0f;
const float CAMPUS_FLOOR_H = 3.2f;
const int CAMPUS_PEOPLE_PER_SITE = 12;
const int CAMPUS_TREE_PER_SITE = 14;
const float CAMPUS_BOULEVARD_Z = 27.0f;

struct CampusPerson {
    int campus;
    float t;
    float speed;
    float lane;
    int variant;
    bool entering;
};
CampusPerson campusPeople[3 * CAMPUS_PEOPLE_PER_SITE];

float campusTreeX[3 * CAMPUS_TREE_PER_SITE];
float campusTreeZ[3 * CAMPUS_TREE_PER_SITE];
int campusTreeType[3 * CAMPUS_TREE_PER_SITE];
float campusTreePhase[3 * CAMPUS_TREE_PER_SITE];

bool pointInsideCampusBuilding(float px, float pz, int i) {
    Campus &c = campuses[i];
    return fabs(px-c.x) < c.w*0.5f && fabs(pz-c.z) < c.d*0.5f;
}

bool pointInsideCampusBoundary(float px, float pz, int i) {
    Campus &c = campuses[i];
    return fabs(px-c.x) < c.w*0.5f + 5.0f && fabs(pz-c.z) < c.d*0.5f + 5.0f;
}

bool campusFrontGateOpen(int i) {
    return campuses[i].gateOpen && campuses[i].gateAngle < -40.0f;
}

bool checkCampusCollision(float px, float pz) {
    const float margin = 0.55f;
    for (int i=0; i<3; i++) {
        Campus &c = campuses[i];
        float hw=c.w*0.5f, hd=c.d*0.5f;
        float bx=hw+5.0f, bz=hd+5.0f;

        if (fabs(px-c.x) < hw && fabs(pz-c.z) < hd) {
            bool inside = (fabs(px-c.x) < hw-margin && fabs(pz-c.z) < hd-margin);
            bool doorGap = (pz < c.z-hd+margin &&                  // <-- '>' থেকে '<', +hd থেকে -hd
                            fabs(px-c.x) < CAMPUS_GATE_W*0.5f+0.35f &&
                            campusFrontGateOpen(i));
            if (!inside && !doorGap) return true;
        }

        bool insideBoundary = fabs(px-c.x) < bx && fabs(pz-c.z) < bz;
        if (insideBoundary && !pointInsideCampusBuilding(px,pz,i)) {
            bool gateGap = (pz < c.z-bz+margin &&                  // <-- '>' থেকে '<', +bz থেকে -bz
                            fabs(px-c.x) < CAMPUS_GATE_W*0.5f+0.45f &&
                            campusFrontGateOpen(i));
            bool nearBoundary = (fabs(fabs(px-c.x)-bx) < margin ||
                                 fabs(fabs(pz-c.z)-bz) < margin);
            if (nearBoundary && !gateGap) return true;
        }
    }
    return false;
}

int findNearestCampus(float px, float pz) {
    int best=-1;
    float bestDist=1e9f;
    for (int i=0;i<3;i++) {
        float dx=px-campuses[i].x, dz=pz-campuses[i].z;
        float d=sqrtf(dx*dx+dz*dz);
        if (d<bestDist) { bestDist=d; best=i; }
    }
    if (bestDist < 22.0f) return best;
    return -1;
}

void campusColor(int kind, float &r, float &g, float &b) {
    if (kind==CAMPUS_SCHOOL) {
        r=0.74f; g=0.40f; b=0.28f;
    } else if (kind==CAMPUS_UNIVERSITY) {
        r=0.82f; g=0.74f; b=0.52f;
    } else {
        r=0.93f; g=0.95f; b=0.97f;
    }
}

void drawCampusFlag(float x, float z, float topY) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.28f,0.20f,0.10f);
    glPushMatrix();
    glTranslatef(0.0f, topY*0.5f, 0.0f);
    drawBox(0.16f, topY, 0.16f);
    glPopMatrix();

    glTranslatef(0.0f, topY-2.2f, 0.0f);
    glBegin(GL_QUADS);
    glColor3f(0.0f,0.42f,0.16f);
    for (int i=0;i<12;i++) {
        float u0=(float)i/12.0f, u1=(float)(i+1)/12.0f;
        float yWave0=sinf(flagTime*4.0f+u0*7.0f)*0.28f*u0;
        float yWave1=sinf(flagTime*4.0f+u1*7.0f)*0.28f*u1;
        glVertex3f(u0*3.8f, 1.2f, yWave0);
        glVertex3f(u1*3.8f, 1.2f, yWave1);
        glVertex3f(u1*3.8f,-0.9f, yWave1);
        glVertex3f(u0*3.8f,-0.9f, yWave0);
    }
    glEnd();

    glColor3f(0.85f,0.05f,0.06f);
    glPushMatrix();
    glTranslatef(1.65f,0.15f,0.0f);
    glutSolidSphere(0.48f,18,14);
    glPopMatrix();

    glPopMatrix();
}

// ---------------------- Campus sign (now with optional banner image) ----------------------
void drawCampusSign(const Campus &c) {
    float signW = c.kind==CAMPUS_UNIVERSITY ? 29.0f : (c.kind==CAMPUS_HOSPITAL ? 25.0f : 20.0f);
    float y = c.floors*CAMPUS_FLOOR_H + 0.65f;

    glPushMatrix();
    glTranslatef(c.x, y, c.z-c.d*0.5f-0.18f);
    glColor3f(0.08f,0.10f,0.12f);
    drawBox(signW,1.55f,0.16f);
    glPopMatrix();

    GLuint tex = (c.kind==CAMPUS_SCHOOL) ? texSchoolBanner :
                 (c.kind==CAMPUS_UNIVERSITY) ? texUniversityBanner : texHospitalBanner;
    int texW = (c.kind==CAMPUS_SCHOOL) ? texSchoolBannerW :
               (c.kind==CAMPUS_UNIVERSITY) ? texUniversityBannerW : texHospitalBannerW;
    int texH = (c.kind==CAMPUS_SCHOOL) ? texSchoolBannerH :
               (c.kind==CAMPUS_UNIVERSITY) ? texUniversityBannerH : texHospitalBannerH;

    if (tex != 0) {
        glPushMatrix();
        glTranslatef(c.x, y, c.z-c.d*0.5f-0.28f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);   // <-- নতুন লাইন: mirror ঠিক করার জন্য
        drawTexturedQuadFit(tex, texW, texH, signW - 1.0f, 1.3f);
        glPopMatrix();
    } else {
        glColor3f(1.0f,0.86f,0.25f);
        float tw=(float)strlen(c.name)*0.33f;
        drawText3D(c.x-tw, y-0.25f, c.z-c.d*0.5f-0.28f, c.name);
    }
}
void drawCampusWindows(float cx,float y,float cz,float w,float d,int kind) {
    float glassR=0.18f, glassG=0.52f, glassB=0.70f;
    if (kind==CAMPUS_HOSPITAL) { glassR=0.35f; glassG=0.72f; glassB=0.86f; }

    for (int side=-1; side<=1; side+=2) {
        for (int k=-1;k<=1;k++) {
            float x=cx+k*(w/3.0f);
            glColor3f(glassR,glassG,glassB);
            glPushMatrix();
            glTranslatef(x,y,cz+side*(d*0.5f+0.035f));
            drawBox(2.2f,1.15f,0.06f);
            glPopMatrix();
        }
    }
    for (int side=-1; side<=1; side+=2) {
        for (int k=-1;k<=1;k++) {
            float z=cz+k*(d/3.0f);
            glColor3f(glassR,glassG,glassB);
            glPushMatrix();
            glTranslatef(cx+side*(w*0.5f+0.035f),y,z);
            drawBox(0.06f,1.15f,2.0f);
            glPopMatrix();
        }
    }
}

void drawCampusFloor(int i, int floorIndex) {
    Campus &c=campuses[i];
    float y=floorIndex*CAMPUS_FLOOR_H;
    float wallH=CAMPUS_FLOOR_H-0.18f;
    float hw=c.w*0.5f, hd=c.d*0.5f;
    float cr,cg,cb;
    campusColor(c.kind,cr,cg,cb);

    glColor3f(cr,cg,cb);
    // পুরো (গেটবিহীন) দেয়াল আগে ছিল c.z-hd এ, এখন c.z+hd এ (পেছনের দিক)
    glPushMatrix(); glTranslatef(c.x,y+wallH*0.5f,c.z+hd); drawBox(c.w,wallH,0.28f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x-hw,y+wallH*0.5f,c.z); drawBox(0.28f,wallH,c.d); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x+hw,y+wallH*0.5f,c.z); drawBox(0.28f,wallH,c.d); glPopMatrix();

    float sideW=(c.w-CAMPUS_GATE_W)/2.0f;
    // গেট-গ্যাপ সহ সামনের দেয়াল এখন c.z-hd এ
    glPushMatrix(); glTranslatef(c.x-(CAMPUS_GATE_W*0.5f+sideW*0.5f),y+wallH*0.5f,c.z-hd); drawBox(sideW,wallH,0.28f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x+(CAMPUS_GATE_W*0.5f+sideW*0.5f),y+wallH*0.5f,c.z-hd); drawBox(sideW,wallH,0.28f); glPopMatrix();

    glColor3f(0.55f,0.56f,0.58f);
    glPushMatrix(); glTranslatef(c.x,y+0.05f,c.z); drawBox(c.w+0.35f,0.12f,c.d+0.35f); glPopMatrix();

    if (floorIndex>0) {
        glColor3f(0.35f,0.37f,0.40f);
        glPushMatrix(); glTranslatef(c.x,y,c.z); drawBox(c.w+0.65f,0.18f,c.d+0.65f); glPopMatrix();
    }

    glColor3f(0.78f,0.79f,0.76f);
    for (int room=-1; room<=1; room+=2) {
        float rx=c.x+room*c.w*0.25f;
        glPushMatrix(); glTranslatef(rx,y+wallH*0.5f,c.z-1.2f); drawBox(0.12f,wallH*0.78f,c.d*0.55f); glPopMatrix();
    }

    for (int row=0;row<2;row++) {
        for (int col=0;col<3;col++) {
            float dx=c.x+(col-1)*3.0f;
            float dz=c.z-3.5f+row*3.0f;
            glPushMatrix();
            glTranslatef(dx,y+0.75f,dz);
            drawSimpleDesk(1.5f,0.75f,0.68f);
            glPopMatrix();
        }
    }

    drawCampusWindows(c.x,y+1.75f,c.z,c.w,c.d,c.kind);

    if (floorIndex==0) {
        glColor3f(0.08f,0.10f,0.12f);
        // গ্লাস-ডোর প্যানেল এখন c.z-hd দিকে
        glPushMatrix(); glTranslatef(c.x,y+1.2f,c.z-hd-0.08f); drawBox(CAMPUS_GATE_W-0.25f,2.35f,0.08f); glPopMatrix();
        glColor3f(0.35f,0.75f,0.88f);
        glPushMatrix(); glTranslatef(c.x,y+1.2f,c.z-hd-0.13f); drawBox(CAMPUS_GATE_W-0.55f,2.0f,0.04f); glPopMatrix();
    }
}

void drawCampusBuilding(int i) {
    Campus &c=campuses[i];
    for (int f=0;f<c.floors;f++) drawCampusFloor(i,f);

    glColor3f(0.25f,0.28f,0.30f);
    glPushMatrix();
    glTranslatef(c.x,c.floors*CAMPUS_FLOOR_H+0.22f,c.z);
    drawBox(c.w+0.8f,0.32f,c.d+0.8f);
    glPopMatrix();

    drawCampusSign(c);
    drawCampusFlag(c.x,c.z,c.floors*CAMPUS_FLOOR_H+4.0f);
}

void drawCampusBoundary(int i) {
    Campus &c=campuses[i];
    float bx=c.w*0.5f+5.0f, bz=c.d*0.5f+5.0f;
    float h=CAMPUS_WALL_H;

    glColor3f(0.30f,0.31f,0.32f);
    // পুরো (গেটবিহীন) বাউন্ডারি ওয়াল আগে c.z-bz তে ছিল, এখন c.z+bz তে
    glPushMatrix(); glTranslatef(c.x,h*0.5f,c.z+bz); drawBox(c.w+10.0f,h,0.22f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x-bx,h*0.5f,c.z); drawBox(0.22f,h,c.d+10.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x+bx,h*0.5f,c.z); drawBox(0.22f,h,c.d+10.0f); glPopMatrix();

    float sideW=(c.w+10.0f-CAMPUS_GATE_W)/2.0f;
    // গেট-গ্যাপ সহ পাশের দেয়াল এখন c.z-bz তে
    glPushMatrix(); glTranslatef(c.x-(CAMPUS_GATE_W*0.5f+sideW*0.5f),h*0.5f,c.z-bz); drawBox(sideW,h,0.22f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x+(CAMPUS_GATE_W*0.5f+sideW*0.5f),h*0.5f,c.z-bz); drawBox(sideW,h,0.22f); glPopMatrix();

    glPushMatrix();
    // আসল গেট-পাল্লা (খোলে/বন্ধ হয়) এখন c.z-bz তে
    glTranslatef(c.x-CAMPUS_GATE_W*0.5f,c.z*0.0f+0.0f,c.z-bz);
    glRotatef(c.gateAngle,0.0f,1.0f,0.0f);
    glTranslatef(CAMPUS_GATE_W*0.5f, h*0.5f, 0.0f);
    glColor3f(0.10f,0.12f,0.14f);
    drawBox(CAMPUS_GATE_W,h,0.18f);
    glPopMatrix();

    glColor3f(0.75f,0.65f,0.18f);
    // গেটের উপরের সাজসজ্জার বার-ও একই দিকে
    glPushMatrix(); glTranslatef(c.x,h+0.35f,c.z-bz); drawBox(CAMPUS_GATE_W+1.0f,0.35f,0.30f); glPopMatrix();
}
void drawCampusField(int i) {
    Campus &c=campuses[i];
    float fx=c.x, fz=c.z-c.d*0.5f-12.0f;
    float fw=c.w+10.0f, fd=18.0f;

    glColor3f(0.16f,0.48f,0.16f);
    glPushMatrix(); glTranslatef(fx,0.01f,fz); drawBox(fw,0.06f,fd); glPopMatrix();

    glColor3f(0.78f,0.72f,0.58f);
    glPushMatrix(); glTranslatef(fx,0.045f,fz); drawBox(2.2f,0.03f,fd); glPopMatrix();
    glPushMatrix(); glTranslatef(fx,0.045f,fz+fd*0.25f); drawBox(fw,0.03f,1.3f); glPopMatrix();

    for (int b=-1;b<=1;b+=2) {
        glPushMatrix(); glTranslatef(fx+b*fw*0.30f,0.0f,fz+2.0f); drawBench(3.0f); glPopMatrix();
    }

    for (int k=0;k<CAMPUS_TREE_PER_SITE;k++) {
        int idx=i*CAMPUS_TREE_PER_SITE+k;
        FruitTree t;
        t.x=campusTreeX[idx]; t.z=campusTreeZ[idx]; t.rotY=0.0f;
        t.leanDeg=4.0f+sinf(flagTime*0.7f+campusTreePhase[idx])*3.0f;
        t.leanAxisX=sinf(k*1.37f); t.leanAxisZ=cosf(k*1.37f);
        t.type=campusTreeType[idx];
        t.swayPhase=campusTreePhase[idx];
        t.fruitCount=4+(k%3);
        for (int n=0;n<MAX_FRUITS;n++) {
            t.fruitPicked[n]=false;
            float a=2.0f*PI*(float)n/(float)MAX_FRUITS;
            t.fruitLocalX[n]=cosf(a)*0.9f;
            t.fruitLocalZ[n]=sinf(a)*0.9f;
            t.fruitLocalY[n]=0.6f+(n%2)*0.5f;
        }
        drawFruitTree(t);
    }

    for (int k=0;k<12;k++) {
        float ox=fmodf(k*7.3f,fw)-fw*0.5f;
        float oz=fmodf(k*5.1f,fd)-fd*0.5f;
        glColor3f((k%2)?0.95f:0.9f, (k%3)?0.2f:0.75f, (k%4)?0.25f:0.9f);
        glPushMatrix(); glTranslatef(fx+ox,0.28f,fz+oz); glutSolidSphere(0.16f,8,6); glPopMatrix();
        glColor3f(0.1f,0.45f,0.12f);
        glPushMatrix(); glTranslatef(fx+ox,0.12f,fz+oz); drawBox(0.05f,0.25f,0.05f); glPopMatrix();
    }
}

void drawCampusGuard(int i) {
    Campus &c=campuses[i];
    float z=c.z-c.d*0.5f-5.5f;
    drawSecurityGuard(c.x,z,0.0f,false);
    glColor3f(1.0f,1.0f,1.0f);
    drawText3D(c.x-2.0f,2.9f,z+0.2f,"SECURITY");
}

void drawCampusPerson(const CampusPerson &p) {
     Campus &c = campuses[p.campus];
    float fieldZ = c.z - c.d*0.5f - 12.0f;
    float insideZ = c.z;

    float x=c.x+p.lane;
    float z;
    float ang=0.0f;
    if (p.entering) {
        z=fieldZ-(fieldZ-insideZ)*p.t;
        ang=0.0f;
    } else {
        z=fieldZ + sinf(p.t*6.283f)*4.0f;
        x=c.x + p.lane + cosf(p.t*6.283f)*7.0f;
        ang=90.0f;
    }

    if (c.kind==CAMPUS_HOSPITAL)
        drawHumanoid(x,z,ang,0.80f,0.82f,0.86f,false);
    else
        drawHumanoid(x,z,ang,0.82f,0.82f,0.88f,false);
}

void drawCampusRoads() {
    drawRoadSegment(-170.0f, CAMPUS_BOULEVARD_Z, 170.0f, CAMPUS_BOULEVARD_Z, CAMPUS_ROAD_W);
    drawRoadSegment(0.0f, LOOP_R, 0.0f, CAMPUS_BOULEVARD_Z, CAMPUS_ROAD_W);

    for (int i = 0; i < 3; i++) {
        Campus &c = campuses[i];
        float gateZ = c.z - c.d * 0.5f - 5.0f;
        drawRoadSegment(c.x, CAMPUS_BOULEVARD_Z, c.x, gateZ, CAMPUS_ROAD_W);
    }
}

void drawCampusCars() {
    for (int i=0;i<12;i++) {
        float t=fmodf(flagTime*(2.0f+0.25f*i)+i*17.0f,340.0f);
        float x=-170.0f+t;
        float z=CAMPUS_BOULEVARD_Z + ((i%2)?1.35f:-1.35f);
        float heading=(i%2)?270.0f:90.0f;

        glPushMatrix();
        glTranslatef(x,0.0f,z);
        glRotatef(heading,0,1,0);
        glColor3f((i%3==0)?0.78f:0.15f,(i%3==1)?0.45f:0.18f,(i%2)?0.72f:0.20f);
        glPushMatrix(); glTranslatef(0,0.65f,0); drawBox(1.5f,0.75f,2.8f); glPopMatrix();
        glColor3f(0.55f,0.75f,0.85f);
        glPushMatrix(); glTranslatef(0,1.05f,-0.2f); drawBox(1.15f,0.42f,1.25f); glPopMatrix();
        glColor3f(0.05f,0.05f,0.05f);
        for (int w=-1;w<=1;w+=2) {
            glPushMatrix(); glTranslatef(0.72f,0.32f,w*0.9f); drawBox(0.25f,0.45f,0.45f); glPopMatrix();
            glPushMatrix(); glTranslatef(-0.72f,0.32f,w*0.9f); drawBox(0.25f,0.45f,0.45f); glPopMatrix();
        }
        glPopMatrix();
    }
}

void drawCampusParking(int i) {
    Campus &c=campuses[i];
    int count=(i==CAMPUS_HOSPITAL)?5:3;
    float px=c.x+c.w*0.5f+9.0f;
    float pz=c.z-c.d*0.25f;
    for (int k=0;k<count;k++) {
        float z=pz+(k-(count-1)*0.5f)*3.0f;
        glPushMatrix();
        glTranslatef(px,0.0f,z);
        glRotatef(90.0f,0,1,0);
        glColor3f(0.10f+0.12f*(k%3),0.25f+0.10f*(k%2),0.50f+0.12f*(k%2));
        glPushMatrix(); glTranslatef(0,0.65f,0); drawBox(1.5f,0.75f,2.8f); glPopMatrix();
        glColor3f(0.55f,0.75f,0.85f);
        glPushMatrix(); glTranslatef(0,1.05f,-0.2f); drawBox(1.15f,0.42f,1.25f); glPopMatrix();
        glPopMatrix();
    }
}

void drawAllCampuses() {
    drawCampusRoads();
    for (int i=0;i<3;i++) {
        drawCampusField(i);
        drawCampusBoundary(i);
        drawCampusBuilding(i);
        drawCampusGuard(i);
        drawCampusParking(i);
    }
    for (int i=0;i<3*CAMPUS_PEOPLE_PER_SITE;i++) drawCampusPerson(campusPeople[i]);
    drawCampusCars();
}

void updateCampusPeople() {
    for (int i=0;i<3* CAMPUS_PEOPLE_PER_SITE;i++) {
        CampusPerson &p=campusPeople[i];
        p.t += p.speed*TIMER_DT;
        if (p.t>=1.0f) {
            p.t=0.0f;
            p.entering=!p.entering;
        }
    }
}

void initCampuses() {
    int idx=0;
    for (int i=0;i<3;i++) {
        Campus &c=campuses[i];
        for (int n=0;n<CAMPUS_PEOPLE_PER_SITE;n++) {
            CampusPerson &p=campusPeople[idx++];
            p.campus=i;
            p.t=(float)n/(float)CAMPUS_PEOPLE_PER_SITE;
            p.speed=0.08f+0.015f*(n%4);
            p.lane=((n%5)-2)*1.6f;
            p.variant=n;
            p.entering=(n%2==0);
        }

        float fx=c.x, fz=c.z+c.d*0.5f+12.0f;
        float fw=c.w+10.0f, fd=18.0f;
        for (int k=0;k<CAMPUS_TREE_PER_SITE;k++) {
            int ti=i*CAMPUS_TREE_PER_SITE+k;
            int col=k%5, row=k/5;
            campusTreeX[ti]=fx-fw*0.42f+col*(fw*0.21f)+((row%2)?1.5f:0.0f);
            campusTreeZ[ti]=fz-fd*0.40f+row*(fd*0.28f);
            campusTreeType[ti]=(k+i)%NUM_FRUIT_TYPES;
            campusTreePhase[ti]=0.5f*k+1.2f*i;
        }
    }
}

// ---------------------- Collision ----------------------
bool checkHouseCollision(float px, float pz) {
    const float margin = 0.6f;
    for (int i = 0; i < NUM_HOUSES; i++) {
        House &h = houses[i];
        float dx = px - h.x, dz = pz - h.z;
        float rad = -h.rotY * PI / 180.0f;
        float localX = dx * cos(rad) - dz * sin(rad);
        float localZ = dx * sin(rad) + dz * cos(rad);

        if (fabs(localX) < h.hw && fabs(localZ) < h.hd) {
            bool fullyInside = (fabs(localX) < h.hw - margin) && (fabs(localZ) < h.hd - margin);
            bool inDoorGap = (localZ > h.hd - margin) && (fabs(localX) < h.doorW / 2.0f + 0.3f) && (h.doorAngle < -40.0f);
            if (!fullyInside && !inDoorGap) return true;
        }
    }
    return false;
}

int findNearestHouse(float px, float pz) {
    int best = -1; float bestDist = 1e9f;
    for (int i = 0; i < NUM_HOUSES; i++) {
        float dx = px - houses[i].x, dz = pz - houses[i].z;
        float dist = sqrt(dx*dx + dz*dz);
        if (dist < bestDist) { bestDist = dist; best = i; }
    }
    if (bestDist < 5.0f) return best;
    return -1;
}

void tryPickFruit() {
    int bestTree = -1; float bestDist = 4.0f;
    for (int i = 0; i < NUM_FRUIT_TREES; i++) {
        float dx = player.x - fruitTrees[i].x, dz = player.z - fruitTrees[i].z;
        float dist = sqrt(dx*dx + dz*dz);
        if (dist < bestDist) { bestDist = dist; bestTree = i; }
    }
    if (bestTree < 0) return;

    FruitTree &t = fruitTrees[bestTree];
    for (int k = 0; k < t.fruitCount; k++) {
        if (!t.fruitPicked[k]) { t.fruitPicked[k] = true; fruitsCollected++; break; }
    }
}

// ---------------------- HUD ----------------------
void drawHUD() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 700);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);

    glColor3f(1.0f, 1.0f, 1.0f);
    char buf[200];
    sprintf(buf, "WASD move | E gate/door | P pick fruit | B boat | N day/night | T train | Campus gates: E | Fruits: %d", fruitsCollected);
    glRasterPos2i(15, 670);
    for (char* c = buf; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
}

// ---------------------- Mountain (village এর এক প্রান্তে) ----------------------
void drawMountain() {
    float baseZ = -150.0f;

    struct Peak { float xOffset, baseHalf, peakHeight; };
    Peak peaks[3] = {
        { -45.0f, 35.0f, 30.0f },
        {   0.0f, 50.0f, 42.0f },
        {  45.0f, 32.0f, 26.0f }
    };

    for (int p = 0; p < 3; p++) {
        glPushMatrix();
        glTranslatef(peaks[p].xOffset, 0.0f, baseZ);

        float bh = peaks[p].baseHalf;
        float ph = peaks[p].peakHeight;

        float verts[4][2] = {
            {-bh, -bh}, { bh, -bh},
            { bh,  bh}, {-bh,  bh}
        };

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < 4; i++) {
            int j = (i + 1) % 4;

            glColor3f(0.10f, 0.42f, 0.10f);
            glVertex3f(verts[i][0], 0.0f, verts[i][1]);

            glColor3f(0.12f, 0.46f, 0.12f);
            glVertex3f(verts[j][0], 0.0f, verts[j][1]);

            glColor3f(0.40f, 0.27f, 0.14f);
            glVertex3f(0.0f, ph, 0.0f);
        }
        glEnd();

        glPopMatrix();
    }
}

// ---------------------- Animated River (পাহাড়ের সামনে) ----------------------
void drawAnimatedRiver() {
    float riverCenterZ = -110.0f;
    float riverHalfDepth = 36.0f;
    float riverHalfWidth = 150.0f;

    glPushMatrix();
    glTranslatef(0.0f, 0.05f, riverCenterZ);

    int segments = 60;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float x = -riverHalfWidth + t * 2.0f * riverHalfWidth;

        float wave = sinf(x * 0.15f - waveOffset) * 1.5f;

        glColor3f(0.10f, 0.28f, 0.65f);
        glVertex3f(x, 0.0f, -riverHalfDepth + wave * 0.3f);

        glColor3f(0.35f, 0.65f, 1.0f);
        glVertex3f(x, 0.0f,  riverHalfDepth + wave * 0.3f);
    }
    glEnd();

    glPopMatrix();
}

// ---------------------- Railway ----------------------
void drawRailwayTrack() {
    glColor3f(0.35f, 0.25f, 0.15f);
    int steps = (int)(RAIL_TOTAL_LEN / 1.2f);
    for (int i = 0; i <= steps; i++) {
        float z = RAIL_Z_START - i * 1.2f;
        float y = railHeightAt(z);
        glPushMatrix();
        glTranslatef(RAIL_X, y + 0.05f, z);
        drawBox(2.4f, 0.1f, 0.3f);
        glPopMatrix();
    }

    glColor3f(0.55f, 0.55f, 0.58f);
    int railSteps = (int)(RAIL_TOTAL_LEN / 2.0f);
    for (int side = -1; side <= 1; side += 2) {
        for (int i = 0; i < railSteps; i++) {
            float z0 = RAIL_Z_START - i * 2.0f, z1 = RAIL_Z_START - (i + 1) * 2.0f;
            float y0 = railHeightAt(z0) + 0.12f, y1 = railHeightAt(z1) + 0.12f;
            glBegin(GL_QUADS);
                glVertex3f(RAIL_X + side * 0.9f - 0.08f, y0, z0);
                glVertex3f(RAIL_X + side * 0.9f + 0.08f, y0, z0);
                glVertex3f(RAIL_X + side * 0.9f + 0.08f, y1, z1);
                glVertex3f(RAIL_X + side * 0.9f - 0.08f, y1, z1);
            glEnd();
        }
    }

    glColor3f(0.5f, 0.5f, 0.5f);
    for (float z = RIVER_Z1 + 5.0f; z < RIVER_Z2; z += 12.0f) {
        float y = railHeightAt(z);
        glPushMatrix();
        glTranslatef(RAIL_X, y / 2.0f, z);
        drawBox(1.0f, y, 1.0f);
        glPopMatrix();
    }
}

void drawRailwayStation() {
    float stationZ = RAIL_Z_START - 8.0f;

    glColor3f(0.65f, 0.6f, 0.55f);
    glPushMatrix(); glTranslatef(RAIL_X + 2.0f, 0.4f, stationZ); drawBox(3.0f, 0.8f, 20.0f); glPopMatrix();

    glColor3f(0.8f, 0.65f, 0.4f);
    glPushMatrix(); glTranslatef(RAIL_X + 5.0f, 1.8f, stationZ); drawBox(4.0f, 3.6f, 6.0f); glPopMatrix();

    glColor3f(0.5f, 0.2f, 0.15f);
    glPushMatrix(); glTranslatef(RAIL_X + 5.0f, 3.8f, stationZ); drawBox(4.6f, 0.3f, 6.6f); glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText3D(RAIL_X + 3.2f, 3.0f, stationZ + 3.4f, "RAILWAY STATION");
}

void drawTrain() {
    float gap = CARRIAGE_LEN + CARRIAGE_GAP;
    for (int i = 0; i < NUM_CARRIAGES; i++) {
        float headDist = trainDist - i * gap;
        if (headDist < 0.0f || headDist > RAIL_TOTAL_LEN) continue;

        float centerDist = headDist - CARRIAGE_LEN / 2.0f;
        if (centerDist < 0.0f) centerDist = 0.0f;
        float z = RAIL_Z_START - centerDist;
        float y = railHeightAt(z) + 0.9f;

        glPushMatrix();
        glTranslatef(RAIL_X, y, z);

        if (i == 0) {
            glColor3f(0.55f, 0.1f, 0.08f);
            drawBox(2.2f, 1.5f, CARRIAGE_LEN);
            glColor3f(0.15f, 0.15f, 0.15f);
            glPushMatrix(); glTranslatef(0.0f, 1.1f, CARRIAGE_LEN/2.0f - 1.0f); drawBox(2.0f, 1.3f, 1.6f); glPopMatrix();
        } else {
            float col[3][3] = { {0.2f,0.35f,0.6f}, {0.6f,0.55f,0.15f}, {0.35f,0.5f,0.3f} };
            int ci = (i - 1) % 3;
            glColor3f(col[ci][0], col[ci][1], col[ci][2]);
            drawBox(2.0f, 1.3f, CARRIAGE_LEN);
        }

        glColor3f(0.05f, 0.05f, 0.05f);
        float wz[2] = { -CARRIAGE_LEN/2.0f + 1.0f, CARRIAGE_LEN/2.0f - 1.0f };
        for (float wzp : wz) {
            glPushMatrix(); glTranslatef(-0.9f, -0.85f, wzp); drawBox(0.3f, 0.5f, 0.5f); glPopMatrix();
            glPushMatrix(); glTranslatef(0.9f, -0.85f, wzp); drawBox(0.3f, 0.5f, 0.5f); glPopMatrix();
        }

        glPopMatrix();
    }
}

void initWaterfallFX() {
    for (int i = 0; i < MAX_WF_PARTICLES; i++) {
        wfParticles[i].t = (float)(rand() % 100) / 100.0f;
        wfParticles[i].speed = 0.35f + (rand() % 100) / 400.0f;
        wfParticles[i].xJitter = ((float)(rand() % 100) / 100.0f - 0.5f) * 1.6f;
        wfParticles[i].size = 0.08f + (rand() % 10) / 100.0f;
        wfParticles[i].alpha = 0.6f + (rand() % 40) / 100.0f;
    }
    for (int i = 0; i < MAX_WF_MIST; i++) {
        float bx, by, bz, bw;
        waterfallPathAt(1.0f, bx, by, bz, bw);
        wfMist[i].x = bx + (-3.0f + (rand() % 60) / 10.0f);
        wfMist[i].y = by + (rand() % 15) / 10.0f;
        wfMist[i].z = bz + (-2.0f + (rand() % 40) / 10.0f);
        wfMist[i].size = 0.4f + (rand() % 10) / 10.0f;
        wfMist[i].alpha = 0.12f + (rand() % 20) / 100.0f;
        wfMist[i].drift = 0.01f + (rand() % 10) / 500.0f;
    }
}

void updateWaterfallFX() {
    for (int i = 0; i < MAX_WF_PARTICLES; i++) {
        wfParticles[i].t += wfParticles[i].speed * TIMER_DT;
        if (wfParticles[i].t > 1.0f) {
            wfParticles[i].t = 0.0f;
            wfParticles[i].xJitter = ((float)(rand() % 100) / 100.0f - 0.5f) * 1.6f;
        }
    }
    for (int i = 0; i < MAX_WF_MIST; i++) {
        wfMist[i].y += wfMist[i].drift;
        wfMist[i].x += sinf(flagTime * 2.0f + i) * 0.005f;
        wfMist[i].alpha -= 0.0006f;
        if (wfMist[i].alpha <= 0.0f) {
            float bx, by, bz, bw;
            waterfallPathAt(1.0f, bx, by, bz, bw);
            wfMist[i].x = bx + (-3.0f + (rand() % 60) / 10.0f);
            wfMist[i].y = by + (rand() % 5) / 10.0f;
            wfMist[i].z = bz + (-2.0f + (rand() % 40) / 10.0f);
            wfMist[i].alpha = 0.12f + (rand() % 20) / 100.0f;
        }
    }
}

void drawWaterfall() {
    int bands = 30;

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= bands; i++) {
        float t = (float)i / bands;
        float x, y, z, halfW;
        waterfallPathAt(t, x, y, z, halfW);

        float flow = sinf(t * 20.0f - waterfallOffset) * 0.5f + 0.5f;
        float shade = 0.6f + flow * 0.4f;
        glColor3f(shade * 0.8f, shade * 0.9f, 1.0f);

        glVertex3f(x - halfW, y, z);
        glVertex3f(x + halfW, y, z);
    }
    glEnd();

    glColor3f(0.85f, 0.93f, 1.0f);
    const int NUM_DROPS = 16;
    for (int i = 0; i < NUM_DROPS; i++) {
        float phase = fmodf(waterfallOffset * 1.3f + i * (100.0f / NUM_DROPS), 100.0f);
        float t = phase / 100.0f;

        float x, y, z, halfW;
        waterfallPathAt(t, x, y, z, halfW);
        float xJitter = sinf(i * 12.9898f) * halfW * 0.8f;
        float zJitter = cosf(i * 78.233f) * 0.25f;

        glPushMatrix();
        glTranslatef(x + xJitter, y, z + zJitter);
        glutSolidSphere(0.18f, 6, 5);
        glPopMatrix();
    }

    glColor3f(0.9f, 0.95f, 1.0f);
    float bx, by, bz, bw;
    waterfallPathAt(1.0f, bx, by, bz, bw);
    for (int i = 0; i < 6; i++) {
        float fx = bx - bw + ((float)i / 5.0f) * (bw * 2.0f);
        float bob = sinf(waterfallOffset * 2.0f + i) * 0.15f;
        glPushMatrix();
        glTranslatef(fx, by + 0.2f + bob, bz + 0.4f);
        glutSolidSphere(0.35f, 8, 6);
        glPopMatrix();
    }
}

void drawWaterfallParticlesAndMist() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_WF_PARTICLES; i++) {
        float x, y, z, halfW;
        waterfallPathAt(wfParticles[i].t, x, y, z, halfW);
        glColor4f(0.85f, 0.93f, 1.0f, wfParticles[i].alpha);
        glVertex3f(x + wfParticles[i].xJitter * halfW, y, z);
    }
    glEnd();

    for (int i = 0; i < MAX_WF_MIST; i++) {
        glColor4f(1.0f, 1.0f, 1.0f, wfMist[i].alpha);
        glPushMatrix();
        glTranslatef(wfMist[i].x, wfMist[i].y, wfMist[i].z);
        float s = wfMist[i].size;
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);
        for (int a = 0; a <= 10; a++) {
            float ang = a * (2.0f * PI / 10.0f);
            glVertex3f(cosf(ang) * s, sinf(ang) * s * 0.6f, 0.0f);
        }
        glEnd();
        glPopMatrix();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void initFish() {
    for (int i = 0; i < MAX_FISH; i++) {
        fishX[i] = (((float)rand()/RAND_MAX)*2.0f - 1.0f) * RIVER_HALF_WIDTH;
        fishZ[i] = RIVER_CENTER_Z + (((float)rand()/RAND_MAX)*2.0f - 1.0f) * RIVER_HALF_DEPTH;
        fishJumping[i] = false;
        fishJumpT[i] = 0.0f;
        fishWaitTimer[i] = 1.0f + (rand() % 400) / 100.0f;
        fishJumpDuration[i] = 0.8f + (rand() % 60) / 100.0f;
        fishJumpHeight[i] = 1.2f + (rand() % 100) / 100.0f;
        fishSize[i] = 0.18f + (rand() % 12) / 100.0f;
        fishRotY[i] = ((float)rand()/RAND_MAX) * 360.0f;
    }
}

void updateFish() {
    for (int i = 0; i < MAX_FISH; i++) {
        if (!fishJumping[i]) {
            fishWaitTimer[i] -= TIMER_DT;
            if (fishWaitTimer[i] <= 0.0f) {
                fishJumping[i] = true;
                fishJumpT[i] = 0.0f;
                fishX[i] = (((float)rand()/RAND_MAX)*2.0f - 1.0f) * RIVER_HALF_WIDTH;
                fishZ[i] = RIVER_CENTER_Z + (((float)rand()/RAND_MAX)*2.0f - 1.0f) * RIVER_HALF_DEPTH;
                fishRotY[i] = ((float)rand()/RAND_MAX) * 360.0f;
            }
        } else {
            fishJumpT[i] += TIMER_DT / fishJumpDuration[i];
            if (fishJumpT[i] >= 1.0f) {
                fishJumpT[i] = 0.0f;
                fishJumping[i] = false;
                fishWaitTimer[i] = 1.5f + (rand() % 500) / 100.0f;
            }
        }
    }
}

void drawFish(int i) {
    if (!fishJumping[i]) return;

    float t = fishJumpT[i];
    float y = 4.0f * fishJumpHeight[i] * t * (1.0f - t);
    float tilt = (0.5f - t) * 60.0f;

    glPushMatrix();
    glTranslatef(fishX[i], 0.05f + y, fishZ[i]);
    glRotatef(fishRotY[i], 0.0f, 1.0f, 0.0f);
    glRotatef(tilt, 1.0f, 0.0f, 0.0f);

    glColor3f(0.55f, 0.6f, 0.65f);
    glPushMatrix();
    glScalef(fishSize[i] * 2.2f, fishSize[i], fishSize[i] * 0.9f);
    glutSolidSphere(1.0, 10, 8);
    glPopMatrix();

    glColor3f(0.45f, 0.5f, 0.55f);
    glPushMatrix();
    glTranslatef(-fishSize[i] * 2.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glutSolidCone(fishSize[i] * 0.9f, fishSize[i] * 1.2f, 6, 4);
    glPopMatrix();

    glPopMatrix();

    if (t < 0.08f || t > 0.92f) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        glPushMatrix();
        glTranslatef(fishX[i], 0.06f, fishZ[i]);
        glutSolidSphere(fishSize[i] * 1.6f, 8, 6);
        glPopMatrix();
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
}

void drawAllFish() {
    for (int i = 0; i < MAX_FISH; i++) drawFish(i);
}

void boatHullAt(float t, float &halfW, float &topY, float &bottomY) {
    float w;
    if (t < 0.45f) w = t / 0.45f;
    else           w = 1.0f;
    halfW = BOAT_HALF_W * powf(w, 0.6f);

    float rise = (t < 0.22f) ? (0.22f - t) / 0.22f : 0.0f;
    topY    = BOAT_DECK_Y + rise * 1.4f;
    bottomY = -0.5f * (0.35f + 0.65f * w);
}

void drawBoat() {
    glPushMatrix();
    glTranslatef(boatX, 0.15f, boatZ);
    glRotatef(boatAngle, 0.0f, 1.0f, 0.0f);

    const int SEG = 20;

    for (int i = 0; i < SEG; i++) {
        float t0 = (float)i / SEG, t1 = (float)(i + 1) / SEG;
        float z0 = -BOAT_LEN/2.0f + t0 * BOAT_LEN;
        float z1 = -BOAT_LEN/2.0f + t1 * BOAT_LEN;
        float hw0, top0, bot0, hw1, top1, bot1;
        boatHullAt(t0, hw0, top0, bot0);
        boatHullAt(t1, hw1, top1, bot1);

        glColor3f(0.08f, 0.12f, 0.32f);
        glBegin(GL_QUADS);
            glVertex3f(0.0f, bot0, z0); glVertex3f(-hw0, top0, z0);
            glVertex3f(-hw1, top1, z1); glVertex3f(0.0f, bot1, z1);
        glEnd();
        glBegin(GL_QUADS);
            glVertex3f(0.0f, bot0, z0); glVertex3f(0.0f, bot1, z1);
            glVertex3f(hw1, top1, z1); glVertex3f(hw0, top0, z0);
        glEnd();

        glColor3f(0.05f, 0.08f, 0.22f);
        glBegin(GL_QUADS);
            glVertex3f(-hw0, top0, z0); glVertex3f(-hw0, top0+0.08f, z0);
            glVertex3f(-hw1, top1+0.08f, z1); glVertex3f(-hw1, top1, z1);
        glEnd();
        glBegin(GL_QUADS);
            glVertex3f(hw0, top0, z0); glVertex3f(hw0, top0+0.08f, z0);
            glVertex3f(hw1, top1+0.08f, z1); glVertex3f(hw1, top1, z1);
        glEnd();
    }

    glColor3f(0.75f, 0.1f, 0.08f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= SEG; i++) {
        float t = (float)i / SEG;
        float z = -BOAT_LEN/2.0f + t * BOAT_LEN;
        float hw, top, bot;
        boatHullAt(t, hw, top, bot);
        float dhw = hw * 0.85f;
        glVertex3f(-dhw, BOAT_DECK_Y + 0.02f, z);
        glVertex3f( dhw, BOAT_DECK_Y + 0.02f, z);
    }
    glEnd();

    glPushMatrix();
    glTranslatef(0.0f, BOAT_DECK_Y + 0.65f, -1.8f);

    glColor3f(0.78f, 0.78f, 0.78f);
    drawBox(1.5f, 1.3f, 1.6f);

    glColor3f(0.55f, 0.8f, 0.95f);
    glPushMatrix(); glTranslatef(0.0f, 0.15f, 0.81f); drawBox(1.1f, 0.6f, 0.05f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.76f, 0.15f, 0.0f); drawBox(0.05f, 0.6f, 1.4f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.76f, 0.15f, 0.0f); drawBox(0.05f, 0.6f, 1.4f); glPopMatrix();

    glColor3f(0.85f, 0.1f, 0.1f);
    glPushMatrix(); glTranslatef(0.0f, 0.85f, 0.0f); drawBox(1.9f, 0.25f, 2.0f); glPopMatrix();

    glPopMatrix();

    float railX = BOAT_HALF_W * 0.92f;
    float railStartZ = 0.3f, railEndZ = BOAT_LEN/2.0f - 0.6f;
    int postCount = 5;
    float postY = BOAT_DECK_Y;

    for (int side = -1; side <= 1; side += 2) {
        float prevX=0, prevY=0, prevZ=0;
        for (int i = 0; i < postCount; i++) {
            float t = (float)i / (postCount - 1);
            float pz = railStartZ + t * (railEndZ - railStartZ);
            float px = side * railX;

            glColor3f(0.6f, 0.6f, 0.62f);
            glPushMatrix();
            glTranslatef(px, postY + 0.35f, pz);
            drawBox(0.1f, 0.7f, 0.1f);
            glPopMatrix();

            if (i > 0) {
                glColor3f(0.95f, 0.8f, 0.1f);
                int ropeSeg = 5;
                for (int s = 0; s < ropeSeg; s++) {
                    float s0 = (float)s / ropeSeg, s1 = (float)(s+1) / ropeSeg;
                    float sag0 = sinf(PI * s0) * 0.15f;
                    float sag1 = sinf(PI * s1) * 0.15f;
                    float z0 = prevZ + (pz - prevZ) * s0;
                    float z1 = prevZ + (pz - prevZ) * s1;
                    float y0 = postY + 0.65f - sag0;
                    float y1 = postY + 0.65f - sag1;
                    float mz = (z0+z1)/2.0f, my = (y0+y1)/2.0f;
                    float dz = z1 - z0, dy = y1 - y0;
                    float len = sqrtf(dz*dz + dy*dy);
                    float ang = atan2f(dy, dz) * 180.0f / PI;
                    glPushMatrix();
                    glTranslatef(px, my, mz);
                    glRotatef(-ang, 1.0f, 0.0f, 0.0f);
                    drawBox(0.04f, 0.04f, len);
                    glPopMatrix();
                }
            }
            prevX = px; prevY = postY + 0.65f; prevZ = pz;
        }
    }

    glPushMatrix();
    glTranslatef(0.35f, BOAT_DECK_Y, -1.9f);
    glScalef(0.55f, 0.55f, 0.55f);
    drawHumanoid(0.0f, 0.0f, 200.0f, 0.85f, 0.5f, 0.1f, false);
    glPopMatrix();

    glPopMatrix();
}

void update(int value) {
    waveOffset += 0.05f;
    if (waveOffset > 31.415f) waveOffset = 0.0f;

    waterfallOffset += 0.15f;
    if (waterfallOffset > 100.0f) waterfallOffset = 0.0f;

    float target = isNight ? 1.0f : 0.0f;
    if (dayNightBlend < target) {
        dayNightBlend += DAYNIGHT_SPEED * 0.016f;
        if (dayNightBlend > target) dayNightBlend = target;
    } else if (dayNightBlend > target) {
        dayNightBlend -= DAYNIGHT_SPEED * 0.016f;
        if (dayNightBlend < target) dayNightBlend = target;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ---------------------- Display / Reshape ----------------------
void display() {
    float dayR=0.55f, dayG=0.75f, dayB=0.95f;
    float nightR=0.02f, nightG=0.03f, nightB=0.09f;
    float skyR = dayR + (nightR - dayR) * dayNightBlend;
    float skyG = dayG + (nightG - dayG) * dayNightBlend;
    float skyB = dayB + (nightB - dayB) * dayNightBlend;
    glClearColor(skyR, skyG, skyB, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float lightIntensity = 1.0f - dayNightBlend * 0.75f;
    GLfloat lightColor[] = { lightIntensity, lightIntensity, lightIntensity * 1.05f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

    GLfloat ambientColor[] = { 0.15f + 0.05f*(1.0f-dayNightBlend), 0.15f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

    float angleRad = player.angle * PI / 180.0f;
    float camX = player.x - sin(angleRad) * camDist;
    float camZ = player.z - cos(angleRad) * camDist;
    gluLookAt(camX, camHeight, camZ, player.x, 3.0f, player.z, 0.0f, 1.0f, 0.0f);

    drawGround();
    drawVillageRoads();
    drawMansion();
    drawTreesAroundMansion();
    drawAllFruitTrees();
    drawVillageFlowers();
    drawBarricade();
    drawVillageHouses();
    drawAllGuards();
    drawAllPedestrians();
    drawAllVehicles();
    drawPlayer();
    drawMountain();
    drawAnimatedRiver();
    drawRailwayTrack();
    drawRailwayStation();
    drawTrain();
    drawAllFish();
    drawBoat();
    drawWaterfall();
    drawWaterfallParticlesAndMist();

    drawAllShops();
    drawAllCampuses();

    glFlush();
    drawHUD();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 1.0, 400.0);
    glMatrixMode(GL_MODELVIEW);
}

// ---------------------- Keyboard ----------------------
void keyboardDown(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W': w_down = true; break;
        case 's': case 'S': s_down = true; break;
        case 'a': case 'A': a_down = true; break;
        case 'd': case 'D': d_down = true; break;
        case 'n': case 'N': isNight = !isNight; break;
        case 't': case 'T': trainRunning = !trainRunning; break;
        case 'e': case 'E': {
    float ddx = player.x - 0.0f, ddz = player.z - MANSION_HD;
    if (sqrt(ddx*ddx + ddz*ddz) < 6.0f) {
        mansionDoorOpen = !mansionDoorOpen;
    } else {
        int sIdx = findNearestShop(player.x, player.z);
        if (sIdx >= 0) {
            shopDoorOpen[sIdx] = !shopDoorOpen[sIdx];
        } else {
            int cIdx = findNearestCampus(player.x, player.z);
            if (cIdx >= 0) {
                float gx=player.x-campuses[cIdx].x;
                float gz=player.z-(campuses[cIdx].z+campuses[cIdx].d*0.5f+5.0f);
                if (sqrtf(gx*gx+gz*gz) < 9.0f) campuses[cIdx].gateOpen = !campuses[cIdx].gateOpen;
            } else {
                int idx = findNearestHouse(player.x, player.z);
                if (idx >= 0) houses[idx].doorOpen = !houses[idx].doorOpen;
            }
        }
    }
    break;
}

        case 'p': case 'P': tryPickFruit(); break;
        case 'g': case 'G': selectedGuard = (selectedGuard + 1) % 2; break;
        case 'z': case 'Z': camDist += 1.0f; break;
        case 'x': case 'X': camDist = (camDist > 5.0f) ? camDist - 1.0f : camDist; break;
        case 'r': case 'R': camHeight += 1.0f; break;
        case 'f': case 'F': camHeight = (camHeight > 2.0f) ? camHeight - 1.0f : camHeight; break;

        case 'b': case 'B': {
    if (!inBoat) {
        float dx = player.x - boatX, dz = player.z - boatZ;
        if (sqrt(dx*dx + dz*dz) < 4.0f) {
            inBoat = true;
            player.x = boatX;
            player.z = boatZ;
            player.angle = boatAngle;
        }
    } else {
        inBoat = false;
        player.x = boatX;
        player.z = RIVER_CENTER_Z + RIVER_HALF_DEPTH + 2.0f;
    }
    break;
}
        case 27: exit(0);
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W': w_down = false; break;
        case 's': case 'S': s_down = false; break;
        case 'a': case 'A': a_down = false; break;
        case 'd': case 'D': d_down = false; break;
    }
}

void specialDown(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP: up_down = true; break;
        case GLUT_KEY_DOWN: down_down = true; break;
        case GLUT_KEY_LEFT: left_down = true; break;
        case GLUT_KEY_RIGHT: right_down = true; break;
    }
}

void specialUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP: up_down = false; break;
        case GLUT_KEY_DOWN: down_down = false; break;
        case GLUT_KEY_LEFT: left_down = false; break;
        case GLUT_KEY_RIGHT: right_down = false; break;
    }
}

// ---------------------- Timer ----------------------
void timerUpdate(int value) {
    if (inBoat) {
    if (a_down) boatAngle += playerTurnSpeed * TIMER_DT;
    if (d_down) boatAngle -= playerTurnSpeed * TIMER_DT;
    float bAngleRad = boatAngle * PI / 180.0f;
    float bMove = 0.0f;
    if (w_down) bMove += boatSpeed * TIMER_DT;
    if (s_down) bMove -= boatSpeed * TIMER_DT;
    if (bMove != 0.0f) {
        float nx = boatX + sin(bAngleRad) * bMove;
        float nz = boatZ + cos(bAngleRad) * bMove;

        float minZ = RIVER_CENTER_Z - RIVER_HALF_DEPTH + BOAT_MARGIN;
        float maxZ = RIVER_CENTER_Z + RIVER_HALF_DEPTH - BOAT_MARGIN;
        float minX = -RIVER_HALF_WIDTH + BOAT_MARGIN;
        float maxX =  RIVER_HALF_WIDTH - BOAT_MARGIN;

        if (nx < minX) nx = minX;
        if (nx > maxX) nx = maxX;
        if (nz < minZ) nz = minZ;
        if (nz > maxZ) nz = maxZ;

        boatX = nx; boatZ = nz;
    }
    player.x = boatX;
    player.z = boatZ;
    player.angle = boatAngle;
} else {
    if (a_down) player.angle += playerTurnSpeed * TIMER_DT;
    if (d_down) player.angle -= playerTurnSpeed * TIMER_DT;
    float pAngleRad = player.angle * PI / 180.0f;
    float moveDist = 0.0f;
    if (w_down) moveDist += playerSpeed * TIMER_DT;
    if (s_down) moveDist -= playerSpeed * TIMER_DT;
    if (moveDist != 0.0f) {
        float nx = player.x + sin(pAngleRad) * moveDist;
        float nz = player.z + cos(pAngleRad) * moveDist;
        if (nx > -GROUND_SIZE+2 && nx < GROUND_SIZE-2 && nz > -GROUND_SIZE+2 && nz < GROUND_SIZE-2) {

           if (!checkHouseCollision(nx, nz) && !checkShopCollision(nx, nz) && !checkMansionCollision(nx, nz) && !checkCampusCollision(nx, nz)) { player.x = nx; player.z = nz; }
        }
    }
}

    Guard &g = guards[selectedGuard];
    if (left_down) g.angle += guardTurnSpeed * TIMER_DT;
    if (right_down) g.angle -= guardTurnSpeed * TIMER_DT;
    float gAngleRad = g.angle * PI / 180.0f;
    float gMove = 0.0f;
    if (up_down) gMove += guardSpeed * TIMER_DT;
    if (down_down) gMove -= guardSpeed * TIMER_DT;
    g.x += sin(gAngleRad) * gMove;
    g.z += cos(gAngleRad) * gMove;
    if (g.x > 40.0f) g.x = 40.0f; if (g.x < -40.0f) g.x = -40.0f;
    if (g.z > 40.0f) g.z = 40.0f; if (g.z < -40.0f) g.z = -40.0f;

    for (int i = 0; i < NUM_PEDS; i++) peds[i].t += peds[i].speed * TIMER_DT;
    for (int i = 0; i < NUM_VEHICLES; i++) vehicles[i].t += vehicles[i].speed * TIMER_DT;

    for (int i = 0; i < NUM_HOUSES; i++) {
        float target = houses[i].doorOpen ? -100.0f : 0.0f;
        if (houses[i].doorAngle < target) {
            houses[i].doorAngle += 4.0f;
            if (houses[i].doorAngle > target) houses[i].doorAngle = target;
        } else if (houses[i].doorAngle > target) {
            houses[i].doorAngle -= 4.0f;
            if (houses[i].doorAngle < target) houses[i].doorAngle = target;
        }
    }
for (int i = 0; i < NUM_SHOPS; i++) {
    float target = shopDoorOpen[i] ? -100.0f : 0.0f;
    if (shopDoorAngle[i] < target) { shopDoorAngle[i] += 4.0f; if (shopDoorAngle[i] > target) shopDoorAngle[i] = target; }
    else if (shopDoorAngle[i] > target) { shopDoorAngle[i] -= 4.0f; if (shopDoorAngle[i] < target) shopDoorAngle[i] = target; }
}

{
    float target = mansionDoorOpen ? -100.0f : 0.0f;
    if (mansionDoorAngle < target) { mansionDoorAngle += 4.0f; if (mansionDoorAngle > target) mansionDoorAngle = target; }
    else if (mansionDoorAngle > target) { mansionDoorAngle -= 4.0f; if (mansionDoorAngle < target) mansionDoorAngle = target; }
}

    for (int i=0; i<3; i++) {
        float target = campuses[i].gateOpen ? -95.0f : 0.0f;
        if (campuses[i].gateAngle < target) {
            campuses[i].gateAngle += 5.0f;
            if (campuses[i].gateAngle > target) campuses[i].gateAngle = target;
        } else if (campuses[i].gateAngle > target) {
            campuses[i].gateAngle -= 5.0f;
            if (campuses[i].gateAngle < target) campuses[i].gateAngle = target;
        }
    }

    updateCampusPeople();

    flagTime += TIMER_DT;

    if (trainRunning) {
        trainDist += trainSpeed * TIMER_DT;
        float cycle = RAIL_TOTAL_LEN + NUM_CARRIAGES * (CARRIAGE_LEN + CARRIAGE_GAP) + 40.0f;
        if (trainDist > cycle) trainDist = 0.0f;
    }

    updateFish();
    updateWaterfallFX();

    glutPostRedisplay();
    glutTimerFunc((int)(TIMER_DT * 1000), timerUpdate, 0);
}

// ---------------------- Init ----------------------
void initHouses() {
    srand(42);
    float typeHalfW[4] = {3.0f, 3.25f, 2.5f, 2.75f};
    float typeHalfD[4] = {2.5f, 2.75f, 2.25f, 2.25f};
    float typeDoorW[4] = {1.0f, 1.1f, 0.9f, 0.9f};
    float typeDoorH[4] = {1.9f, 2.0f, 1.7f, 1.8f};

    for (int i = 0; i < NUM_HOUSES; i++) {
        float x, z;
        int attempts = 0;
        bool ok;
        do {
            ok = true;
            float ang = ((float)rand() / RAND_MAX) * 2.0f * PI;
            float rad = 26.0f + ((float)rand() / RAND_MAX) * 22.0f;
            x = sin(ang) * rad;
            z = cos(ang) * rad;

            if (fabs(x) < 5.0f && z > 18.0f) ok = false;
            if (z < -80.0f) ok = false;

            if (fabs(x) < BARRIER + 4.0f && fabs(z) < BARRIER + 4.0f) ok = false;
            if (fabs(x - RAIL_X) < 8.0f) ok = false;

            for (int j = 0; j < i && ok; j++) {
                float dx = x - houses[j].x, dz = z - houses[j].z;
                if (sqrt(dx*dx+dz*dz) < 10.0f) ok = false;
            }
            attempts++;
        } while (!ok && attempts < 200);

        houses[i].x = x;
        houses[i].z = z;
        houses[i].type = i % 4;

        float ox, oz;
        nearestPointOnRect(x, z, LOOP_R, ox, oz);
        houses[i].rotY = atan2(ox - x, oz - z) * 180.0f / PI;

        int t = houses[i].type;
        houses[i].hw = typeHalfW[t];
        houses[i].hd = typeHalfD[t];
        houses[i].doorW = typeDoorW[t];
        houses[i].doorH = typeDoorH[t];
        houses[i].doorOpen = false;
        houses[i].doorAngle = 0.0f;
    }
}

void initFruitTrees() {
    for (int i = 0; i < NUM_FRUIT_TREES; i++) {
        float x, z;
        int attempts = 0;
        bool ok;
        do {
            ok = true;
            x = (((float)rand()/RAND_MAX)*2.0f - 1.0f) * (GROUND_SIZE - 6.0f);
            z = (((float)rand()/RAND_MAX)*2.0f - 1.0f) * (GROUND_SIZE - 6.0f);

             if (z < -80.0f) ok = false;
            if (sqrt(x*x+z*z) < BARRIER + 6.0f) ok = false;
            if (fabs(x) < 5.0f && z > 15.0f && z < LOOP_R + 3.0f) ok = false;
            if (fabs(x - RAIL_X) < 6.0f) ok = false;

            for (int j = 0; j < NUM_HOUSES && ok; j++) {
                float dx = x - houses[j].x, dz = z - houses[j].z;
                if (sqrt(dx*dx+dz*dz) < 7.0f) ok = false;
            }
            if (ok && distToLoopRoad(x, z) < 3.0f) ok = false;

            for (int j = 0; j < i && ok; j++) {
                float dx = x - fruitTrees[j].x, dz = z - fruitTrees[j].z;
                if (sqrt(dx*dx+dz*dz) < 4.0f) ok = false;
            }
            attempts++;
        } while (!ok && attempts < 300);

        fruitTrees[i].x = x;
        fruitTrees[i].z = z;
        fruitTrees[i].rotY = ((float)rand()/RAND_MAX) * 360.0f;
        fruitTrees[i].type = i % NUM_FRUIT_TYPES;

        int t = fruitTrees[i].type;
        if (t == JAM || t == BOROI) fruitTrees[i].leanDeg = 12.0f + ((float)rand()/RAND_MAX) * 15.0f;
        else if (t == LEMON || t == GUAVA) fruitTrees[i].leanDeg = ((float)rand()/RAND_MAX) * 8.0f;
        else fruitTrees[i].leanDeg = ((float)rand()/RAND_MAX) * 4.0f;

        float la = ((float)rand()/RAND_MAX) * 2.0f * PI;
        fruitTrees[i].leanAxisX = sin(la);
        fruitTrees[i].leanAxisZ = cos(la);
        fruitTrees[i].swayPhase = ((float)rand()/RAND_MAX) * 2.0f * PI;

        int fc;
        switch (t) {
            case MANGO: fc = 5; break;
            case JACKFRUIT: fc = 3; break;
            case JAM: fc = 6; break;
            case LEMON: fc = 6; break;
            case GUAVA: fc = 5; break;
            case PAPAYA: fc = 4; break;
            default: fc = 4; break;
        }
        fruitTrees[i].fruitCount = fc;
        for (int k = 0; k < fc; k++) {
            float ang = (2.0f * PI * k) / fc + ((float)rand()/RAND_MAX) * 0.5f;
            float rad, hy;
            if (t == JACKFRUIT) { rad = 0.3f; hy = 2.0f + (k % 2) * 0.8f; }
            else if (t == PAPAYA) { rad = 0.25f; hy = 3.2f + (k % 2) * 0.4f; }
            else { rad = 0.9f + ((float)rand()/RAND_MAX) * 0.4f; hy = 0.3f + ((float)rand()/RAND_MAX) * 0.9f; }
            fruitTrees[i].fruitLocalX[k] = cos(ang) * rad;
            fruitTrees[i].fruitLocalZ[k] = sin(ang) * rad;
            fruitTrees[i].fruitLocalY[k] = hy;
            fruitTrees[i].fruitPicked[k] = false;
        }
    }
}

void initPedsVehicles() {
    for (int i = 0; i < NUM_PEDS; i++) {
        peds[i].t = i * (totalLoopLen / NUM_PEDS);
        peds[i].speed = 3.0f + (i % 3) * 0.4f;
        peds[i].laneOffset = ROAD_WIDTH / 2.0f + 1.2f;
        peds[i].variant = i;
    }
    for (int i = 0; i < NUM_VEHICLES; i++) {
        vehicles[i].t = i * (totalLoopLen / NUM_VEHICLES) + 20.0f;
        vehicles[i].speed = 9.0f + (i % 2) * 1.5f;
        vehicles[i].laneOffset = -(ROAD_WIDTH / 4.0f);
        vehicles[i].variant = i;
    }
}

void init() {
    glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos[] = {50.0f, 80.0f, 50.0f, 1.0f};
    GLfloat lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glutIgnoreKeyRepeat(1);

    initHouses();
    initFruitTrees();
    initPedsVehicles();
    initCampuses();
    initFish();
    initWaterfallFX();

    loadAllBannerTextures(); // load banner/sign images (falls back to text if missing)
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1100, 750);
    glutCreateWindow("3D Movable Village - Complete");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutTimerFunc((int)(TIMER_DT * 1000), timerUpdate, 0);
glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
