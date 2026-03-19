#include <vectrex.h>
#include "LUT.h"

typedef struct { int x,y,z; } Vec3;
typedef struct { int x,y; } Vec2;

int faces[6][4] = {
    {0,1,2,3}, // far away
    {4,5,6,7}, // close
    {0,1,5,4}, // bottom
    {2,3,7,6}, // top
    {1,2,6,5}, // right
    {0,3,7,4} // left
};

const int edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, // Far away
    {4,5}, {5,6}, {6,7}, {7,4}, // Close
    {0,4}, {1,5}, {2,6}, {3,7}  // Lines connecting them
};

int sensitivity = 10;

//Vec3 blockPositions[30]; // 30 is the maximum amount of cubes that can be rendered at the same time.

Vec3 playerPosition;
Vec2 playerRotation;

int SearchThroughArray(Vec2 line, Vec2 *list, int length)
{
    for(int i = 0; i < length; i++)
    {
        if (list[i].x == line.x && list[i].y == line.y)
        {
            return 1;
        }
    }
    return 0;
}

void project_point(Vec3 p, Vec2 *out)
{
    long angle = (long)((playerRotation.x + 128) & 0xFF);
    
    int sinv = sin_table[angle]; // value from -127 to 127
    int cosv = sin_table[(angle + 64) & 0xFF]; // value from -127 to 127

    long dz = (long)(p.z - playerPosition.z);
    long dy = (long)(p.y - playerPosition.y);
    long dx = (long)(p.x - playerPosition.x);

    long rz = (long)(dx * sinv + dz * cosv) / 127;
    long rx = (long)(dx * cosv - dz * sinv) / 127;
    
    if(rz <= 0) 
    {
        out->x = (int)-128;
        out->y = (int)-128;
        return;
    }

    long fx = (rx * 50) / rz;
    long fy = (dy * 50) / rz;
    
    if (fx > 127 || fx < -127 || fy > 127 || fy < -127){
        out->x = -128;
        out->y = -128;
        return;
    }
    
    out->x = (int)fx;
    out->y = (int)fy;
}

void MoveToScreenPosition(Vec2 p)
{
    Reset0Ref();
    Moveto_d(p.y, p.x);
}

void DrawCube(Vec3 *cube)
{
    Vec2 pts[24];
    //Vec3 drawnlines[12];
    for(int i=0;i<24;i++)
    {
        if (cube[i].x == -128)
        {
            pts[i] = (Vec2){ -128, -128 };
        }
        else 
        {
            project_point(cube[i], &pts[i]);
        }
    }
    
    int currentPosx;
    int currentPosy;
    int newPosx;
    int newPosy;
    int drawnLines = 0;
    
    for(int i=0;i<12;i++)
    {
        Vec2 p1 = pts[edges[i][0]];
        Vec2 p2 = pts[edges[i][1]];
        
        if (p1.x == -128) {
            continue;
        }

        //Vec2 p2 = pts[faces[i][(u + 1) % 4]];


        //if{LinesDrawn[0] == 0)
        if (drawnLines == 0 || drawnLines == 6) {
            currentPosx = p1.x;
            currentPosy = p1.y;
            MoveToScreenPosition((Vec2) { currentPosx, currentPosy });
        }
        newPosx = p1.x;
        newPosy = p1.y;
        Moveto_d(newPosy - currentPosy, newPosx - currentPosx);
        currentPosx = newPosx;
        currentPosy = newPosy;

        int deltax = p2.x - p1.x;
        int deltay = p2.y - p1.y;
        Draw_Line_d(deltay, deltax);
        drawnLines++;

        currentPosx += deltax;
        currentPosy += deltay;
        
    }
}

