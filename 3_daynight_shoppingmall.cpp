// ================================================================
// PART 3: DIN-RAT (Day/Night) System + SHOPPING MALL System
// ----------------------------------------------------------------
// main.cpp থেকে দরকার:
//   - const float PI
//   - void drawBox(float w, float h, float d)
//   - void drawDoor(float doorW, float doorH, float halfD, float doorAngle)
//   - void drawText3D(float x, float y, float z, const char* text)
//   - GLuint texShopBanner; int texShopBannerW, texShopBannerH;
//   - void drawTexturedQuadFit(GLuint, int, int, float, float)
//   - Player struct + extern Player player;
// ================================================================

// ---------------------- Day/Night System ----------------------
bool isNight = false;
float dayNightBlend = 0.0f;
const float DAYNIGHT_SPEED = 0.25f;

// update() ফাংশনে প্রতি frame এ smooth transition করা হয়:
//
//   float target = isNight ? 1.0f : 0.0f;
//   if (dayNightBlend < target) {
//       dayNightBlend += DAYNIGHT_SPEED * 0.016f;
//       if (dayNightBlend > target) dayNightBlend = target;
//   } else if (dayNightBlend > target) {
//       dayNightBlend -= DAYNIGHT_SPEED * 0.016f;
//       if (dayNightBlend < target) dayNightBlend = target;
//   }
//
// keyboardDown() এ টগল করার জন্য:
//   case 'n': case 'N': isNight = !isNight; break;
//
// display() এ dayNightBlend ব্যবহার করে sky color এবং light বদলানো হয়:
//
//   float dayR=0.55f, dayG=0.75f, dayB=0.95f;
//   float nightR=0.02f, nightG=0.03f, nightB=0.09f;
//   float skyR = dayR + (nightR - dayR) * dayNightBlend;
//   float skyG = dayG + (nightG - dayG) * dayNightBlend;
//   float skyB = dayB + (nightB - dayB) * dayNightBlend;
//   glClearColor(skyR, skyG, skyB, 1.0f);
//
//   float lightIntensity = 1.0f - dayNightBlend * 0.75f;
//   GLfloat lightColor[] = { lightIntensity, lightIntensity, lightIntensity * 1.05f, 1.0f };
//   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
//
//   GLfloat ambientColor[] = { 0.15f + 0.05f*(1.0f-dayNightBlend), 0.15f, 0.2f, 1.0f };
//   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

// ---------------------- Shopping Mall System (Simple) ----------------------
#define NUM_SHOPS 2
float shopX[NUM_SHOPS]         = { 10.0f, 80.0f };
float shopZ[NUM_SHOPS]         = {  80.0f,  16.0f };
float shopRotY[NUM_SHOPS]      = { 180.0f, -90.0f };
bool  shopDoorOpen[NUM_SHOPS]  = { false, false };
float shopDoorAngle[NUM_SHOPS] = { 0.0f, 0.0f };

const float SHOP_W = 16.0f, SHOP_D = 12.0f, SHOP_H = 4.5f;
const float SHOP_DOOR_W = 2.4f, SHOP_DOOR_H = 2.8f;

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

// ---------------------- Shop banner (optional texture image সহ) ----------------------
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

// ----------------------------------------------------------------
// keyboardDown() এর 'e'/'E' কেসে shop door টগল (nearest shop খুঁজে):
//
//   int sIdx = findNearestShop(player.x, player.z);
//   if (sIdx >= 0) shopDoorOpen[sIdx] = !shopDoorOpen[sIdx];
//
// timerUpdate() এ shop door animation:
//
//   for (int i = 0; i < NUM_SHOPS; i++) {
//       float target = shopDoorOpen[i] ? -100.0f : 0.0f;
//       if (shopDoorAngle[i] < target) { shopDoorAngle[i] += 4.0f; if (shopDoorAngle[i] > target) shopDoorAngle[i] = target; }
//       else if (shopDoorAngle[i] > target) { shopDoorAngle[i] -= 4.0f; if (shopDoorAngle[i] < target) shopDoorAngle[i] = target; }
//   }
//
// timerUpdate() এ movement collision চেক করার সময়:
//   !checkShopCollision(nx, nz)
//
// display() এ কল: drawAllShops();
// ----------------------------------------------------------------
