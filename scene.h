#if !defined(SCENE_HEADER)
#define SCENE_HEADER

#include "model.h"
#include "math.h"

typedef struct Camera {
    Vec3 Position;
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

#endif // SCENE_IMPLEMENTED
#endif // SCENE_IMPLEMENTATION