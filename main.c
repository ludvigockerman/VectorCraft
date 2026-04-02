#include <vectrex.h>
#include "LUT.h"
#include "controller.h"

#define bool int
#define true 1
#define false 0

typedef struct {
    int x;
    int y;
} vec2;

typedef struct {
    int x;
    int y;
    int z;
} vec3;

/*
bool tf[8][3] = {
    {false, false, false},
    {true,  false, false},
    {true,  true,  false},
    {false, true,  false},
    {false, false, true },
    {true,  false, true },
    {true,  true,  true },
    {false, true,  true }  
};*/

int world[3][3][3] = { // x, y, z
    { {1, 0, 0}, {1, 0, 0}, {1, 0, 0} },
    { {1, 0, 0}, {1, 1, 0}, {1, 0, 0} },
    { {1, 0, 0}, {1, 0, 0}, {1, 0, 0} }
};



vec3 playerposition;
vec2 playerrotation;
long decimalx = 0;
long decimalz = 0;
int speed = 2;
int sensitivity = 10;

int sinv;
int cosv;
int sinu;
int cosu;

// ---------------------------------------------------------
// Help functions
// ---------------------------------------------------------

int SearchThroughArray(vec2* list, int length, vec2 line) {
    for(int i = 0; i < length; i++) {
        if (list[i].x == line.x && list[i].y == line.y) {
            return 1;
        }
    }
    return 0;
}

void UpdateDirections(){
    // Rotation in x is called v and rotation in y is called u
    unsigned char anglev = (unsigned char)(playerrotation.x + 128);
    unsigned char angleu = (unsigned char)(playerrotation.y);
    
    sinv = sin_table[anglev];
    cosv = sin_table[(anglev + 64)];
    
    sinu = sin_table[angleu];
    cosu = sin_table[(angleu + 64)];
}

void project_point(vec3 p, vec2* out) {    
    long dz = (long)(p.z - playerposition.z);
    long dy = (long)(p.y - playerposition.y);
    long dx = (long)(p.x - playerposition.x);
    
    if (dx >= 127 || dy >= 127 || dz >= 127 || dx <= -127 || dy <= -127 || dz <= -127){
        out->x = -128;
        out->y = -128;
        return;
    }
    
    long r1z = (long)(dx * sinv + dz * cosv) >> 7; // Bit shifting (same as dividing by 128)
    long r1x = (long)(dx * cosv - dz * sinv) >> 7;

    long r2y = (long)(dy * cosu - r1z * sinu) >> 7;
    long r2z = (long)(dy * sinu + r1z * cosu) >> 7;
    
    if(r2z <= 0) {
        out->x = (int)-128;
        out->y = (int)-128;
        return;
    }

    if (r1x >= 127 || r2y >= 127 || r2z >= 127 || r1x <= -127 || r2y <= -127 || r2z <= -127){
        out->x = -128;
        out->y = -128;
        return;
    }
    
    //long fx = (r1x << 5) / r2z;
    //long fy = (r2y << 5) / r2z;

    long fx = (r1x) * recip_table[r2z] >> 3;
    long fy = (r2y) * recip_table[r2z] >> 3;
    
    if (fx > 127 || fx < -127 || fy > 127 || fy < -127) {
        out->x = -128;
        out->y = -128;
        return;
    }
    
    out->x = (int)fx;
    out->y = (int)fy;
}

// ---------------------------------------------------------
// Drawing functions
// ---------------------------------------------------------

