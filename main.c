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
#define gridSizeX 200
#define gridSizeY 200

static Mesh GenMyMesh(void);
float* noiseData;
Model model;
float noiseMin,noiseMax;
bool interactive=true;
bool twoD=false;
int sliderBarValue = 1;
int octaves        = 4;
float persistance  = 0.5f;
float lacunarity   = 2.0f;
float scale        = 1.0f;
int deepBlue       = 10;
int lightBlue      = 50;
int sandYellow     = 60;
int lightGreen     = 100;
int darkGreen      = 150;
int lightRock      = 200;
int darkRock       = 250;
int whiteSnow      = 255;
Texture groundTexture;

float* genNoise(int sizeX,int sizeY, float scale, int octaves, float persistance, float lacunarity) {
    float* noiseData = malloc(sizeX * sizeY *sizeof(float));
    if (scale==0) scale=0.001;
    noiseMin=FLT_MAX;
    noiseMax=FLT_MIN;
    fnl_state noise = fnlCreateState(601700);
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.octaves = 1;
    noise.lacunarity = 1;
    int index = 0;
    //Define offset for each octave
    int offSetX[octaves];
    int offSetY[octaves];
    for (int i=0;i<octaves; i++) {
        offSetX[i]=GetRandomValue(-100, 100);
        offSetY[i]=GetRandomValue(-100000, 100000);
    }

    for (int y=0;y<sizeY;y++) {
        for (int x=0; x<sizeX;x++) {
            float amplitude = 1.0f;
            float frequency = 1.0f;
            for (int o=0;o<octaves;o++) {
                noiseData[index] = fnlGetNoise2D(&noise, (x/scale)*frequency+offSetX[o], (y/scale)*frequency+offSetY[o]);
                noiseData[index] = noiseData[index] + noiseData[index]*amplitude;

                amplitude *= persistance;
                frequency *= lacunarity;

                if (noiseData[index]>noiseMax) noiseMax=noiseData[index];
                if (noiseData[index]<noiseMin) noiseMin=noiseData[index];
            }
        index++;
        }
    }

    //Normalise NoiseData os that it sites between 0 and 1:
    index=0;
    for (int y=0;y<sizeY;y++) {
        for (int x=0;x<sizeX;x++) {
            noiseData[index]=Remap(noiseData[x+y*gridSizeX],noiseMin,noiseMax,0,1);
            index++;
        }
    }
    TraceLog(LOG_INFO,TextFormat("Noise Min/Max: %f/%f",noiseMin,noiseMax));
    return noiseData;
}

Texture genTextureFromNoise(float* noiseData) {
    Image gTexture;
    Color color;
    int noiseRemaped=0;
    gTexture = GenImageColor(gridSizeY,gridSizeY,RAYWHITE);
    for (int y=0;y<gridSizeY;y++) {
        for (int x=0; x<gridSizeX;x++) {
            noiseRemaped = Remap(noiseData[x+y*gridSizeX],0,1,0,255);
            if (noiseRemaped  <  deepBlue)                                 color=(Color){51 ,99 ,192,255}; //deepBlue
            if ((noiseRemaped >= deepBlue) && (noiseRemaped<lightBlue))    color=(Color){51 ,99 ,255,255}; //lightBlue
            if ((noiseRemaped >= lightBlue) && (noiseRemaped<sandYellow))  color=(Color){210,208,123,255}; //sandYellow
            if ((noiseRemaped >= sandYellow) && (noiseRemaped<lightGreen)) color=(Color){87 ,152,27 ,255}; //lightGreen
            if ((noiseRemaped >= lightGreen) && (noiseRemaped<darkGreen))  color=(Color){62 ,107,17 ,255}; //darkGreen
            if ((noiseRemaped >= darkGreen) && (noiseRemaped<lightRock))   color=(Color){91 ,69 ,61 ,255}; //lightRock
            if ((noiseRemaped >= lightRock) && (noiseRemaped<darkRock))    color=(Color){75 ,60 ,54 ,255}; //darkRock
            if (noiseRemaped >= darkRock)                                  color=(Color){255,255,255,255}; //whiteSnow
            //color = (Color){noiseRemaped,125,125,255};
            ImageDrawPixel(&gTexture,x,y,color);
        }
    }
    return LoadTextureFromImage(gTexture);
}

void refreshMap() {
    UnloadModel(model);
    noiseData=genNoise(gridSizeX,gridSizeX,scale,octaves,persistance,lacunarity);
    groundTexture = genTextureFromNoise(noiseData);
    model = LoadModelFromMesh(GenMyMesh());
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = groundTexture;
}
void repaintMap() {
    UnloadModel(model);
    groundTexture = genTextureFromNoise(noiseData);
    model = LoadModelFromMesh(GenMyMesh());
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = groundTexture;
}

