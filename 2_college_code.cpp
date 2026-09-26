// =====================================================================
// 2) COLLEGE (SCHOOL) CODE
// -----------------------------------------------------------------
// "ORIANA SCHOOL & COLLEGE" is campuses[0] (kind == CAMPUS_SCHOOL)
// in the array below.
//
// IMPORTANT: College/School, University, and Hospital all share the
// SAME generic "Campus" system functions in the original source —
// there is no separate function written only for the college.
// =====================================================================

// =====================================================================
// DEPENDENCIES this module needs from your main project (must already
// exist elsewhere in your .cpp, e.g. main.cpp):
//   drawBox(w,h,d), drawText3D(x,y,z,text), drawBench(len),
//   drawHumanoid(x,z,angle,r,g,b,isGuard), drawSecurityGuard(x,z,angle,b),
//   drawRoadSegment(x1,z1,x2,z2,width), drawFruitTree(FruitTree),
//   struct FruitTree { ... } with fields used below,
//   float flagTime, const float PI, const int MAX_FRUITS,
//   const int NUM_FRUIT_TYPES, const float LOOP_R,
//   GLuint texSchoolBanner/texUniversityBanner/texHospitalBanner (+ W/H),
//   drawTexturedQuadFit(...), const float TIMER_DT
// =====================================================================

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
            bool doorGap = (pz < c.z-hd+margin &&
                            fabs(px-c.x) < CAMPUS_GATE_W*0.5f+0.35f &&
                            campusFrontGateOpen(i));
            if (!inside && !doorGap) return true;
        }

        bool insideBoundary = fabs(px-c.x) < bx && fabs(pz-c.z) < bz;
        if (insideBoundary && !pointInsideCampusBuilding(px,pz,i)) {
            bool gateGap = (pz < c.z-bz+margin &&
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
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
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
    glPushMatrix(); glTranslatef(c.x,y+wallH*0.5f,c.z+hd); drawBox(c.w,wallH,0.28f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x-hw,y+wallH*0.5f,c.z); drawBox(0.28f,wallH,c.d); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x+hw,y+wallH*0.5f,c.z); drawBox(0.28f,wallH,c.d); glPopMatrix();

    float sideW=(c.w-CAMPUS_GATE_W)/2.0f;
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
    glPushMatrix(); glTranslatef(c.x,h*0.5f,c.z+bz); drawBox(c.w+10.0f,h,0.22f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x-bx,h*0.5f,c.z); drawBox(0.22f,h,c.d+10.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x+bx,h*0.5f,c.z); drawBox(0.22f,h,c.d+10.0f); glPopMatrix();

    float sideW=(c.w+10.0f-CAMPUS_GATE_W)/2.0f;
    glPushMatrix(); glTranslatef(c.x-(CAMPUS_GATE_W*0.5f+sideW*0.5f),h*0.5f,c.z-bz); drawBox(sideW,h,0.22f); glPopMatrix();
    glPushMatrix(); glTranslatef(c.x+(CAMPUS_GATE_W*0.5f+sideW*0.5f),h*0.5f,c.z-bz); drawBox(sideW,h,0.22f); glPopMatrix();

    glPushMatrix();
    glTranslatef(c.x-CAMPUS_GATE_W*0.5f,c.z*0.0f+0.0f,c.z-bz);
    glRotatef(c.gateAngle,0.0f,1.0f,0.0f);
    glTranslatef(CAMPUS_GATE_W*0.5f, h*0.5f, 0.0f);
    glColor3f(0.10f,0.12f,0.14f);
    drawBox(CAMPUS_GATE_W,h,0.18f);
    glPopMatrix();

    glColor3f(0.75f,0.65f,0.18f);
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
    // NOTE: original source was truncated here (cut off mid-loop),
    // so this init function may need a closing brace/continuation
    // depending on what else your project needs to initialize.
}