void drawcube(vec3* cube, int edges[12][2]) {
    vec2 pts[8];
    for(int i = 0; i < 8; i++) {
        if (cube[i].x == -128) {
            pts[i] = (vec2){ -128, -128 };
        } else {
            project_point(cube[i], &pts[i]);
        }
    }

    int currentposx;
    int currentposy;
    int newposx;
    int newposy;
    int drawnlines = 0;

    //Reset0Ref();

    for(int i = 0; i < 12; i++) {
        if (edges[i][0] == -1) continue;
        vec2 p1 = pts[edges[i][0]];
        vec2 p2 = pts[edges[i][1]];
        
        if (p1.x == -128 || p2.x == -128) {
            continue;
        }
        
        if (drawnlines == 0) {
            currentposx = p1.x;
            currentposy = p1.y;
            Reset0Ref();
            Moveto_d(currentposy, currentposx);
        } else {
            newposx = p1.x;
            newposy = p1.y;
            Moveto_d(newposy - currentposy, newposx - currentposx);
            currentposx = newposx;
            currentposy = newposy;
        }
        
        int deltax = p2.x - p1.x;
        int deltay = p2.y - p1.y;
        Draw_Line_d(deltay, deltax);
        drawnlines++;
        
        currentposx += deltax;
        currentposy += deltay;
    }
}

void createcubeat(vec3 cubepos, int x, int y, int z) {
    vec3 out[8];
    out[0] = (vec3){ cubepos.x - 10, cubepos.y - 10, cubepos.z - 10 };
    out[1] = (vec3){ cubepos.x,      cubepos.y - 10, cubepos.z - 10 };
    out[2] = (vec3){ cubepos.x,      cubepos.y,      cubepos.z - 10 };
    out[3] = (vec3){ cubepos.x - 10, cubepos.y,      cubepos.z - 10 };
    out[4] = (vec3){ cubepos.x - 10, cubepos.y - 10, cubepos.z };
    out[5] = (vec3){ cubepos.x,      cubepos.y - 10, cubepos.z };
    out[6] = (vec3){ cubepos.x,      cubepos.y,      cubepos.z };
    out[7] = (vec3){ cubepos.x - 10, cubepos.y,      cubepos.z };

    /*for (int i = 0; i < 8; i++) {
        bool xb = true;
        bool yb = true;
        bool zb = true;
        
        if (playerposition.x >= out[i].x) xb = false;
        if (playerposition.y >= out[i].y) yb = false;
        if (playerposition.z >= out[i].z) zb = false;
        
        if (xb == tf[i][0] && yb == tf[i][1] && zb == tf[i][2]) {
            out[i] = (vec3){ -128, -128, -128 };
        }
    }*/

    // True means blocked or not visible
    bool upBlock   = ( (y < 2) && world[x][y+1][z] );
    bool downBlock = ( (y > 0) && world[x][y-1][z] );
    
    bool leftBlock  = ( (x > 0) && world[x-1][y][z] );
    bool rightBlock = ( (x < 2) && world[x+1][y][z] );
    
    bool frontBlock = ( (z < 2) && world[x][y][z+1] );
    bool backBlock  = ( (z > 0) && world[x][y][z-1] );


    bool upInvis   = (playerposition.y <= cubepos.y);
    bool downInvis = (playerposition.y >= cubepos.y - 10);
    
    bool leftInvis  = (playerposition.x >= cubepos.x - 10);
    bool rightInvis = (playerposition.x <= cubepos.x);
    
    bool frontInvis = (playerposition.z <= cubepos.z);
    bool backInvis  = (playerposition.z >= cubepos.z - 10);


    bool up   = (upBlock || upInvis);
    bool down = (downBlock || downInvis);
    
    bool left = (leftBlock || leftInvis);
    bool right = (rightBlock || rightInvis);
    
    bool front = (frontBlock || frontInvis);
    bool back = (backBlock || backInvis);
    
    //for (int i = 0; i < 8; i++) {
        //if (out[i].x == -128) continue;
        
    if (back && down && left) out[0] = (vec3){ -128, -128, -128 };
    
    if (back && down && right) out[1] = (vec3){ -128, -128, -128 };
    
    if (back && up && right) out[2] = (vec3){ -128, -128, -128 };
    
    if (back && up && left) out[3] = (vec3){ -128, -128, -128 };
    
    if (front && down && left) out[4] = (vec3){ -128, -128, -128 };

    if (front && down && right) out[5] = (vec3){ -128, -128, -128 };
    
    if (front && up && right) out[6] = (vec3){ -128, -128, -128 };
    
    if (front && up && left) out[7] = (vec3){ -128, -128, -128 };
    

    int edges[12][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
    };
    
   
    if(backBlock){
        edges[0][0] = -1; edges[0][1] = -1;
        edges[1][0] = -1; edges[1][1] = -1;
        edges[2][0] = -1; edges[2][1] = -1;
        edges[3][0] = -1; edges[3][1] = -1;
    }
    
    if(frontBlock){
        edges[4][0] = -1; edges[4][1] = -1;
        edges[5][0] = -1; edges[5][1] = -1;
        edges[6][0] = -1; edges[6][1] = -1;
        edges[7][0] = -1; edges[7][1] = -1;
    }

    /*if( downBlock ){
        edges[0][0] = -1; edges[0][1] = -1;
        edges[4][0] = -1; edges[4][1] = -1;
        edges[8][0] = -1; edges[8][1] = -1;
        edges[9][0] = -1; edges[9][1] = -1;
    }*/

    drawcube(&out[0], edges);
}

