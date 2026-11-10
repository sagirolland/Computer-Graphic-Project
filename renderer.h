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

void RenderScene(Scene *scene, Renderer *renderer, double time) {
    // Note that 'TextFormat' uses global storage for the strings it returns, so don't save pointers
    // to those directly if you need to store them long-term, copy the result into a persistant buffer.
    DrawText(TextFormat("Drawing scene with %d models", arrlen(scene->models)), 400, 100, 10, BLACK);
    
    RenderSettings *settings = renderer->settings;
    
    // You can use raylib to draw shapes to help debug some things, what you draw using raylib's functions
    // will end up on the debug layer and will be overlayed above the actual scene (if enabled).
    // Note that you're expected to implement everything yourself, those functions are only here for helping
    // you debug your own code!
    for (int modelIndex = 0; modelIndex < arrlen(scene->models); ++modelIndex) {
        Model model = scene->models[modelIndex];
        
        for (int i = 0; i < arrlen(model.vertices); i += 3) {
            Vector2 v1 = { (model.vertices[i + 0].x + 1.f) * settings->width / 2, (model.vertices[i + 0].y + 1.f) * settings->width / 2 };
            Vector2 v2 = { (model.vertices[i + 1].x + 1.f) * settings->width / 2, (model.vertices[i + 1].y + 1.f) * settings->width / 2 };
            Vector2 v3 = { (model.vertices[i + 2].x + 1.f) * settings->width / 2, (model.vertices[i + 2].y + 1.f) * settings->width / 2 };
            DrawTriangle(v1, v2, v3, WHITE);
        }
    }
    
    // Here's where the real stuff happens: we go over every pixel and initialize it with some random values,
    // but soon you'll render geometry here instead :)
    
    // A little advice, I recommend using the naming convensions for projection matrices from Anton Mikhailov,
    // i.e. fromXtoY (fromModelToCamera, fromCameraToScreen, etc.),
    // that'll save you a lot of time. Reference: https://youtu.be/QV5OwOCtHQE?t=2750
    
    for (int y = 0; y < settings->height; ++y) {
        for (int x = 0; x < settings->width; ++x) {
            Vec4 *pixel = &renderer->pixels[y*settings->width + x];
            
            if (x == settings->problemX && y == settings->problemY) {
                *pixel = (Vec4){0};
                // An example of drawing a circle and some text with raylib over your scene
                DrawCircleLines(x, y, 2, RED);
                DrawText("^problem", x, y + 2, 8, RED);
            } else {
                pixel->r = (((float)x / (float)(renderer->settings->width))  + renderer->settings->tint.r) / 2.f;
                pixel->g = (((float)y / (float)(renderer->settings->height)) + renderer->settings->tint.g) / 2.f;
                pixel->b = (((sin(time) + 1.0)/2.0) + renderer->settings->tint.b) / 2.f;
                pixel->a = 255;
            }
        }
    }
}

#endif // RENDERER_IMPLEMENTED
#endif // RENDERER_IMPLEMENTATION