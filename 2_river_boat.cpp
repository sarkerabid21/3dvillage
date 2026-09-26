// ================================================================
// PART 2: NODI (River) + NOUKA (Boat) System (মাছ সহ)
// ----------------------------------------------------------------
// main.cpp থেকে দরকার:
//   - const float PI
//   - float flagTime, TIMER_DT
//   - Player struct + extern Player player;
//   - void drawHumanoid(...)
//   - a_down, d_down, w_down, s_down (keyboard state)
//   - const float playerTurnSpeed  (boat turning এ ব্যবহার হচ্ছে না,
//     boat নিজের boatTurnSpeed ব্যবহার করে)
// ================================================================

// ---------------------- river System ----------------------
const float RIVER_CENTER_Z   = -110.0f;
const float RIVER_HALF_DEPTH = 34.0f;
const float RIVER_HALF_WIDTH = 60.0f;

float waveOffset = 0.0f;

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

// ----------------------------------------------------------------
// keyboardDown() এর 'b'/'B' কেসে boat-এ ওঠা/নামার লজিক:
//
//   if (!inBoat) {
//       float dx = player.x - boatX, dz = player.z - boatZ;
//       if (sqrt(dx*dx + dz*dz) < 4.0f) {
//           inBoat = true;
//           player.x = boatX; player.z = boatZ; player.angle = boatAngle;
//       }
//   } else {
//       inBoat = false;
//       player.x = boatX;
//       player.z = RIVER_CENTER_Z + RIVER_HALF_DEPTH + 2.0f;
//   }
//
// timerUpdate() এ boat movement (inBoat true হলে):
//
//   if (a_down) boatAngle += playerTurnSpeed * TIMER_DT;
//   if (d_down) boatAngle -= playerTurnSpeed * TIMER_DT;
//   ... (boatSpeed দিয়ে move, নদীর সীমার মধ্যে clamp) ...
//
// display() এর ভেতরে কল:
//   drawAnimatedRiver(); drawAllFish(); drawBoat();
// init() এর ভেতরে: initFish();
// timerUpdate() এর ভেতরে: updateFish();
// ----------------------------------------------------------------
