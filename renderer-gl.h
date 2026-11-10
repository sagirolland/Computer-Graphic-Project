#if !defined(RENDERER_GL_HEADER)
#define RENDERER_GL_HEADER

#include "scene.h"
#include "renderer.h"

typedef enum {
    LOCATION_POSITION = 0,
    LOCATION_TEXCOORD = 1,
    LOCATION_NORMAL   = 2,
} GpuVertexAttributeLocation;

typedef struct ShaderUniformLocations {
    int toScreenFromModel;
    int normalTransform;
    // TODO(student): add whatever uniforms you need in your model shaders
} ShaderUniformLocations;

typedef enum {
    SHADER_TYPE_FLAT,
    SHADER_TYPE_GOURAUD,
    SHADER_TYPE_PHONG,
    
    SHADER_TYPE_COUNT,
} ShaderType;

typedef struct Shader {
    unsigned int id;
    ShaderUniformLocations loc;
} Shader;

typedef struct RendererGL {
    RenderSettings *settings;
    
    struct {    
        unsigned int id;
        int VAO;
        int VBO;
        int timeLoc;
    } backgroundShader;
    
    Shader shaders[SHADER_TYPE_COUNT];
    
    RenderTexture target;
} RendererGL;

void RendererLoadShaders(RendererGL *renderer);
void RendererUnloadShaders(RendererGL *renderer);
void RenderSceneGL(Scene *scene, RendererGL *renderer, double deltaTime);

void LoadModelGLBuffers(Model *model);
void UnloadModelGLBuffers(Model *model);

#endif // RENDERER_GL_HEADER

// Renderer Implementation
#if defined(RENDERER_GL_IMPLEMENTATION)
#if !defined(RENDERER_GL_IMPLEMENTED)
#define RENDERER_GL_IMPLEMENTED

#include <stb_ds.h> // For dynamic arrays
#include <assert.h>
#include "math.h"

#include <rlgl.h> // OpenGL time! :)

void RendererLoadModelShader(RendererGL *renderer, ShaderType shaderType) {
    assert(shaderType < SHADER_TYPE_COUNT);
    
    const char *shaderName = NULL;
    Shader *shader = &renderer->shaders[shaderType];
    
    switch (shaderType) {
        case SHADER_TYPE_FLAT:    { shaderName = "flat";    } break;
        case SHADER_TYPE_GOURAUD: { shaderName = "gouraud"; } break;
        case SHADER_TYPE_PHONG:   { shaderName = "phong";   } break;
        
        default: { assert(false); } break;
    }
    
    { // Load shader program
        const char *vShaderPath = TextFormat("%s/shaders/vshader_%s.glsl", GetApplicationDirectory(), shaderName);
        char *vShaderData = LoadFileText(vShaderPath);
        const char *fShaderPath = TextFormat("%s/shaders/fshader_%s.glsl", GetApplicationDirectory(), shaderName);
        char *fShaderData = LoadFileText(fShaderPath);
        
        shader->id = rlLoadShaderProgram(vShaderData, fShaderData);
        assert(shader->id > 0);
        
        UnloadFileText(vShaderData);
        UnloadFileText(fShaderData);
    }
    
    // Fill uniforms locations
    shader->loc.toScreenFromModel = rlGetLocationUniform(shader->id, "toScreenFromModel");
    shader->loc.normalTransform   = rlGetLocationUniform(shader->id, "normalTransform");
    // TODO(student)
    
    if (rlCheckErrors()) TraceLog(LOG_ERROR, "%s:%d: Something wrong with OpenGL (see above)", __FILE__, __LINE__);
}

