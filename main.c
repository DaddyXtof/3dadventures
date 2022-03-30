#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#define RAYGUI_IMPLEMENTATION
#include "include/raylib.h"
#include "include/raygui.h"
#include "include/raymath.h"
#include "include/fastnoiselite.h"
#include "math.h"

#define FNL_IMPL

#define WIDTH  1600
#define HEIGHT 1200
#define MYDARK  (Color) {26, 18, 43, 255}
#define MYLIGHT (Color) {232,223,173,255}
#define MYMID1  (Color) {109,131,109,255}
#define gridSizeX 500
#define gridSizeY 500

static Mesh GenMyMesh(void);
float* noiseData;
float noiseMin,noiseMax;
bool interactive=true;
int sliderBarValue = 1;
int octaves = 1;
float persistance = 0.5f;
float lacunarity = 2.0f;

float* genNoise(int sizeX,int sizeY, int octaves, float persistance, float lacunarity) {
    float* noiseData = malloc(sizeX * sizeY *sizeof(float));
    noiseMin=FLT_MAX;
    noiseMax=FLT_MIN;
    fnl_state noise = fnlCreateState(601700);
    noise.noise_type = FNL_NOISE_PERLIN;
    int index = 0;
    //Define offset for each octave
    int offSetX[octaves];
    int offSetY[octaves];
    for (int i=0;i<octaves; i++) {
        offSetX[i]=GetRandomValue(-1000, 1000);
        offSetY[i]=GetRandomValue(-1000, 1000);
    }




    for (int y=0;y<sizeY;y++) {
        for (int x=0; x<sizeX;x++) {
            float amplitude = 1.0f;
            float frequency = 1.0f;
            for (int o=0;o<octaves;o++) {
                noiseData[index] = fnlGetNoise2D(&noise, x*frequency+offSetX[o], y*frequency+offSetY[o]);
                noiseData[index] = noiseData[index] + noiseData[index]*amplitude;

                amplitude *= persistance;
                frequency *= lacunarity;

                if (noiseData[index]>noiseMax) noiseMax=noiseData[index];
                if (noiseData[index]<noiseMin) noiseMin=noiseData[index];
            }
        index++;
        }
    }
    TraceLog(LOG_INFO,TextFormat("Noise Min/Max: %f/%f",noiseMin,noiseMax));
    return noiseData;
}

Image genImageFromNoise(float* noiseData) {
    Image gTexture;
    gTexture = GenImageColor(gridSizeY,gridSizeY,RAYWHITE);
    Color color;
    for (int y=0;y<gridSizeY;y++) {
        for (int x=0; x<gridSizeX;x++) {
            color = (Color){Remap(noiseData[x+y*gridSizeX],noiseMin,noiseMax,0,255),125,125,255};
            ImageDrawPixel(&gTexture,x,y,color);
        }
    }
    return gTexture;
}

