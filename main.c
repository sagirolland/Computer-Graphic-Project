#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define RENDERER_IMPLEMENTATION
#include "renderer.h"

#define RENDERER_GL_IMPLEMENTATION
#include "renderer-gl.h"

#define MODEL_IMPLEMENTATION
#include "model.h"

#define SCENE_IMPLEMENTATION
#include "scene.h"

#define INTERFACE_IMPLEMENTATION
#include "interface.h"

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h> // For dynamic arrays

int main()
{
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitWindow(900, 900, "Computer Graphics Demo :)");
    SetTargetFPS(60);
    
    SetExitKey(-1); // Disable closing the window on escape
    
    // In raylib, Image is an array of pixels stored in a CPU (RAM) buffer
    Image renderImage = {};
    // while a Texture is an array of pixels stored in a GPU (VRAM) buffer
    Texture renderTexture = {};
    
    // A RenderTexture is a Frame Buffer Object (FBO) in OpenGL terms, it's a Texture that
    // can be a target of OpenGL draw commands.
    RenderTexture debugTexture = {};
    
    // Our main structures are Scene, RenderSettings and Renderer, initialized here.
    Scene scene = {};
    { // TODO(student): Move this to the UI
        const char *modelPath = TextFormat("%s" PATH_SEPERATOR "models" PATH_SEPERATOR "monkey.obj", GetApplicationDirectory());
        AddModel(&scene, LoadModelFromObjFile(modelPath));
    }
    
    RenderSettings renderSettings = {};
    renderSettings.tint = (Vec4){.42f, .20f, .47f, 1.f};
    renderSettings.problemX = 300;
    renderSettings.problemY = 300;
    renderSettings.showDebugLayer = true;
    
    Renderer renderer = {};
    renderer.settings = &renderSettings;
    
    // RendererGL is for HW3, ignore it for now.
    RendererGL rendererGL = {};
    rendererGL.settings = &renderSettings;
    RendererLoadShaders(&rendererGL);
    
    while (!WindowShouldClose())
    {
        { // Resize our textures to match the window's dimensions
            static bool initialized = false;
            int currentWidth  = GetScreenWidth();
            int currentHeight = GetScreenHeight();
            
            if (!initialized || (currentWidth != renderSettings.width) || (currentHeight != renderSettings.height)) {
                if (initialized) {
                    UnloadRenderTexture(debugTexture);
                    UnloadRenderTexture(rendererGL.target);
                    UnloadTexture(renderTexture);
                    UnloadImage(renderImage);
                }
                
                renderSettings.width  = currentWidth;
                renderSettings.height = currentHeight;
                
                renderImage = GenImageColor(renderSettings.width, renderSettings.height, BLANK);
                ImageFormat(&renderImage, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
                
                renderTexture = LoadTextureFromImage(renderImage);
                debugTexture  = LoadRenderTexture(renderSettings.width, renderSettings.height);
                
                rendererGL.target = LoadRenderTexture(renderSettings.width, renderSettings.height);
                
                int pixelCount = renderSettings.width * renderSettings.height;
                renderer.pixels = MemRealloc(renderer.pixels,      pixelCount * sizeof(*renderer.pixels));
                
                initialized = true;
            }
        }
        
        switch (renderSettings.renderingMethod) {
            case RENDERING_METHOD_SOFTWARE: { 
                // debugTexture allows you to call raylib's drawing functions inside your renderer
                // for debug purposes. If you don't use it the renderTexture will be drawn on top
                // and hide the result of those calls.
                BeginTextureMode(debugTexture);
                { // Update the pixel buffer
                    ClearBackground(BLANK); // Clear the debugTexture
                    RenderScene(&scene, &renderer, GetFrameTime()); // <-- Here's where you shine!
                    UpdateTexture(renderTexture, renderer.pixels);
                }
                EndTextureMode();
            } break;
            case RENDERING_METHOD_OPENGL: {
                RenderGLScene(&scene, &rendererGL, GetFrameTime()); // <-- Here's where you shine! (in HW3)
            } break;
            case RENDERING_METHOD_RAYTRACING: {
                // TODO(student): It's already HW4?! How's the time passed so fast :)
                // you're mature enough to fill it yourself, right?
            } break;
            case RENDERING_METHOD_COUNT: { /* this case is here only to disable a compiler warning */ } break;
        }
        
        ClearBackground(BLACK);
        BeginDrawing();
        { // Draw the textures to the window
            // Texture2D uses inverted y coordinate so we need to flip the source rectangle height,
            // essentially vertically mirroring it so it will be drawn correctly.
            
            Rectangle sourceRect = (Rectangle) {0, 0, renderSettings.width, -renderSettings.height};
            Rectangle destRect   = (Rectangle) {0, 0, renderSettings.width,  renderSettings.height};
            
            switch (renderSettings.renderingMethod) {
                case RENDERING_METHOD_SOFTWARE:   { DrawTexturePro(renderTexture, sourceRect, destRect, (Vector2) {0, 0}, 0.f, WHITE); } break;
                case RENDERING_METHOD_OPENGL:     { DrawTexturePro(rendererGL.target.texture, sourceRect, destRect, (Vector2) {0, 0}, 0.f, WHITE); } break;
                case RENDERING_METHOD_RAYTRACING: { /* TODO(student) */ } break;
                case RENDERING_METHOD_COUNT: { /* this case is here only to disable a compiler warning */ } break;
            }
            
            if (renderSettings.showDebugLayer && renderSettings.renderingMethod == RENDERING_METHOD_SOFTWARE) {
                DrawTexturePro(debugTexture.texture, sourceRect, destRect, (Vector2) {0, 0}, 0.f, WHITE);
            }
            
            // After drawing the scene, we can draw the user interface
            CreateInterface(&renderSettings, &scene);
        }
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
