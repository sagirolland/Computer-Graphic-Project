#if !defined(RENDERER_HEADER)
#define RENDERER_HEADER

#include "scene.h"
#include "math.h"

typedef enum {
    RENDERING_METHOD_SOFTWARE   = 0, // HW1-2
    RENDERING_METHOD_OPENGL     = 1, // HW3
    RENDERING_METHOD_RAYTRACING = 2, // HW4
    
    RENDERING_METHOD_COUNT,
} RenderingMethod;

// RenderSettings is pointed by the renderer and not stored on it directly,
// so we can share it between different renderers.
typedef struct RenderSettings {
    int width, height;
    bool showDebugLayer;
    Vec4 tint;
    int problemX, problemY;
    RenderingMethod renderingMethod;
    bool showCameraWidgets; // Bounding box / normals toggles live on each Model
} RenderSettings;

typedef struct Renderer {
    RenderSettings *settings;
    Vec4 *pixels;
} Renderer;

void RenderScene(Scene *scene, Renderer *renderer, double time);

#endif // RENDERER_HEADER

// Renderer Implementation
#if defined(RENDERER_IMPLEMENTATION)
#if !defined(RENDERER_IMPLEMENTED)
#define RENDERER_IMPLEMENTED

#include "stb_ds.h" // For dynamic arrays

//-----------------Mapping, Projection and Math Functions--------------------------------------------

Vec3 modelToScreen(Vec3 v_model, Mat4 fromModelToWorld, Mat4 fromWorldToCamera, Mat4 fromCameraToScreen){
    Vec3 v_screen = M4MulV3(fromCameraToScreen, M4MulV3(fromWorldToCamera, M4MulV3(fromModelToWorld, v_model)));
    return v_screen;
}

Vec3 worldToScreen(Vec3 v_world, Mat4 fromWorldToCamera, Mat4 fromCameraToScreen){
    Vec3 v_screen = M4MulV3(fromCameraToScreen, M4MulV3(fromWorldToCamera, v_world));
    return v_screen;
}

Vec2 mapToScreenRange(Vec3 v_screen, Mat4 fromCameraToScreen, Camera *camera, RenderSettings *settings){
    // After projection (ortho or perspective) v_screen is in NDC.
    // Map with a FIXED pixel scale centered on the window, so the distance between two projected
    // points doesn't depend on the window size (resizing shows more/less of the scene, no stretching).
    (void)fromCameraToScreen; (void)camera; // unused, kept for the signature
    const float S = 450.f; // pixels per NDC unit (half of the initial 900px window)
    Vec2 v_mapped = { settings->width  * 0.5f + v_screen.x * S,
                      settings->height * 0.5f + v_screen.y * S };
    return v_mapped;
}

Mat4 getM4Orthographic(Camera *camera){
    float zFar = camera->zBounds.x;     float zNear = camera->zBounds.y;
    float left = camera->xBounds.x;     float right = camera->xBounds.y;
    float bottom = camera->yBounds.x;   float top = camera->yBounds.y;
    return M4Orthographic(left, right, bottom, top, zNear, zFar);
}

Mat4 getM4Perspective(Camera *camera){
    float zFar = camera->zBounds.x;     float zNear = camera->zBounds.y;
    return M4Perspective(camera->fov, 1.f, zNear, zFar); // aspect = 1: fixed pixel scale handles the window shape
}

bool CameraBehind(Vec3 v_world, Mat4 fromWorldToCamera, float zNear){
    Vec3 v_camera = M4MulV3(fromWorldToCamera, v_world);
    if(v_camera.z >= -zNear) return true;
    return false;
}

bool TriangleBehindCamera(Vec3 v1_model, Vec3 v2_model, Vec3 v3_model, Mat4 fromModelToWorld, Mat4 fromWorldToCamera, float zNear){
    Vec3 v1_world = M4MulV3(fromModelToWorld, v1_model);
    Vec3 v2_world = M4MulV3(fromModelToWorld, v2_model);
    Vec3 v3_world = M4MulV3(fromModelToWorld, v3_model);
    if(CameraBehind(v1_world, fromWorldToCamera, zNear)) return true;
    if(CameraBehind(v2_world, fromWorldToCamera, zNear)) return true;
    if(CameraBehind(v3_world, fromWorldToCamera, zNear)) return true;
    return false;
}