int main (void) {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    noiseData=genNoise(gridSizeX,gridSizeX, scale, octaves, persistance, lacunarity);
    InitWindow(WIDTH, HEIGHT, "3D Adventures 1");
    SetTargetFPS(119);
    groundTexture = genTextureFromNoise(noiseData);
    ExportImage(LoadImageFromTexture(groundTexture),"test1.png");
    model = LoadModelFromMesh(GenMyMesh());
    GuiSetStyle(DEFAULT,TEXT_SIZE,20);
    GuiSetStyle(DEFAULT,TEXT_COLOR_NORMAL,ColorToInt(RAYWHITE));
    GuiSetStyle(BUTTON ,TEXT_COLOR_NORMAL,ColorToInt(BLACK));
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = groundTexture;
    Camera3D camera = {0};
    camera.position = (Vector3){0.0f, 30.0f, 200.0f};
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
                DrawGrid(gridSizeX, 10.0f);
                DrawRay(xAxis,RED);
                DrawRay(yAxis,BLUE);
                DrawRay(zAxis,YELLOW);
                DrawModel(model,position,1.0f,MYLIGHT);
            EndMode3D();
            DrawRectangle(0,0,400,HEIGHT,MYMID1);
            int noctaves = GuiSliderBar((Rectangle){ 50, 20, 200, 40 }, "Oct", TextFormat("%i", (int)octaves), octaves, 1, 8);
            if (noctaves!=octaves) {
                octaves=noctaves;
                refreshMap();
            }
            float npersistance = GuiSliderBar((Rectangle){ 50, 80, 200, 40 }, "Per", TextFormat("%f", (float)persistance), persistance, 0, 4);
            if (npersistance != persistance) {
                persistance = npersistance;
                refreshMap();
            }
            float nlacunarity  = GuiSliderBar((Rectangle){ 50, 140,200, 40 }, "Lac", TextFormat("%f", (float)lacunarity), lacunarity, 0, 4);
            if (nlacunarity!=lacunarity) {
                lacunarity=nlacunarity;
                refreshMap();
            }
            if (GuiSliderBar((Rectangle){ 50, 200,200, 40 }, "Sca", TextFormat("%f", (float)scale), scale, 0, 50) != scale) {
                scale = GuiSliderBar((Rectangle){ 50, 200,200, 40 }, "Sca", TextFormat("%f", (float)scale), scale, 0, 50);
                refreshMap();
            }
            int ndeepBlue = GuiSliderBar((Rectangle){ 50, 380 ,200, 40 }, "dBl", TextFormat("%i", (int)deepBlue), deepBlue, 0, 255);
            if (ndeepBlue != deepBlue) {
                deepBlue = ndeepBlue;
                repaintMap();
            }
            int nlightBlue = GuiSliderBar((Rectangle){ 50, 440 ,200, 40 }, "lBl", TextFormat("%i", (int)lightBlue), lightBlue, 0, 255);
            if (nlightBlue != lightBlue) {
                lightBlue = nlightBlue;
                repaintMap();
            }
            int nsandYellow = GuiSliderBar((Rectangle){ 50, 500 ,200, 40 }, "sYe", TextFormat("%i", (int)sandYellow), sandYellow, 0, 255);
            if (nsandYellow != sandYellow) {
                sandYellow = nsandYellow;
                repaintMap();
            }
            int nlightGreen = GuiSliderBar((Rectangle){ 50, 560 ,200, 40 }, "lGr", TextFormat("%i", (int)lightGreen), lightGreen, 0, 255);
            if (nlightGreen != lightGreen) {
                lightGreen = nlightGreen;
                repaintMap();
            }
            int ndarkGreen = GuiSliderBar((Rectangle){ 50, 620 ,200, 40 }, "dGr", TextFormat("%i", (int)darkGreen), darkGreen, 0, 255);
            if (ndarkGreen != darkGreen) {
                darkGreen = ndarkGreen;
                repaintMap();
            }
            int nlightRock = GuiSliderBar((Rectangle){ 50, 680 ,200, 40 }, "lRk", TextFormat("%i", (int)lightRock), lightRock, 0, 255);
            if (nlightRock != lightRock) {
                lightRock = nlightRock;
                repaintMap();
            }
            int ndarkRock = GuiSliderBar((Rectangle){ 50, 740 ,200, 40 }, "dRk", TextFormat("%i", (int)darkRock), darkRock, 0, 255);
            if (ndarkRock != darkRock) {
                darkRock = ndarkRock;
                repaintMap();
            }
            GuiSetState(GUI_STATE_NORMAL);
            if (GuiButton((Rectangle){ 50, 260, 200, 40 }, "Refresh")) refreshMap();
            if (GuiButton((Rectangle){ 50, 320, 200, 40 }, "2d/3d")|| IsKeyPressed('2')) {
                twoD=!twoD;
            }
            if (twoD) {
                DrawTextureEx(groundTexture,(Vector2){WIDTH-gridSizeX*5,HEIGHT-gridSizeY*5},0,5.0f,RAYWHITE);
            }
        EndDrawing();
    }
    UnloadModel(model);
    UnloadTexture(groundTexture);
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