// ---------------------------------------------------------
// Main
// ---------------------------------------------------------

int main(void) {
    playerposition = (vec3){ 0, 0, 0 };
    playerrotation = (vec2){ 0, 0 };
    UpdateDirections();

    while(1) {
        Wait_Recal();
        Intensity_a(0x5f);
        
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                for (int z = 0; z < 3; z++) {
                    if (world[x][y][z] == 1) {
                        vec3 pos;
                        pos.x = x * 10;
                        pos.y = y * 10;
                        pos.z = (z * 10) + 30;
                        
                        createcubeat(pos, x, y, z);
                    }
                }
            }
        }

        //createcubeat((vec3){-10, 0, 30});
        //createcubeat((vec3){-10, -10, 30});
        //createcubeat((vec3){-10, 10, 30});
        //createcubeat((vec3){-10, 10, 40});
        //createcubeat((vec3){-10, 10, 20});

        Read_Btns();
        check_joysticks();

        if (joystick_1_x() > 0) { playerrotation.x += sensitivity; UpdateDirections(); }
        if (joystick_1_x() < 0) { playerrotation.x -= sensitivity; UpdateDirections(); }
        if (joystick_1_y() > 0 && playerrotation.y < 64) { playerrotation.y += sensitivity; UpdateDirections(); }
        if (joystick_1_y() < 0 && playerrotation.y > -64) { playerrotation.y -= sensitivity; UpdateDirections(); }

        if (Vec_Btn_State & 0b00000001) {
            long move_angle = (playerrotation.x + 128) & 0xff;
            int s = sin_table[move_angle];
            int c = sin_table[(move_angle + 64) & 0xff];
            
            decimalx += ((long)speed * s);
            decimalz += ((long)speed * c);
            
            while (decimalx > 127)  { decimalx -= 127; playerposition.x += 1; }
            while (decimalz > 127)  { decimalz -= 127; playerposition.z += 1; }
            while (decimalx < -127) { decimalx += 127; playerposition.x -= 1; }
            while (decimalz < -127) { decimalz += 127; playerposition.z -= 1; }
            
        }

        if (Vec_Btn_State & 0b00000010) 
        {
            long move_angle = (playerrotation.x + 128) & 0xff;
            int s = sin_table[move_angle];
            int c = sin_table[(move_angle + 64) & 0xff];
            
            decimalx -= ((long)speed * s);
            decimalz -= ((long)speed * c);
            
            while (decimalx > 127)  { decimalx -= 127; playerposition.x += 1; }
            while (decimalz > 127)  { decimalz -= 127; playerposition.z += 1; }
            while (decimalx < -127) { decimalx += 127; playerposition.x -= 1; }
            while (decimalz < -127) { decimalz += 127; playerposition.z -= 1; }
        }
        if (Vec_Btn_State & 0b00000100) { /* Button 3 */ }
        if (Vec_Btn_State & 0b00001000) { /* Button 4 */ }
    }
    
    return 0;
}