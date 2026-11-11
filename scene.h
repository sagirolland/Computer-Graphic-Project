#if !defined(SCENE_HEADER)
#define SCENE_HEADER

#include "model.h"
#include "math.h"

typedef enum {
    ORTHOGRAPHIC = 0,
    PERSPECTIVE = 1,
} ProjectionType;

typedef struct Camera {
    Vec3 Position;
    Vec3 Target;
    Vec2 xBounds; //left, right
    Vec2 yBounds; //top, bottom
    Vec2 zBounds; //zFar, zNear
    Vec3 up;
    //Vec3 euclideanBasis[3];
    //Vec4 homogeneousBasis[4];
    Mat4 viewMatrix;
    ProjectionType projectionType;
    float fov;
    float aspect;
    bool hidden;
    
    // TODO(student)
} Camera;

typedef struct Scene {
    Model *models;
    Camera *cameras;
	
	int activeModelIndex;
	int activeCameraIndex;
    
    bool advanceTime;
    float step;
} Scene;

void AddModel(Scene *scene, Model model);

void RemoveModel(Scene *scene, int modelIndex);
void AddCamera(Scene *scene, Camera camera);
void RemoveCamera(Scene *scene, int cameraIndex);
Camera DefaultCamera(void);

#endif // SCENE_HEADER

// Scene Implementation
#if defined(SCENE_IMPLEMENTATION) 
#if !defined(SCENE_IMPLEMENTED)
#define SCENE_IMPLEMENTED

#include "stb_ds.h" // For dynamic arrays

void AddModel(Scene *scene, Model model) {
    arrput(scene->models, model);
}

void RemoveModel(Scene *scene, int modelIndex) {
    arrdel(scene->models, modelIndex);
}

void AddCamera(Scene *scene, Camera camera) {
    arrput(scene->cameras, camera);
}

void RemoveCamera(Scene *scene, int cameraIndex) {
    arrdel(scene->cameras, cameraIndex);
}

// scene.h (implementation section)
Camera DefaultCamera(void) {
    return (Camera){
        .Position = {0, 0, 5}, .Target = {0, 0, 0}, .up = {0, 1, 0},
        .xBounds = {-2, 2}, .yBounds = {-2, 2},
        .zBounds = {100.f, 0.1f},  // (far, near)   
        .projectionType = PERSPECTIVE,
        .fov = 1.0472f,             // 60° in radians
        .aspect = 1.f,
    };
}

#endif // SCENE_IMPLEMENTED
#endif // SCENE_IMPLEMENTATION