//-------------------Drawing Functions---------------------------------------------------------------------

static void PutPixel(Renderer *renderer, int x, int y, Vec4 color) {
    RenderSettings *settings = renderer->settings;
    if (x < 0 || y < 0 || x >= settings->width || y >= settings->height) return;
    renderer->pixels[y*settings->width + x] = color;
}
 
static void LineDraw(Renderer *renderer, int x0, int y0, int x1, int y1, Vec4 color) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;   
    int dy = y1 > y0 ? y0 - y1 : y1 - y0;   
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;                      

    for (;;) {
        PutPixel(renderer, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } 
        if (e2 <= dx) { err += dx; y0 += sy; } 
    }
}

static void Draw3x3Square(Renderer *renderer, int x, int y, Vec4 colour){
    for(int dx=-1; dx<=1; dx++){
        for(int dy=-1; dy<=1; dy++){
            PutPixel(renderer, x+dx, y+dy, colour);
        }
    }
}

static void CameraDraw(Renderer *renderer, int x, int y, Vec4 colour){
    //Draws a plus sign around x,y
    Draw3x3Square(renderer, x, y, colour);
    x +=3;
    Draw3x3Square(renderer, x, y, colour);
    x-=6;
    Draw3x3Square(renderer, x, y, colour);
    x+=3; y+=3;
    Draw3x3Square(renderer, x, y, colour);
    y-=6;
    Draw3x3Square(renderer, x, y, colour);
}