void RendererLoadShaders(RendererGL *renderer) {
    { // Load Background shader program
        const char *vShaderPath = TextFormat("%s/shaders/vshader_background.glsl", GetApplicationDirectory());
        char *vShaderData = LoadFileText(vShaderPath);
        const char *fShaderPath = TextFormat("%s/shaders/fshader_background.glsl", GetApplicationDirectory());
        char *fShaderData = LoadFileText(fShaderPath);
        
        renderer->backgroundShader.id = rlLoadShaderProgram(vShaderData, fShaderData);
        assert(renderer->backgroundShader.id > 0);
        renderer->backgroundShader.timeLoc = rlGetLocationUniform(renderer->backgroundShader.id, "time");
        
        UnloadFileText(vShaderData);
        UnloadFileText(fShaderData);
        
        static const Vec2 vertices[] = {
            {  1.f,  1.f },
            { -1.f,  1.f },
            {  1.f, -1.f },
            {  1.f, -1.f },
            { -1.f,  1.f },
            { -1.f, -1.f },
        };
        renderer->backgroundShader.VAO = rlLoadVertexArray();
        rlEnableVertexArray(renderer->backgroundShader.VAO);
        
        renderer->backgroundShader.VBO = rlLoadVertexBuffer(vertices, sizeof(vertices), false);
        rlSetVertexAttribute(LOCATION_POSITION, 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(LOCATION_POSITION);
        
        rlDisableVertexArray();
        
        if (rlCheckErrors()) TraceLog(LOG_ERROR, "%s:%d: Something wrong with OpenGL (see above)", __FILE__, __LINE__);
    }
    
    RendererLoadModelShader(renderer, SHADER_TYPE_FLAT);
    // TODO(student): Add create and add other shaders
    // RendererLoadModelShader(renderer, SHADER_TYPE_GOURAUD);
    // RendererLoadModelShader(renderer, SHADER_TYPE_PHONG);
}

void RendererUnloadShader(RendererGL *renderer, ShaderType shaderType) {
    assert(shaderType < SHADER_TYPE_COUNT);
    
    Shader *shader = &renderer->shaders[shaderType];
    rlUnloadShaderProgram(shader->id);
    memset(&shader->loc, -1, sizeof(shader->loc));
    
    if (rlCheckErrors()) TraceLog(LOG_ERROR, "%s:%d: Something wrong with OpenGL (see above)", __FILE__, __LINE__);
}

void RendererUnloadShaders(RendererGL *renderer) {
    RendererUnloadShader(renderer, SHADER_TYPE_FLAT);
    RendererUnloadShader(renderer, SHADER_TYPE_GOURAUD);
    RendererUnloadShader(renderer, SHADER_TYPE_PHONG);
}

void LoadModelGLBuffers(Model *model) {
    // A Vertex Array Object (VAO) combines all of the information you need to pass for a vertex,
    // i.e. normal, texture coordinate, color...
    model->gpuVertexArrayId = rlLoadVertexArray();
    rlEnableVertexArray(model->gpuVertexArrayId);
    
    // A Vertex Buffer Object (VBO) is one "line" of mentioned information
    if (model->vertices) {
        model->gpuVertexBufferId = rlLoadVertexBuffer(model->vertices, model->vertexCount * sizeof(model->vertices[0]), false);
        rlSetVertexAttribute(LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(LOCATION_POSITION);
    }
    if (model->normals) {
        model->gpuNormalBufferId = rlLoadVertexBuffer(model->normals, model->vertexCount * sizeof(model->normals[0]), false);
        rlSetVertexAttribute(LOCATION_NORMAL, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(LOCATION_NORMAL);
    }
    
    rlDisableVertexArray();
    
    if (rlCheckErrors()) TraceLog(LOG_ERROR, "%s:%d: Something wrong with OpenGL (see above)", __FILE__, __LINE__);
}

void UnloadModelGLBuffers(Model *model) {
    rlUnloadVertexBuffer(model->gpuVertexBufferId);
    rlUnloadVertexBuffer(model->gpuNormalBufferId);
    
    model->gpuVertexBufferId = 0;
    model->gpuNormalBufferId = 0;
}

void RenderGLScene(Scene *scene, RendererGL *renderer, double deltaTime) {
    BeginTextureMode(renderer->target);
    rlClearScreenBuffers();
    
    static double time = 0.0;
    if (scene->advanceTime) {
        time += GetFrameTime();
    }
    
    { // Draw background
        rlEnableShader(renderer->backgroundShader.id);
        
        float uniformTime = (float)time;
        rlSetUniform(renderer->backgroundShader.timeLoc, &uniformTime, RL_SHADER_UNIFORM_FLOAT, 1);
        rlEnableVertexArray(renderer->backgroundShader.VAO);
        rlDrawVertexArray(0, 6); // 6 vertices for a single quad
    }
    
    // TODO(student): Actually render the scene
    
    EndTextureMode();
    rlDisableDepthTest(); // Needed for the default raylib shader (used for GUI etc.)
}


#endif // RENDERER_GL_IMPLEMENTED
#endif // RENDERER_GL_IMPLEMENTATION
