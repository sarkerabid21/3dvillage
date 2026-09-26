// ================================================================
// PART 1: PAHAR (Mountain) + JHORNA (Waterfall) System
// ----------------------------------------------------------------
// এই অংশটা main.cpp এর ভেতরেই থাকতে হবে, নিচের জিনিসগুলো main
// ফাইল থেকে already declare করা আছে বলে ধরে নেওয়া হয়েছে:
//   - const float PI
//   - float flagTime, TIMER_DT
//   - void drawBox(float w, float h, float d)
//   - GLUquadric, glut* functions (freeglut/OpenGL headers)
// ================================================================

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

float waterfallOffset = 0.0f;

void waterfallPathAt(float t, float &x, float &y, float &z, float &halfW) {
    y = WF_TOP_Y + (WF_BOTTOM_Y - WF_TOP_Y) * t;
    z = WF_TOP_Z + (WF_BOTTOM_Z - WF_TOP_Z) * t;

    float bend = sinf(t * 3.0f) * 1.4f + sinf(t * 7.3f + 1.7f) * 0.5f;
    x = WF_X + bend * (1.0f - t * 0.4f);

    halfW = WF_TOP_HALFW + (WF_BOTTOM_HALFW - WF_TOP_HALFW) * t;
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

// ----------------------------------------------------------------
// Call করতে হবে:
//   init()     এর ভেতরে -> initWaterfallFX();
//   timerUpdate() এর ভেতরে -> updateWaterfallFX();
//   display()  এর ভেতরে -> drawMountain(); drawWaterfall(); drawWaterfallParticlesAndMist();
// ----------------------------------------------------------------
