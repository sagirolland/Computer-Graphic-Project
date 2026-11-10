#if !defined(SCENE_HEADER)
#define SCENE_HEADER

#include "model.h"
#include "math.h"

typedef enum {
    HOMOGRAPHIC = 0,
    PERSPECTIVE = 1,
} ProjectionType;

typedef struct Camera {
    Vec3 Position;
    Vec3 Target;
    Vec2 xBounds;
    Vec2 yBounds;
    Vec2 zBounds;
    //Vec3 euclideanBasis[3];
    //Vec4 homogeneousBasis[4];
    Mat4 viewMatrix;
    ProjectionType projectionType;
    float fov;
    float aspect;
    
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

#endif // SCENE_IMPLEMENTED
#endif // SCENE_IMPLEMENTATION