// Draws a world-space segment: skips it if either end is closer than the near plane, otherwise projects and draws it
static void DrawSegmentWorld(Renderer *renderer, Vec3 a_world, Vec3 b_world, Mat4 fromWorldToCamera,
                             Mat4 fromCameraToScreen, Camera *camera, RenderSettings *settings, Vec4 color){
    float zNear = camera->zBounds.y;
    if(CameraBehind(a_world, fromWorldToCamera, zNear) || CameraBehind(b_world, fromWorldToCamera, zNear)) return;
    Vec2 a = mapToScreenRange(worldToScreen(a_world, fromWorldToCamera, fromCameraToScreen), fromCameraToScreen, camera, settings);
    Vec2 b = mapToScreenRange(worldToScreen(b_world, fromWorldToCamera, fromCameraToScreen), fromCameraToScreen, camera, settings);
    LineDraw(renderer, (int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
}

// Normals transform with the inverse-transpose of the model matrix's 3x3 part (stays correct under non-uniform scale)
static Mat3 NormalMatrix(Mat4 m){
    Mat3 m3 = { .r0 = m.r0.xyz, .r1 = m.r1.xyz, .r2 = m.r2.xyz };
    return M3Transpose(M3Inverse(m3));
}

//--------------------------------------Actually Rendering the Scene------------------------------------

void RenderScene(Scene *scene, Renderer *renderer, double time) {
    // Note that 'TextFormat' uses global storage for the strings it returns, so don't save pointers
    // to those directly if you need to store them long-term, copy the result into a persistant buffer.
    DrawText(TextFormat("Drawing scene with %d models", arrlen(scene->models)), 400, 100, 10, BLACK);
    RenderSettings *settings = renderer->settings;

    //Drawing the background
    for (int y = 0; y < settings->height; ++y) {
        for (int x = 0; x < settings->width; ++x) {
            Vec4 *pixel = &renderer->pixels[y*settings->width + x];
            pixel->r = settings->tint.r;
            pixel->g = settings->tint.g;
            pixel->b = settings->tint.b;
            pixel->a = 1.f;
        }
    }

    if (scene->activeCameraIndex < 0 || scene->activeCameraIndex >= arrlen(scene->cameras)) return;
    Camera *camera = &scene->cameras[scene->activeCameraIndex];
    camera->viewMatrix = M4LookAt(camera->Position, camera->Target,camera->up);
    Mat4 fromCameraToScreen = {};
    Mat4 fromWorldToCamera = camera->viewMatrix;
    Vec4 white = {1.f, 1.f, 1.f, 1.f};
    
    if(camera->projectionType == ORTHOGRAPHIC){
        fromCameraToScreen = getM4Orthographic(camera);
    }
    else if(camera->projectionType == PERSPECTIVE){
        fromCameraToScreen = getM4Perspective(camera);
    }

    //Added: going through every triangle on the model, projecting the 3D corners into 2D using the camera,
    //then drawing lines between them. We will also save the points with the max/min coordinate in each axis 
    //in order to later draw the bounding boxes (TODO)
    for (int modelIndex = 0; modelIndex < arrlen(scene->models); ++modelIndex) {
        Model model = scene->models[modelIndex];
        if(model.hidden) continue;
        ObjTriangle* triangles = model.triangles;
        
        Mat4 T = M4Diag(1,1,1,1);
        T.s03 = model.position.x; T.s13 = model.position.y; T.s23 = model.position.z;
        // Scale first (model space), then rotate, then translate
        Mat4 fromModelToWorld = M4MulM4(T, M4MulM4(model.Rz, M4MulM4(model.Ry, M4MulM4(model.Rx, M4ScaleV3(model.scale)))));
        Mat3 normalMatrix = NormalMatrix(fromModelToWorld);
        const float normalLength = 0.2f; // world units
        Vec4 red   = {1.f, 0.f, 0.f, 1.f};
        Vec4 green = {0.f, 1.f, 0.f, 1.f};

        float x_max = 0; float x_min = 0;
        float y_max = 0; float y_min = 0;
        float z_max = 0; float z_min = 0;

        for (int i = 0; i < arrlen(triangles); i += 1) {
            // Fetch the 3d vertices in model coordinates
            Vec3 v1_model = model.vertices[triangles[i].corners[0].values[0] - 1];
            Vec3 v2_model = model.vertices[triangles[i].corners[1].values[0] - 1];
            Vec3 v3_model = model.vertices[triangles[i].corners[2].values[0] - 1];

            if(i==0){
                x_max = x_min = v1_model.x;
                y_max = y_min = v1_model.y;
                z_max = z_min = v1_model.z;
            }
            else{
                x_max = MAX(x_max, MAX(v1_model.x, MAX(v2_model.x, v3_model.x)));
                y_max = MAX(y_max, MAX(v1_model.y, MAX(v2_model.y, v3_model.y)));
                z_max = MAX(z_max, MAX(v1_model.z, MAX(v2_model.z, v3_model.z)));
                x_min = MIN(x_min, MIN(v1_model.x, MIN(v2_model.x, v3_model.x)));
                y_min = MIN(y_min, MIN(v1_model.y, MIN(v2_model.y, v3_model.y)));
                z_min = MIN(z_min, MIN(v1_model.z, MIN(v2_model.z, v3_model.z)));
            }

            if(TriangleBehindCamera(v1_model, v2_model, v3_model, fromModelToWorld, fromWorldToCamera, camera->zBounds.y)) continue;
            //Project from model->screen
            Vec3 v1_screen = modelToScreen(v1_model, fromModelToWorld, fromWorldToCamera, fromCameraToScreen);
            Vec3 v2_screen = modelToScreen(v2_model, fromModelToWorld, fromWorldToCamera, fromCameraToScreen);
            Vec3 v3_screen = modelToScreen(v3_model, fromModelToWorld, fromWorldToCamera, fromCameraToScreen);

            //Mapping the values to the range of pixel positions in the image
            Vec2 v1 = mapToScreenRange(v1_screen, fromCameraToScreen, camera, settings);
            Vec2 v2 = mapToScreenRange(v2_screen, fromCameraToScreen, camera, settings);
            Vec2 v3 = mapToScreenRange(v3_screen, fromCameraToScreen, camera, settings);

            //Actually drawing the 3 edges
            LineDraw(renderer, (int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, white);
            LineDraw(renderer, (int)v1.x, (int)v1.y, (int)v3.x, (int)v3.y, white);
            LineDraw(renderer, (int)v3.x, (int)v3.y, (int)v2.x, (int)v2.y, white);
            if(model.showFaceNormals){
                // One normal per triangle, computed from its edges, drawn from the triangle's center (red)
                Vec3 center_model = V3MulS(V3Add(V3Add(v1_model, v2_model), v3_model), 1.f/3.f);
                Vec3 n_model = V3Cross(V3Sub(v2_model, v1_model), V3Sub(v3_model, v1_model));
                Vec3 n_world = V3Normalize(M3MulV3(normalMatrix, n_model));
                Vec3 start = M4MulV3(fromModelToWorld, center_model);
                Vec3 end   = V3Add(start, V3MulS(n_world, normalLength));
                DrawSegmentWorld(renderer, start, end, fromWorldToCamera, fromCameraToScreen, camera, settings, red);
            }
            if(model.showVertexNormals){
                // One normal per triangle corner, read from the model's normal data, drawn from the vertex (green)
                Vec3 corners_model[3] = {v1_model, v2_model, v3_model};
                for(int c = 0; c < 3; ++c){
                    int nIdx = triangles[i].corners[c].values[2]; // 1-based, 0 = no normal
                    if(nIdx <= 0 || nIdx > arrlen(model.normals)) continue;
                    Vec3 n_world = V3Normalize(M3MulV3(normalMatrix, model.normals[nIdx - 1]));
                    Vec3 start = M4MulV3(fromModelToWorld, corners_model[c]);
                    Vec3 end   = V3Add(start, V3MulS(n_world, normalLength));
                    DrawSegmentWorld(renderer, start, end, fromWorldToCamera, fromCameraToScreen, camera, settings, green);
                }
            }
        }

        if(model.showBoundingBox){
            //Here we draw the bounding box: 12 lines connecting the 8 corners to form the box
            Vec3 c_model[8] = {};
            Vec2 c[8] = {};
            bool tooClose = false;
            for (int i = 0; i < 8; ++i){
                c_model[i] = (Vec3){ (i & 1) ? x_max : x_min,
                            (i & 2) ? y_max : y_min,
                            (i & 4) ? z_max : z_min };
                // Skip the whole box if any corner is closer than the near plane
                if(CameraBehind(M4MulV3(fromModelToWorld, c_model[i]), fromWorldToCamera, camera->zBounds.y)){
                    tooClose = true;
                    break;
                }
                c_model[i] = modelToScreen(c_model[i], fromModelToWorld, fromWorldToCamera, fromCameraToScreen);
                c[i] = mapToScreenRange(c_model[i], fromCameraToScreen, camera, settings);
            }
            if(!tooClose)
            for (int i = 0; i < 8; ++i)
                for (int b = 1; b <= 4; b <<= 1)
                    if (!(i & b)) LineDraw(renderer, c[i].x, c[i].y, c[i|b].x, c[i|b].y, white);
        }
    }

    if(settings->showCameraWidgets){
        for(int i=0; i<arrlen(scene->cameras); i++){
            if (i == scene->activeCameraIndex || scene->cameras[i].hidden) continue; // skip self and hidden cameras
            Camera *camera_to_show = &scene->cameras[i];
            if(!CameraBehind(camera_to_show->Position, fromWorldToCamera, camera->zBounds.y)){
                Vec3 camera_position_screen = worldToScreen(camera_to_show->Position, fromWorldToCamera, fromCameraToScreen);
                Vec2 camera_position = mapToScreenRange(camera_position_screen, fromCameraToScreen, camera, settings);
                CameraDraw(renderer, camera_position.x, camera_position.y, white);
            }  
        }
    }
}

#endif // RENDERER_IMPLEMENTED
#endif // RENDERER_IMPLEMENTATION