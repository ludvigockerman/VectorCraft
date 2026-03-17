#include <vectrex.h>
#include "LUT.h"

typedef struct { int x,y,z; } Vec3;
typedef struct { int x,y; } Vec2;

int sensitivity = 10;

//Vec3 blockPositions[30]; // 30 is the maximum amount of cubes that can be rendered at the same time.

int edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

Vec3 playerPosition;
Vec2 playerRotation;

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
    //Wait_Recal();
    Reset0Ref();
    Moveto_d(p.y, p.x);
}

void DrawCube(Vec3 *cube)
{
    Vec2 pts[8];
    for(int w=0;w<8;w++)
    {
        project_point(cube[w], &pts[w]);
    }
    
    int currentPosx;
    int currentPosy;
    int newPosx;
    int newPosy;
    int drawnLines = 0;
    
    for(int i=0;i<12;i++)
    {
        if (pts[edges[i][0]].x == -128 || pts[edges[i][1]].x == -128){
            continue;
        }
        if (drawnLines==0 || drawnLines==6){
            currentPosx = pts[edges[i][0]].x;
            currentPosy = pts[edges[i][0]].y;
            MoveToScreenPosition((Vec2){currentPosx, currentPosy});
        }
        newPosx = pts[edges[i][0]].x;
        newPosy = pts[edges[i][0]].y;
        Moveto_d(newPosy - currentPosy, newPosx - currentPosx);
        currentPosx = newPosx;
        currentPosy = newPosy;
        
        int deltax = pts[edges[i][1]].x - pts[edges[i][0]].x;
        int deltay = pts[edges[i][1]].y - pts[edges[i][0]].y;
        Draw_Line_d(deltay, deltax);
        drawnLines++;
        
        currentPosx += deltax;
        currentPosy += deltay;
    }
}

void CreateCubeAt(Vec3 cubePos, Vec3 *out)
{
    out[0] = (Vec3){cubePos.x-10,cubePos.y-10,cubePos.z-10};
    out[1] = (Vec3){cubePos.x,   cubePos.y-10,cubePos.z-10};
    out[2] = (Vec3){cubePos.x,   cubePos.y,   cubePos.z-10};
    out[3] = (Vec3){cubePos.x-10,cubePos.y,   cubePos.z-10};
    
    out[4] = (Vec3){cubePos.x-10,cubePos.y-10,cubePos.z};
    out[5] = (Vec3){cubePos.x,   cubePos.y-10,cubePos.z};
    out[6] = (Vec3){cubePos.x,   cubePos.y,   cubePos.z};
    out[7] = (Vec3){cubePos.x-10,cubePos.y,   cubePos.z};
}

int main(void)
{
    Vec3 cube[8]; // 8 points per cube
    
    playerPosition = (Vec3){ 0, 0, 0 };
    playerRotation = (Vec2){ 0, 0 };

    while(1)
    {        
        Wait_Recal();
        CreateCubeAt((Vec3){-10, 0, 30}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, -10, 30}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, 10, 30}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, 10, 40}, &cube[0]);
        DrawCube(&cube[0]);
        CreateCubeAt((Vec3){-10, 10, 20}, &cube[0]);
        DrawCube(&cube[0]);
        Intensity_a(0x5F);
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