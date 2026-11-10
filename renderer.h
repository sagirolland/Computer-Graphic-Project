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
    //Added:
    bool showBoundingBox;
    bool showFaceNormals;
    bool showVertexNormals;
    bool drawCameraWidgets;
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

// Writes one pixel into the buffer; pixels outside the window are ignored
static void PutPixel(Renderer *renderer, int x, int y, Vec4 color) {
    RenderSettings *settings = renderer->settings;
    if (x < 0 || y < 0 || x >= settings->width || y >= settings->height) return;
    renderer->pixels[y*settings->width + x] = color;
}

// Bresenham's line algorithm 
static void LineDraw(Renderer *renderer, int x0, int y0, int x1, int y1, Vec4 color) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;   // |dx|
    int dy = y1 > y0 ? y0 - y1 : y1 - y0;   // -|dy|
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;                      // error term e = dx - |dy|

    for (;;) {
        PutPixel(renderer, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } // step in x
        if (e2 <= dx) { err += dx; y0 += sy; } // step in y
    }
}

void RenderScene(Scene *scene, Renderer *renderer, double time) {
    // Note that 'TextFormat' uses global storage for the strings it returns, so don't save pointers
    // to those directly if you need to store them long-term, copy the result into a persistant buffer.
    DrawText(TextFormat("Drawing scene with %d models", arrlen(scene->models)), 400, 100, 10, BLACK);
    RenderSettings *settings = renderer->settings;
    Camera *camera = &scene->cameras[scene->activeCameraIndex];
    camera->viewMatrix = M4LookAt(camera->Position, camera->Target,(Vec3){0,0,1});
    Mat4 fromCameraToScreen = {};
    float zFar = camera->zBounds.x; float zNear = camera->zBounds.y;
    float left = camera->xBounds.x; float right = camera->xBounds.y;
    float bottom = camera->yBounds.x; float top = camera->yBounds.y;
    if(camera->projectionType == HOMOGRAPHIC){
        fromCameraToScreen = M4Orthographic(left, right, bottom, top, zNear, zFar);
    }
    else if(camera->projectionType == PERSPECTIVE){
        fromCameraToScreen = M4Perspective(camera->fov, camera->aspect, zNear, zFar);
    }

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

    //Added: going through every triangle on the model, projecting the 3D corners into 2D using the camera,
    //then drawing lines between them. We will also save the points with the max/min coordinate in each axis 
    //in order to later draw the bounding boxes (TODO)
    for (int modelIndex = 0; modelIndex < arrlen(scene->models); ++modelIndex) {
        Model model = scene->models[modelIndex];
        if(model.hidden) continue;
        ObjTriangle* triangles = model.triangles;
        Vec3 t = V3Neg(model.position);
        Mat4 T = M4Diag(1,1,1,1);
        T.s03 = t.x; T.s13 = t.y; T.s23=t.z;
        Mat4 fromModelToWorld = M4MulM4(T, M4MulM4(model.Rz, M4MulM4(model.Ry, model.Rx)));
        Mat4 fromWorldToCamera = camera->viewMatrix;

        for (int i = 0; i < arrlen(triangles); i += 1) {
            // Fetch the 3d vertices in model coordinates
            Vec3 v1_model = model.vertices[triangles[i].corners[0].values[0] - 1];
            Vec3 v2_model = model.vertices[triangles[i].corners[1].values[0] - 1];
            Vec3 v3_model = model.vertices[triangles[i].corners[2].values[0] - 1];

            //Model->World
            Vec3 v1_world = M4MulV3(fromModelToWorld, v1_model);
            Vec3 v2_world = M4MulV3(fromModelToWorld, v2_model);
            Vec3 v3_world = M4MulV3(fromModelToWorld, v3_model);

            //World->camera
            Vec3 v1_camera = M4MulV3(fromWorldToCamera, v1_world);
            Vec3 v2_camera = M4MulV3(fromWorldToCamera, v2_world);
            Vec3 v3_camera = M4MulV3(fromWorldToCamera, v3_world);

            //Camera->Image (projection)
            
            Vec2 v1_image = M4MulV3(fromCameraToScreen, v1_camera).xy;
            Vec2 v2_image = M4MulV3(fromCameraToScreen, v2_camera).xy;
            Vec2 v3_image = M4MulV3(fromCameraToScreen, v3_camera).xy;

            //Mapping the values to the range of pixel positions in the image
            float left_mapped =  M4MulV3(fromCameraToScreen, (Vec3){left, bottom, zNear}).x;
            float right_mapped =  M4MulV3(fromCameraToScreen, (Vec3){right, bottom, zNear}).x;
            float bottom_mapped =  M4MulV3(fromCameraToScreen, (Vec3){left, bottom, zNear}).y;
            float top_mapped =  M4MulV3(fromCameraToScreen, (Vec3){left, top, zNear}).y;
            int width = settings->width;
            int height = settings->height;
            Vec2 v1 = { width * (v1_image.x - left_mapped) / (right_mapped - left_mapped),
                           height * (v1_image.y - bottom_mapped) / (top_mapped - bottom_mapped)};
            Vec2 v2 = { width * (v2_image.x - left_mapped) / (right_mapped - left_mapped),
                            height * (v2_image.y - bottom_mapped) / (top_mapped - bottom_mapped)};
            Vec2 v3 = { width * (v3_image.x - left_mapped) / (right_mapped - left_mapped),
                           height * (v3_image.y - bottom_mapped) / (top_mapped - bottom_mapped)};

            //Actually drawing the 3 edges
            Vec4 white = {1.f, 1.f, 1.f, 1.f};
            LineDraw(renderer, (int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, white);
            LineDraw(renderer, (int)v1.x, (int)v1.y, (int)v3.x, (int)v3.y, white);
            LineDraw(renderer, (int)v3.x, (int)v3.y, (int)v2.x, (int)v2.y, white);
        }
    }
}

#endif // RENDERER_IMPLEMENTED
#endif // RENDERER_IMPLEMENTATION