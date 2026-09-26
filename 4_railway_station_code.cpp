// =====================================================================
// 4) RAILWAY STATION CODE
// -----------------------------------------------------------------
// This one IS genuinely separate/standalone code (unlike files 1-3,
// which share one generic system). It covers: the track, the bridge
// over the river, the station building, and the moving train.
//
// DEPENDENCIES this module needs from your main project:
//   drawBox(w,h,d), drawText3D(x,y,z,text)
// =====================================================================

// ---------------------- river System (needed for bridge height calc) ----------------------
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

// ---------------------------------------------------------------
// Call this each frame/timer tick (e.g. inside your update() function)
// to move the train along the track and loop it back to the start:
//
//   if (trainRunning) {
//       trainDist += trainSpeed * TIMER_DT;
//       if (trainDist > RAIL_TOTAL_LEN + NUM_CARRIAGES * (CARRIAGE_LEN + CARRIAGE_GAP))
//           trainDist = 0.0f;
//   }
//
// And in your display() function, call in this order after the ground:
//   drawRailwayTrack();
//   drawRailwayStation();
//   drawTrain();
// ---------------------------------------------------------------