int main (void) {
    Model model;
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    noiseData=genNoise(gridSizeX,gridSizeX, octaves, persistance, lacunarity);
    Image groundTexture = genImageFromNoise(noiseData);
    InitWindow(WIDTH, HEIGHT, "3D Adventures 1");
    SetTargetFPS(120);
    ExportImage(groundTexture,"test1.png");
    model = LoadModelFromMesh(GenMyMesh());
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(groundTexture);
    Camera3D camera = {0};
    camera.position = (Vector3){0.0f, 400.0f, 400.0f};
    camera.target   = (Vector3){0.0f, 0.0f,  0.0f};
    camera.up       = (Vector3){0.0f, 1.0f,  0.0f};
    camera.fovy     = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    Ray xAxis = {.direction=(Vector3){1.0f,0.0f,0.0f},.position=(Vector3){0.0f,0.0f,0.0f}};
    Ray yAxis = {.direction=(Vector3){0.0f,1.0f,0.0f},.position=(Vector3){0.0f,0.0f,0.0f}};
    Ray zAxis = {.direction=(Vector3){0.0f,0.0f,1.0f},.position=(Vector3){0.0f,0.0f,0.0f}};
    SetCameraMode(camera, CAMERA_FIRST_PERSON);

    while (!WindowShouldClose()) {
//        model = LoadModelFromMesh(GenMyMesh());
        UpdateCamera(&camera);
        if (IsKeyDown('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
        if (IsKeyPressed(KEY_SPACE)) {
            if (!interactive) {
                SetCameraMode(camera, CAMERA_FIRST_PERSON);
            }
            else {
                SetCameraMode(camera, CAMERA_CUSTOM);
            }
            interactive=!interactive;
        }
        BeginDrawing();
            ClearBackground(MYDARK);
            BeginMode3D(camera);
                //DrawCube(Position, 2.0f, 2.0f, 2.0f, MYMID1);
                //DrawCubeWires(Position, 2.0f, 2.0f, 2.0f, MYLIGHT);
                DrawGrid(gridSizeX, 1.0f);
                DrawRay(xAxis,RED);
                DrawRay(yAxis,BLUE);
                DrawRay(zAxis,YELLOW);
                DrawModel(model,position,1.0f,MYLIGHT);
            EndMode3D();
            octaves     = GuiSliderBar((Rectangle){ 30, 20, 200, 40 }, "Oct", TextFormat("%i", (int)octaves), octaves, 1, 8);
            persistance = GuiSliderBar((Rectangle){ 30, 80, 200, 40 }, "Per", TextFormat("%f", (float)persistance), persistance, 0, 4);
            lacunarity  = GuiSliderBar((Rectangle){ 30, 140,200, 40 }, "Lac", TextFormat("%f", (float)lacunarity), lacunarity, 0, 4);
            GuiSetState(GUI_STATE_NORMAL);
            if (GuiButton((Rectangle){ 20, 200, 200, 40 }, "Refresh")) {
                UnloadModel(model);
                noiseData=genNoise(gridSizeX,gridSizeX,octaves,persistance,lacunarity);
                groundTexture = genImageFromNoise(noiseData);
                model = LoadModelFromMesh(GenMyMesh());
                model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(groundTexture);
            }
        EndDrawing();
    }
    UnloadModel(model);
    UnloadImage(groundTexture);
    CloseWindow();
    free(noiseData);
    return 0;
}
float yFunction(int x, int z){
//    static float i=0.0f;
//    i+=0.001f;
//    return sin(0.5*x*i);
//    printf("NoiseData:%f",noiseData[(x+gridSizeX/2)+(z+gridSizeY/
//    )*gridSizeX]);
    return noiseData[(x+gridSizeX/2)+(z+gridSizeY/2)*gridSizeX]*20;
}
static Mesh GenMyMesh(void){
    Mesh mesh = {0};
    mesh.triangleCount=gridSizeY*gridSizeY*2;
    mesh.vertexCount = mesh.triangleCount*3;
    mesh.vertices = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords= (float *)MemAlloc(mesh.vertexCount*2*sizeof(float));
    mesh.normals  = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));
    int i = 0;
    int j = 0;
    float txX,txY,txX1,txY1;
    for (int z=-gridSizeY/2;z<(gridSizeY/2)-1;z++) {
        for (int x=-gridSizeX/2;x<(gridSizeX/2)-1;x++) {
            txX = (x + (float)gridSizeX/2)/gridSizeX;
            txY = (z + (float)gridSizeY/2)/gridSizeY;
            txX1 = (x + 1 + (float)gridSizeX/2)/gridSizeX;
            txY1 = (z + 1 + (float)gridSizeY/2)/gridSizeY;

            mesh.vertices[i]    = x;
            mesh.vertices[i+1]  = yFunction(x,z);
            mesh.vertices[i+2]  = z;
            mesh.texcoords[j]   = txX;
            mesh.texcoords[j+1] = txY;

            mesh.vertices[i+3]  = x;
            mesh.vertices[i+4]  = yFunction(x,z+1);
            mesh.vertices[i+5]  = z+1;
            mesh.texcoords[j+2] = txX;
            mesh.texcoords[j+3] = txY1;

            mesh.vertices[i+6]  = x+1;
            mesh.vertices[i+7]  = yFunction(x+1,z+1);
            mesh.vertices[i+8]  = z+1;
            mesh.texcoords[j+4] = txX1; 
            mesh.texcoords[j+5] = txY1; 

            mesh.vertices[i+9]  = x+1;
            mesh.vertices[i+10] = yFunction(x+1,z+1); 
            mesh.vertices[i+11] = z+1;
            mesh.texcoords[j+6] = txX1; 
            mesh.texcoords[j+7] = txY1; 

            mesh.vertices[i+12] = x+1;
            mesh.vertices[i+13] = yFunction(x+1,z);
            mesh.vertices[i+14] = z;
            mesh.texcoords[j+8] = txX1; 
            mesh.texcoords[j+9] = txY; 

            mesh.vertices[i+15]  = x;
            mesh.vertices[i+16]  = yFunction(x,z);
            mesh.vertices[i+17]  = z;
            mesh.texcoords[j+10] = txX; 
            mesh.texcoords[j+11] = txY; 

            i+=18;
            j+=12;
        }
    }
    UploadMesh(&mesh, false);
    return mesh;
}