void CreateCubeAt(Vec3 cubePos)
{
    Vec3 out[8];
    out[0] = (Vec3){ cubePos.x - 10,cubePos.y - 10,cubePos.z - 10 };
    out[1] = (Vec3){ cubePos.x,   cubePos.y - 10,cubePos.z - 10 };
    out[2] = (Vec3){ cubePos.x,   cubePos.y,   cubePos.z - 10 };
    out[3] = (Vec3){ cubePos.x - 10,cubePos.y,   cubePos.z - 10 };

    out[4] = (Vec3){ cubePos.x - 10,cubePos.y - 10,cubePos.z };
    out[5] = (Vec3){ cubePos.x,   cubePos.y - 10,cubePos.z };
    out[6] = (Vec3){ cubePos.x,   cubePos.y,   cubePos.z };
    out[7] = (Vec3){ cubePos.x - 10,cubePos.y,   cubePos.z };

    Vec3 output[24];

    //connections[12][2] = edges;

    // implement some sort of check if the point should be drawn. Else set position to -128
    for (int i = 0; i < 4; i++)
    {
        if (playerPosition.x >= cubePos.x)
        {
            output[i] = (Vec3){ -128, -128, -128 };
        }
        else {
            int index = faces[0][i];
            output[i] = out[index];
        }
    }

    for (int i = 4; i < 8; i++)
        if (playerPosition.x <= cubePos.x - 10)
        {
            output[i] = (Vec3){ -128, -128, -128 };
        }
        else {
            int index = faces[1][i - 4];
            output[i] = out[index];
        }

    for (int i = 8; i < 12; i++)
        if (playerPosition.y >= cubePos.y)
        {
            output[i] = (Vec3){ -128, -128, -128 };
        }
        else {
            int index = faces[2][i - 8];
            output[i] = out[index];
        }
    for (int i = 12; i < 16; i++)
        if (playerPosition.y <= cubePos.y - 10)
        {
            output[i] = (Vec3){ -128, -128, -128 };
        }
        else {
            int index = faces[3][i - 12];
            output[i] = out[index];
        }

    for (int i = 16; i < 20; i++)
        if (playerPosition.z >= cubePos.z)
        {
            output[i] = (Vec3){ -128, -128, -128 };
        }
        else {
            int index = faces[4][i - 16];
            output[i] = out[index];
        }
    for (int i = 20; i < 24; i++)
        if (playerPosition.z <= cubePos.z - 10)
        {
            output[i] = (Vec3){ -128, -128, -128 };
        }
        else {
            int index = faces[5][i - 20];
            output[i] = out[index];
        }


    for (int i = 0; i < 24; i++)
    {
        Vec3 value = output[i];
        if (value.x == -128) continue;
        for (int u = 0; u < 24; u++)
        {
            if (u == i) continue;
            if (output[u].x == value.x &&
                output[u].y == value.y &&
                output[u].z == value.z)
            {
                output[u] = (Vec3){ -128, -128, -128 };
            }
        }
    }
    DrawCube(&output[0]);
}

int main(void)
{
    //Vec3 cube[8]; // 8 points per cube
    
    playerPosition = (Vec3){ 0, 0, 0 };
    playerRotation = (Vec2){ 0, 0 };

    while(1)
    {
        Wait_Recal();
        Intensity_a(0x5F);
        CreateCubeAt((Vec3){-10, 0, 30});
        CreateCubeAt((Vec3){-10, -10, 30});
        //CreateCubeAt((Vec3){-10, 10, 30});
        //CreateCubeAt((Vec3){-10, 10, 40});
        //CreateCubeAt((Vec3){-10, 10, 20});
        
        //Joy_Digital();
        Read_Btns();
        if (Vec_Btn_State & 0b00000001){
            //Print_Str_d( 50, -50, "BUTTON1");
            playerPosition.z ++;
        }
        if (Vec_Btn_State & 0b00000010){
            playerPosition.z --;
        }
        if (Vec_Btn_State & 0b00000100){
            playerRotation.x += sensitivity;
        }
        if (Vec_Btn_State & 0b00001000){
            playerRotation.x -= sensitivity;
        }
    }
}