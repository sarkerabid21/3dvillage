// ================================================================
// FISH JUMP SYSTEM (নদীর মাছ লাফানোর কোড, আলাদা করে দেওয়া হলো)
// ----------------------------------------------------------------
// main.cpp থেকে দরকার:
//   - float TIMER_DT;
//   - const float RIVER_CENTER_Z, RIVER_HALF_DEPTH, RIVER_HALF_WIDTH;
//   - <cstdlib> এর rand()/RAND_MAX (stdlib.h ইতিমধ্যে include করা)
// ================================================================

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

    // লাফের শুরু/শেষে পানির ছোট ছিটা (splash)
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

// ----------------------------------------------------------------
// কল করার জায়গা:
//   init()        -> initFish();
//   timerUpdate()  -> updateFish();
//   display()      -> drawAllFish();
// ----------------------------------------------------------------
