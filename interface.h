#if !defined(INTERFACE_HEADER)
#define INTERFACE_HEADER

#include "renderer.h"
#include "scene.h"
#include "primitives.h"

void CreateInterface(RenderSettings *settings, Scene *scene);

#endif // INTERFACE_HEADER


// UI Implementation
#if defined(INTERFACE_IMPLEMENTATION)
#if !defined(INTERFACE_IMPLEMENTED)
#define INTERFACE_IMPLEMENTED

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include <gui_window_file_dialog.h> // For file dialogs

// enums
typedef enum FileDialogOperaion {
    FILE_DIALOG_LOAD_MODEL = 1,
} FileDialogOperaion;

//
// Provided helper functions
//
static Rectangle Box_int(int *value, bool *editMode, bool *dragging, char *textValue, int textValueCap, float *left, int uiTop) {
    Rectangle rect = (Rectangle) { *left, uiTop, 40, 16 };
    *left += rect.width + 2;
    
    bool mouseInRect = CheckCollisionPointRec(GetMousePosition(), rect);
    static bool moved = false; // Did the current press actually drag? (only one box drags at a time)

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *editMode = !GuiIsLocked() && mouseInRect;
        *dragging = *editMode;
        moved = false;
    }
    if (IsKeyReleased(KEY_ENTER)) *editMode = false;

    if (*dragging) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            if (delta.y != 0) moved = true;

            if (IsKeyDown(KEY_LEFT_SHIFT)) delta.y *= 10;
            *value -= (int)(delta.y/2.f);

            snprintf(textValue, textValueCap, "%i", *value);
        } else {
            *dragging = false;
            if (!mouseInRect || moved) { // a drag shouldn't leave the box in edit mode (keeps WASD usable)
                *editMode = false;
            }
        }
    }
    
    if (!(*editMode)) {
        snprintf(textValue, textValueCap, "%i", *value);
    } else {
        if (IsKeyPressed(KEY_BACKSPACE) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
            memset(textValue, '\0', textValueCap);
        }
    }
    
    GuiValueBoxInt(rect, NULL, textValue, textValueCap, value, *editMode);
    
    return rect;
}

static Rectangle Box_float(float *value, bool *editMode, bool *dragging, char *textValue, int textValueCap, float *left, float uiTop) {
    Rectangle rect = (Rectangle) { *left, uiTop, 40, 16 };
    *left += rect.width + 2;
    
    bool mouseInRect = CheckCollisionPointRec(GetMousePosition(), rect);
    static bool moved = false; // Did the current press actually drag? (only one box drags at a time)

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *editMode = !GuiIsLocked() && mouseInRect;
        *dragging = *editMode;
        moved = false;
    }
    if (IsKeyReleased(KEY_ENTER)) *editMode = false;

    if (*dragging) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            if (delta.y != 0) moved = true;

            if (IsKeyDown(KEY_LEFT_SHIFT)) delta.y *= 10;
            *value -= delta.y*0.01f;

            snprintf(textValue, textValueCap, "%2.2f", *value);
        } else {
            *dragging = false;
            if (!mouseInRect || moved) { // a drag shouldn't leave the box in edit mode (keeps WASD usable)
                *editMode = false;
            }
        }
    }
    
    if (!(*editMode)) {
        snprintf(textValue, textValueCap, "%2.2f", *value);
    } else {
        if (IsKeyPressed(KEY_BACKSPACE) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
            memset(textValue, '\0', textValueCap);
        }
    }
    GuiValueBoxFloat(rect, NULL, textValue, textValueCap, value, *editMode);
    
    return rect;
}

// Macros for adding float/int edit boxes that can be changed by dragging up/down with the mouse,
// or manually with the keyboard. The 'left' parameter is automatically incremented with the box width.
// The reason those are macros and not functions, is that the statet of the box needs to stay persistant,
// and the macros are copied into place by the preprocessors so each static variable here has it's own
// place in memory and not collide with other boxes data.
#define VALUE_BOX(type, x, left, top, tip) \
do { \
type *value = x; \
int boxId = boxCounter++; \
static bool dragging = false; \
bool editMode = (boxId == activeBoxId); \
if (activateBox && editMode) { \
activateBox = false; \
} \
bool guiLocked = editMode && GuiIsLocked(); \
if (guiLocked) GuiUnlock(); \
\
static char textValue[RAYGUI_VALUEBOX_MAX_CHARS + 1] = {0}; \
Rectangle rect = Box_##type(value, &editMode, &dragging, textValue, sizeof(textValue), left, top); \
if (editMode) { \
activeBoxId = boxId; \
GuiLock(); \
} else if (boxId == activeBoxId) { \
activeBoxId = -1; \
}\
if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) { \
tooltip = tip; \
} \
if (guiLocked) GuiLock(); \
} while (0)

#define BOX_FLOAT(x, left, top, tip) VALUE_BOX(float, x, left, top, tip)
#define BOX_INT(x, left, top, tip)   VALUE_BOX(int, x, left, top, tip)

#define BOX_FLOAT_CLAMP0(x, left, top, tip) \
do { \
VALUE_BOX(float, x, left, top, tip); \
*x = MAX(*x, 0.f); \
} while(0)

#define BOX_FLOAT_CLAMP01(x, left, top, tip) \
do { \
BOX_FLOAT(x, left, top, tip); \
*x = CLAMP(*x, 0.f, 1.f); \
} while(0)

#define BOX_INT_CLAMP(x, min, max, left, top, tip) \
do { \
VALUE_BOX(int, x, left, top, tip); \
*x = CLAMP(*x, min, max); \
} while(0)

static inline Vec4 V4FromColor(Color c) {
    return (Vec4){(float)(c).r/255.f, (float)(c).g/255.f, (float)(c).b/255.f, (float)(c).a/255.f};
}

static inline Color ColorFromV4(Vec4 v) {
    return (Color){(int)(v.r*255.f), (int)(v.g*255.f), (int)(v.b*255.f), (int)(v.a*255.f)};
}

static void RotateModelAxis(Mat4 *rotation, Vec3 axis, float angle) {
    if (M4Eq(*rotation, (Mat4){0})) {
        *rotation = M4Identity();
    }

    *rotation = M4MulM4(M4Rotate(angle, axis), *rotation);
}

static void RotateCameraView(Camera *camera, Vec3 axis, float angle) {
    Vec3 viewDirection = V3Sub(camera->Target, camera->Position);
    if (V3Length(viewDirection) < EPSILON) return;

    // Rotate the view direction AND the up vector together, so they never become parallel
    Mat4 R = M4Rotate(angle, axis);
    camera->Target = V3Add(camera->Position, M4MulV3(R, viewDirection));
    camera->up     = M4MulV3(R, camera->up); // pure rotation (no translation), safe for directions
}

static Vec3 CameraRelativeMovement(Camera *camera, Vec3 movement) {
    Vec3 forward = V3Normalize(V3Sub(camera->Target, camera->Position));
    if (V3Length(forward) < EPSILON) return (Vec3){0};

    Vec3 worldUp = camera->up;
    Vec3 right = V3Cross(forward, worldUp);
    if (V3Length(right) < EPSILON) right = (Vec3){1.f, 0.f, 0.f};
    right = V3Normalize(right);
    Vec3 up = V3Normalize(V3Cross(right, forward));

    Vec3 result = V3Add(V3MulS(right, movement.x),
                        V3Add(V3MulS(up, movement.y), V3MulS(forward, movement.z)));
    if (V3Length(result) > 1.f) result = V3Normalize(result);
    return result;
}

static void OrbitCamera(Camera *camera, float yaw, float pitch) {
    Vec3 offset = V3Sub(camera->Position, camera->Target);
    if (V3Length(offset) < EPSILON) return;

    Vec3 up = V3Normalize(camera->up);
    if (V3Length(up) < EPSILON) up = (Vec3){0.f, 1.f, 0.f};

    Mat4 rotation = M4Rotate(yaw, up);
    offset = M4MulV3(rotation, offset);
    up = V3Normalize(M4MulV3(rotation, up));

    Vec3 forward = V3Normalize(V3MulS(offset, -1.f));
    Vec3 right = V3Normalize(V3Cross(forward, up));
    if (V3Length(right) < EPSILON) return;

    float currentPitch = asinf(CLAMP(V3Dot(forward, up), -1.f, 1.f));
    float targetPitch = CLAMP(currentPitch + pitch, -1.5533f, 1.5533f);
    rotation = M4Rotate(targetPitch - currentPitch, right);
    offset = M4MulV3(rotation, offset);
    up = V3Normalize(M4MulV3(rotation, up));

    camera->Position = V3Add(camera->Target, offset);
    camera->up = up;
}

static void PanCamera(Camera *camera, float deltaX, float deltaY, int viewportHeight) {
    Vec3 forward = V3Normalize(V3Sub(camera->Target, camera->Position));
    Vec3 right = V3Normalize(V3Cross(forward, camera->up));
    if (V3Length(right) < EPSILON || viewportHeight <= 0) return;
    Vec3 up = V3Normalize(V3Cross(right, forward));

    float worldPerPixel;
    if (camera->projectionType == PERSPECTIVE) {
        float distance = V3Length(V3Sub(camera->Target, camera->Position));
        float fov = CLAMP(camera->fov, 0.0873f, 2.9671f);
        worldPerPixel = 2.f * distance * tanf(fov * 0.5f) / viewportHeight;
    } else {
        worldPerPixel = (camera->yBounds.y - camera->yBounds.x) / viewportHeight;
    }

    Vec3 movement = V3Add(V3MulS(right, -deltaX * worldPerPixel),
                          V3MulS(up, deltaY * worldPerPixel));
    camera->Position = V3Add(camera->Position, movement);
    camera->Target = V3Add(camera->Target, movement);
}

static void ZoomCamera(Camera *camera, float wheel) {
    if (wheel == 0.f) return;

    float factor = powf(0.9f, wheel);
    if (camera->projectionType == PERSPECTIVE) {
        Vec3 offset = V3Sub(camera->Position, camera->Target);
        float distance = V3Length(offset);
        if (distance < EPSILON) return;
        distance = CLAMP(distance * factor, 0.01f, 100000.f);
        camera->Position = V3Add(camera->Target, V3MulS(V3Normalize(offset), distance));
    } else {
        float centerX = (camera->xBounds.x + camera->xBounds.y) * 0.5f;
        float centerY = (camera->yBounds.x + camera->yBounds.y) * 0.5f;
        float halfWidth = CLAMP((camera->xBounds.y - camera->xBounds.x) * 0.5f * factor, 0.005f, 100000.f);
        float halfHeight = CLAMP((camera->yBounds.y - camera->yBounds.x) * 0.5f * factor, 0.005f, 100000.f);
        camera->xBounds = (Vec2){centerX - halfWidth, centerX + halfWidth};
        camera->yBounds = (Vec2){centerY - halfHeight, centerY + halfHeight};
    }
}

static bool FrameCameraOnModel(Camera *camera, const Model *model) {
    int vertexCount = arrlen(model->vertices);
    if (vertexCount == 0) return false;

    Mat4 translation = M4Identity();
    translation.s03 = model->position.x;
    translation.s13 = model->position.y;
    translation.s23 = model->position.z;
    Mat4 rotationX = M4Eq(model->Rx, (Mat4){0}) ? M4Identity() : model->Rx;
    Mat4 rotationY = M4Eq(model->Ry, (Mat4){0}) ? M4Identity() : model->Ry;
    Mat4 rotationZ = M4Eq(model->Rz, (Mat4){0}) ? M4Identity() : model->Rz;
    Mat4 modelToWorld = M4MulM4(translation,
        M4MulM4(rotationZ,
        M4MulM4(rotationY,
        M4MulM4(rotationX, M4ScaleV3(model->scale)))));

    Vec3 first = M4MulV3(modelToWorld, model->vertices[0]);
    Vec3 minimum = first;
    Vec3 maximum = first;
    for (int i = 1; i < vertexCount; ++i) {
        Vec3 vertex = M4MulV3(modelToWorld, model->vertices[i]);
        minimum.x = MIN(minimum.x, vertex.x);
        minimum.y = MIN(minimum.y, vertex.y);
        minimum.z = MIN(minimum.z, vertex.z);
        maximum.x = MAX(maximum.x, vertex.x);
        maximum.y = MAX(maximum.y, vertex.y);
        maximum.z = MAX(maximum.z, vertex.z);
    }

    Vec3 center = V3MulS(V3Add(minimum, maximum), 0.5f);
    float radius = 0.f;
    for (int i = 0; i < vertexCount; ++i) {
        Vec3 vertex = M4MulV3(modelToWorld, model->vertices[i]);
        radius = MAX(radius, V3Length(V3Sub(vertex, center)));
    }
    radius = MAX(radius, 0.01f);

    Vec3 forward = V3Normalize(V3Sub(camera->Target, camera->Position));
    if (V3Length(forward) < EPSILON) forward = (Vec3){0.f, 0.f, -1.f};

    float distance;
    if (camera->projectionType == PERSPECTIVE) {
        float fov = CLAMP(camera->fov, 0.0873f, 2.9671f);
        distance = radius * 1.2f / sinf(fov * 0.5f);
    } else {
        float halfExtent = radius * 1.2f;
        camera->xBounds = (Vec2){-halfExtent, halfExtent};
        camera->yBounds = (Vec2){-halfExtent, halfExtent};
        distance = radius * 2.f;
    }

    camera->Target = center;
    camera->Position = V3Sub(center, V3MulS(forward, distance));
    camera->zBounds.y = MIN(camera->zBounds.y > 0.f ? camera->zBounds.y : 0.1f,
                            MAX(0.005f, (distance - radius) * 0.5f));
    camera->zBounds.x = MAX(camera->zBounds.x, distance + radius * 2.f);
    return true;
}

static void DrawCameraOrientationGizmo(Camera *camera, int x, int y) {
    Vec3 forward = V3Normalize(V3Sub(camera->Target, camera->Position));
    Vec3 right = V3Normalize(V3Cross(forward, camera->up));
    if (V3Length(right) < EPSILON) return;
    Vec3 up = V3Normalize(V3Cross(right, forward));

    DrawCircle(x, y, 30.f, (Color){20, 24, 32, 220});
    DrawCircleLines(x, y, 30.f, (Color){110, 120, 132, 255});

    const Vec3 axes[] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};
    const char *labels[] = {"X", "Y", "Z"};
    const Color colors[] = {
        (Color){235, 90, 90, 255},
        (Color){100, 220, 130, 255},
        (Color){100, 160, 255, 255}
    };
    for (int i = 0; i < 3; ++i) {
        float screenX = V3Dot(axes[i], right);
        float screenY = -V3Dot(axes[i], up);
        Vector2 end = {(float)x + screenX * 22.f, (float)y + screenY * 22.f};
        DrawLineEx((Vector2){(float)x, (float)y}, end, 2.f, colors[i]);
        DrawCircle((int)end.x, (int)end.y, 3.f, colors[i]);
        DrawText(labels[i], (int)end.x + 3, (int)end.y - 5, 10, colors[i]);
    }
}

static void Tooltip(const char *label, int x, int y) {
    GuiSetTooltip(label);
    GuiEnableTooltip();
    int width = GuiGetTextWidth(label);
    int height = GuiGetStyle(DEFAULT, TEXT_SIZE);
    int textAlign = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL);
    
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_CENTER);
    GuiTooltip((Rectangle) { x, y, width, height });
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, textAlign);
    GuiDisableTooltip();
}
// Helper functions end

//
// Handling controls and creating the GUI
//
void CreateInterface(RenderSettings *settings, Scene *scene) {
    static bool showInterface = true;
    
    // File dialog should lock all other UI elements if it's active, so we need to initialize it first
    static GuiWindowFileDialogState fileDialogState = {0};
    static FileDialogOperaion fileDialogOperation = -1;
    
    static Vec4 *colorPickerValue = NULL;
    static int colorPickerX = 0, colorPickerY = 0;
    
    static int activeBoxId = -1;

    // What WASD/arrows/mouse-drag control. Only one at a time; camera by default.
    static enum { KB_NONE, KB_MODEL, KB_CAMERA } kbTarget = KB_CAMERA;
    int boxCounter = 0;
    bool activateBox = false;
    
    if (fileDialogState.windowActive || colorPickerValue != NULL) GuiLock();
    if (!GuiIsLocked()) { // General keyboard controls
        if (IsKeyPressed(KEY_TAB)) {
            activateBox = true;
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                activeBoxId -= 1;
            } else {
                activeBoxId += 1;
            }
        }
        
        if (IsKeyPressed(KEY_ESCAPE)) {
            activeBoxId = -1;
            activateBox = false;
        }
        
        if (activeBoxId == -1) {
            // Not currently editing a UI box, handle scene related key bindings
            if (IsKeyPressed(KEY_GRAVE)) {
                showInterface = !showInterface;
            }
            if (IsKeyPressed(KEY_F) &&
                0 <= scene->activeCameraIndex &&
                scene->activeCameraIndex < arrlen(scene->cameras) &&
                0 <= scene->activeModelIndex &&
                scene->activeModelIndex < arrlen(scene->models)) {
                FrameCameraOnModel(&scene->cameras[scene->activeCameraIndex],
                                   &scene->models[scene->activeModelIndex]);
            }
        } else {
            GuiLock();
        }
    }
    
    // See raygui.h, specifically GuiIconName enum to see what icons are available
    const char *label = GuiIconText(showInterface ? ICON_EYE_ON : ICON_EYE_OFF, NULL);
    GuiToggle((Rectangle) {0, 0, 20, 20}, label, &showInterface);

    if (!showInterface) return;
    
    const char *tooltip = NULL;
    int margin = 12;
    int uiLeft = margin, uiTop = margin;
    int uiWidth = 400, uiHeight = settings->height - 2*margin;
    int iconButtonSize = 24;
    Rectangle uiRect = (Rectangle) { uiLeft, uiTop, uiWidth, uiHeight };
    {
        Color background = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
        background.a = 200;
        DrawRectangleRec(uiRect, background);
        GuiGroupBox(uiRect, "Controls");
        uiLeft += margin; uiTop += margin;
    }
    
    
    //
    // Start the actual GUI
    //
    {
        float left = uiLeft;
        {
            const char *label = GuiIconText(scene->advanceTime ? ICON_PLAYER_PAUSE : ICON_PLAYER_PLAY, NULL);
            Rectangle rect = (Rectangle) { left, uiTop, iconButtonSize, iconButtonSize };
            left += iconButtonSize + 2;
            
            if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                tooltip = "Advance time";
            }
            
            if (GuiButton(rect, label)) {
                scene->advanceTime = !scene->advanceTime;
            }
        }
        {
            const char *label = NULL;
            switch (settings->renderingMethod) {
                case RENDERING_METHOD_SOFTWARE:   { label = GuiIconText(ICON_PENCIL_BIG, NULL); } break;
                case RENDERING_METHOD_OPENGL:     { label = GuiIconText(ICON_CPU, NULL);        } break;
                case RENDERING_METHOD_RAYTRACING: { label = GuiIconText(ICON_LASER, NULL);      } break;
                case RENDERING_METHOD_COUNT: { /* this case is here only to disable a compiler warning */ } break;
            }
            Rectangle rect = (Rectangle) { left, uiTop, iconButtonSize, iconButtonSize };
            left += iconButtonSize + 2;
            
            if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                switch (settings->renderingMethod) {
                    case RENDERING_METHOD_SOFTWARE:   { tooltip = "Switch to OpenGL rendering";   } break;
                    case RENDERING_METHOD_OPENGL:     { tooltip = "Switch to raytracing";         } break;
                    case RENDERING_METHOD_RAYTRACING: { tooltip = "Switch to software rendering"; } break;
                    case RENDERING_METHOD_COUNT: { /* this case is here only to disable a compiler warning */ } break;
                }
            }
            
            if (GuiButton(rect, label)) {
                settings->renderingMethod += 1;
                settings->renderingMethod %= RENDERING_METHOD_COUNT;
            }
        }
        {
            Rectangle rect = (Rectangle) { left, uiTop, iconButtonSize, iconButtonSize };
            left += iconButtonSize + 2;
            
            if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                tooltip = "Background tint";
            }
            
            if (GuiButton(rect, GuiIconText(ICON_COLOR_BUCKET, NULL))) {
                colorPickerValue = &settings->tint;
                colorPickerX = left + 8;
                colorPickerY = uiTop + 8;
            }
            GuiDrawRectangle(rect, 3, ColorFromV4(settings->tint), BLANK);
        }
        {
            Rectangle rect = (Rectangle) { left, uiTop, iconButtonSize, iconButtonSize };
            left += iconButtonSize + 2;
            
            if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                tooltip = settings->showDebugLayer ? "Hide debug layer" : "Show debug layer";
            }
            
            GuiToggle(rect, GuiIconText(ICON_LAYERS2, NULL), &settings->showDebugLayer);
        }
        {
            Rectangle rect = (Rectangle) { left, uiTop, iconButtonSize, iconButtonSize };
            left += iconButtonSize + 2;

            if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                tooltip = settings->showCameraWidgets ? "Hide camera widgets" : "Show camera widgets";
            }

            GuiToggle(rect, GuiIconText(ICON_CAMERA, NULL), &settings->showCameraWidgets);
        }

        BOX_FLOAT(&scene->step, &left, uiTop + 4, "Set the step size when moving with WASD");
        BOX_INT(&settings->problemX, &left, uiTop + 4, "Problem Pixel X");
        BOX_INT(&settings->problemY, &left, uiTop + 4, "Problem Pixel Y");
    }
    uiTop += iconButtonSize + margin;
    
    static bool showModelControls = true;
    {
        Rectangle rect = (Rectangle) { uiLeft - margin/2, uiTop + margin/2, margin, margin };
        GuiCheckBox(rect,  NULL, &showModelControls);
        
        GuiLine((Rectangle) { uiLeft + margin, uiTop + margin, uiWidth - margin*2, 0 }, "Model Controls");
        uiTop += 2*margin;
    }
    if (showModelControls) {
        {
            float left = uiLeft + margin;
            int right = uiRect.x + uiRect.width - margin;
            {
                const char *label = GuiIconText(ICON_FOLDER_FILE_OPEN, "Open OBJ");
                Rectangle rect = (Rectangle) { left, uiTop, margin + GuiGetTextWidth(label), 2*margin };
                left += rect.width + 2;
                
                if (GuiButton(rect, label)) {
                    const char *modelDir = TextFormat("%s" PATH_SEPERATOR "models", GetApplicationDirectory());
                    fileDialogState = InitGuiWindowFileDialog(modelDir);
                    fileDialogState.windowActive = true;
                    strcpy(fileDialogState.filterExt, "obj");
                    
                    fileDialogOperation = FILE_DIALOG_LOAD_MODEL;
                }
            }
            {
                Rectangle rect = (Rectangle) { right - iconButtonSize, uiTop, iconButtonSize, iconButtonSize };
                
                if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                    tooltip = "Control active model with keyboard";
                }
                
                bool value = (kbTarget == KB_MODEL);
                GuiToggle(rect, GuiIconText(ICON_PLAYER, NULL), &value);
                if (value) kbTarget = KB_MODEL;                    // turning it on takes control from the camera
                else if (kbTarget == KB_MODEL) kbTarget = KB_NONE; // turning it off releases control
                static bool moveModelRelativeToCamera = false;
                rect = (Rectangle){right - iconButtonSize - 66, uiTop, 64, iconButtonSize};
                if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                    tooltip = "Choose world or active-camera-relative keyboard movement";
                }
                GuiToggle(rect, moveModelRelativeToCamera ? "Camera" : "World",
                          &moveModelRelativeToCamera);
                // TODO(student)DONE WSAD movement for the active model
                if (value && !GuiIsLocked() &&
                    0 <= scene->activeModelIndex &&
                    scene->activeModelIndex < arrlen(scene->models))
                {
                    Model *m = &scene->models[scene->activeModelIndex];
                    Vec3 movement = {0};
                    if (IsKeyDown(KEY_W)) movement.z += scene->step;
                    if (IsKeyDown(KEY_S)) movement.z -= scene->step;
                    if (IsKeyDown(KEY_A)) movement.x -= scene->step;
                    if (IsKeyDown(KEY_D)) movement.x += scene->step;
                    if (IsKeyDown(KEY_Q)) movement.y += scene->step;
                    if (IsKeyDown(KEY_E)) movement.y -= scene->step;

                    if (moveModelRelativeToCamera &&
                        0 <= scene->activeCameraIndex &&
                        scene->activeCameraIndex < arrlen(scene->cameras)) {
                        movement = CameraRelativeMovement(
                            &scene->cameras[scene->activeCameraIndex], movement);
                    }
                    m->position = V3Add(m->position, movement);

                    // Left mouse drag rotates ONLY the active model. Starts only if the click began
                    // outside the control panel (and the eye button), same as the camera rotation.
                    static bool modelMouseRotating = false;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        Vector2 mouse = GetMousePosition();
                        modelMouseRotating = !CheckCollisionPointRec(mouse, uiRect) &&
                                             !CheckCollisionPointRec(mouse, (Rectangle){0, 0, 20, 20});
                    }
                    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) modelMouseRotating = false;
                    if (modelMouseRotating) {
                        // Rotate around the active camera's up/right axes so the drag matches what you see
                        // (falls back to world Y/X if there's no active camera)
                        Vec3 up = {0.f, 1.f, 0.f}, right = {1.f, 0.f, 0.f};
                        if (0 <= scene->activeCameraIndex && scene->activeCameraIndex < arrlen(scene->cameras)) {
                            Camera *cam = &scene->cameras[scene->activeCameraIndex];
                            Vec3 fwd = V3Normalize(V3Sub(cam->Target, cam->Position));
                            right = V3Normalize(V3Cross(fwd, cam->up));
                            up    = V3Normalize(V3Cross(right, fwd));
                        }
                        Vector2 d = GetMouseDelta();
                        float sens = 0.01f; // radians per pixel
                        // Rz is the outermost factor of T*Rz*Ry*Rx, so pre-multiplying it applies a
                        // world-space rotation to the whole model (around its own position)
                        RotateModelAxis(&m->Rz, up,    d.x * sens); // drag right -> turns right
                        RotateModelAxis(&m->Rz, right, d.y * sens); // drag down  -> tilts down
                    }
                }
            }
            
            // Primitives
            {
                Rectangle rect = (Rectangle) { left, uiTop, 2*margin, 2*margin };
                left += rect.width + 2;
                
                if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                    tooltip = "Add Cube";
                }
                if (GuiButton(rect, GuiIconText(ICON_CUBE, NULL))) {
                    // TODO(student)DONE
                    Model primitive = MakePrimitiveCube();
                    AddModel(scene, primitive);
                }
            }
            // Other primitives
            {
                Rectangle rect = (Rectangle){left, uiTop, 2 * margin, 2 * margin};
                left += rect.width + 2;

                if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect))
                {
                    tooltip = "Add Sphere";
                }
                if (GuiButton(rect, GuiIconText(ICON_SPHERE, NULL)))
                {
                    // TODO(student)DONE
                    Model primitive = MakePrimitiveSphere();
                    AddModel(scene, primitive);
                }
            }
            {
                Rectangle rect = (Rectangle){left, uiTop, 2 * margin, 2 * margin};
                left += rect.width + 2;

                if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect))
                {
                    tooltip = "Add Triangle";
                }
                if (GuiButton(rect, "Tri"))
                {
                    // TODO(student)DONE
                    Model primitive = MakePrimitiveTriangle();
                    AddModel(scene, primitive);
                }
            }
            {
                Rectangle rect = (Rectangle){left, uiTop, 2 * margin, 2 * margin};
                left += rect.width + 2;

                if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect))
                {
                    tooltip = "Add Pyramid";
                }
                if (GuiButton(rect, "Pyr"))
                {
                    // TODO(student)DONE
                    Model primitive = MakePrimitive4SidedPyramid();
                    AddModel(scene, primitive);
                }
            }
        }
        uiTop += 2*margin + margin;
        
        { // Transform controls for the active model
            Model dummyModel = {0};
            Model *activeModel = NULL;
            static bool lock = false;
            
            if (0 < arrlen(scene->models) && 0 <= scene->activeModelIndex && scene->activeModelIndex < arrlen(scene->models)) {
                activeModel = &scene->models[scene->activeModelIndex];
                lock = false;
            } else {
                // If there's no active model selected, we want to draw the same UI but locked
                // so we have a dummy model to prevent illegal pointer access when drawing the GUI
                activeModel = &dummyModel;
                
                // We want to lock only if the gui is not already locked so we don't unlock it later
                // if it was intended to stay locked (for example if the file dialog is open)
                lock = !GuiIsLocked();
            }
            if (lock) GuiLock();
            
            float rowHeight = 16;
            float controlTop = uiTop;
            
            float left = uiLeft + margin;
            {
                const char *label = "position: ";
                Rectangle rect = (Rectangle) { left, uiTop, 100, 16 };
                GuiDrawText(label, rect, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT)));
                left += rect.width + 2;
                
                BOX_FLOAT(&activeModel->position.x, &left, uiTop, "Drag vertically to edit the X position");
                BOX_FLOAT(&activeModel->position.y, &left, uiTop, "Drag vertically to edit the Y position");
                BOX_FLOAT(&activeModel->position.z, &left, uiTop, "Drag vertically to edit the Z position");
            }
            uiTop += rowHeight + 2;
            {
                const char *label = "scale: ";
                left = uiLeft + margin;
                Rectangle rect = (Rectangle) { left, uiTop, 100, 16 };
                GuiDrawText(label, rect, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT)));
                left += rect.width + 2;

                BOX_FLOAT(&activeModel->scale.x, &left, uiTop, "Drag vertically to scale along X");
                BOX_FLOAT(&activeModel->scale.y, &left, uiTop, "Drag vertically to scale along Y");
                BOX_FLOAT(&activeModel->scale.z, &left, uiTop, "Drag vertically to scale along Z");
                // Never 0 (or negative): that would flatten the model and make the normal matrix singular
                activeModel->scale.x = MAX(activeModel->scale.x, 0.01f);
                activeModel->scale.y = MAX(activeModel->scale.y, 0.01f);
                activeModel->scale.z = MAX(activeModel->scale.z, 0.01f);
            }
            uiTop += rowHeight + 2;
            // TODO(student) ...DONE ROTATION
            {
                static float rotationDegrees = 90.f;
                const char *label = "Rotate (deg):";
                left = uiLeft + margin;
                Rectangle labelRect = (Rectangle) { left, uiTop, 88, 16 };
                GuiDrawText(label, labelRect, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT)));
                left += labelRect.width + 2;
                BOX_FLOAT(&rotationDegrees, &left, uiTop, "Rotation angle in degrees");
                float angle = rotationDegrees * 0.01745329252f;

                Rectangle rect = (Rectangle) { left, uiTop, 24, 16 };
                if (GuiButton(rect, "X+")) RotateModelAxis(&activeModel->Rx, (Vec3){1.f, 0.f, 0.f}, angle);
                left += rect.width + 1;
                rect.x = left;
                if (GuiButton(rect, "X-")) RotateModelAxis(&activeModel->Rx, (Vec3){1.f, 0.f, 0.f}, -angle);
                left += rect.width + 1;

                rect.x = left;
                if (GuiButton(rect, "Y+")) RotateModelAxis(&activeModel->Ry, (Vec3){0.f, 1.f, 0.f}, angle);
                left += rect.width + 1;
                rect.x = left;
                if (GuiButton(rect, "Y-")) RotateModelAxis(&activeModel->Ry, (Vec3){0.f, 1.f, 0.f}, -angle);
                left += rect.width + 1;

                rect.x = left;
                if (GuiButton(rect, "Z+")) RotateModelAxis(&activeModel->Rz, (Vec3){0.f, 0.f, 1.f}, angle);
                left += rect.width + 1;
                rect.x = left;
                if (GuiButton(rect, "Z-")) RotateModelAxis(&activeModel->Rz, (Vec3){0.f, 0.f, 1.f}, -angle);
            }
            uiTop += rowHeight;
            
            if (activeModel == &dummyModel) {
                Rectangle rect = (Rectangle) {uiLeft - margin, controlTop - margin/2, uiWidth, uiTop - controlTop + margin};
                int textAlignStyle = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL);
                
                GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
                
                Color backcolor = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_DISABLED));
                backcolor.a = 230;
                GuiDrawRectangle(rect, 2,
                                 GetColor(GuiGetStyle(BORDER, BORDER_COLOR_DISABLED)),
                                 backcolor);
                GuiDrawText("No model selected", rect, TEXT_ALIGN_CENTER,
                            GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
                
                GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, textAlignStyle);
            }
            uiTop += margin;
            
            if (lock) GuiUnlock();
        }
        
        { // Model list controls
            int rowHeight = 2*margin;
            int itemCount = 6;
            int modelCount = arrlen(scene->models);

            static Rectangle modelPanelView = { 0 };
            Rectangle scrollRect = (Rectangle) { uiLeft - 1, uiTop - 1, uiWidth - 2*margin + 2, itemCount*(rowHeight + 2) };
            Rectangle contentRect = scrollRect;
            contentRect.x += 1;
            contentRect.y += 1;
            int scrollBarWidth = GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) + 2*GuiGetStyle(DEFAULT, BORDER_WIDTH);
            contentRect.width -= scrollBarWidth + 2;
            contentRect.height = modelCount * (rowHeight + 2);
            static Vector2 panelScroll = { 0, 0 };
            GuiScrollPanel(scrollRect, NULL, contentRect, &panelScroll, &modelPanelView);
            int scrollTop = contentRect.y + panelScroll.y;
            
            if (contentRect.height < scrollRect.height) {
                GuiDrawRectangle((Rectangle) {scrollRect.x + scrollRect.width - scrollBarWidth, scrollRect.y + 1, scrollBarWidth - 1, scrollRect.height - 2 }, 0, WHITE, GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)));
            }
            
            uiTop += scrollRect.height + 2;
            bool lock = !CheckCollisionPointRec(GetMousePosition(), scrollRect) && !GuiIsLocked();
            if (lock) GuiLock();
            
            BeginScissorMode(modelPanelView.x, modelPanelView.y, modelPanelView.width, modelPanelView.height);
            
            for (int modelIndex = 0; modelIndex < modelCount; ++modelIndex) {
                Model *model = &scene->models[modelIndex];
                
                int left  = contentRect.x;
                int right = contentRect.x + contentRect.width;
                
                {
                    Rectangle rect = (Rectangle) { left, scrollTop, 2*margin, 2*margin };
                    left += rect.width + 2;
                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                        tooltip = "Hide/Show Model";
                    }
                    
                    GuiToggle(rect, GuiIconText(model->hidden ? ICON_EYE_OFF : ICON_EYE_ON, NULL), &model->hidden);
                }
                {
                    Rectangle rect = (Rectangle) { right - 2*margin, scrollTop, 2*margin, 2*margin };
                    right -= rect.width + 2;
                    
                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                        tooltip = "Delete Model";
                    }
                    
                    if (GuiButton(rect, GuiIconText(ICON_BIN, NULL))) {
                        RemoveModel(scene, modelIndex);
                        if (scene->activeModelIndex >= modelIndex) scene->activeModelIndex -= 1;
                        --modelCount;
                        // Stop here: the next model's delete button would sit at the same spot and also
                        // fire this frame (the mouse release is still active), deleting everything.
                        break;
                    }
                }
                // TODO(student)DONE Add buttons to show/hide bounding box and normals for each model
                {
                    Rectangle rect = (Rectangle) { right - 2*margin, scrollTop, 2*margin, 2*margin };
                    right -= rect.width + 2;

                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                        tooltip = "Show/Hide Bounding Box";
                    }

                    GuiToggle(rect, GuiIconText(ICON_BOX, NULL), &model->showBoundingBox);
                }
                {
                    Rectangle rect = (Rectangle) { right - 2*margin, scrollTop, 2*margin, 2*margin };
                    right -= rect.width + 2;

                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                        tooltip = "Show/Hide Face Normals";
                    }

                    GuiToggle(rect, GuiIconText(ICON_ARROW_UP, NULL), &model->showFaceNormals);
                }
                {
                    Rectangle rect = (Rectangle) { right - 2*margin, scrollTop, 2*margin, 2*margin };
                    right -= rect.width + 2;

                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                        tooltip = "Show/Hide Vertex Normals";
                    }

                    GuiToggle(rect, GuiIconText(ICON_ARROW_UP_FILL, NULL), &model->showVertexNormals);
                }
                
                {
                    const char *label = TextFormat(" Model %3d", modelIndex);
                    Rectangle rect = (Rectangle) { left, scrollTop, right-left, 2*margin };
                    
                    bool hovered = false;
                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            if (scene->activeModelIndex != modelIndex) {
                                scene->activeModelIndex = modelIndex;
                            } else {
                                scene->activeModelIndex = -1;
                            }
                        }
                        
                        hovered = true;
                    }
                    
                    bool active = modelIndex == scene->activeModelIndex;
                    
                    Color backColor = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
                    Color textColor = GetColor(GuiGetStyle(DEFAULT, TEXT));
                    int border = 0;
                    if (active) {
                        backColor = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED));
                        textColor = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED));
                    } else if (hovered) {
                        backColor = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
                        textColor = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
                    }
                    if (hovered) border = 2;
                    
                    GuiDrawRectangle(rect, border, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED)), backColor);
                    GuiDrawText(label, rect, TEXT_ALIGN_LEFT, textColor);
                }
                
                scrollTop += rowHeight + 2;
            }
            EndScissorMode();
            
            if (lock) GuiUnlock();
        }
    }
    // Camera section header (drawn after the model section so they don't overlap)
    static bool showCameraControls = true;
    {
        uiTop += margin; // spacing after the model section
        Rectangle rect = (Rectangle){uiLeft - margin / 2, uiTop + margin / 2, margin, margin};
        GuiCheckBox(rect, NULL, &showCameraControls);

        GuiLine((Rectangle){uiLeft + margin, uiTop + margin, uiWidth - margin * 2, 0}, "Camera Controls");
        uiTop += 2 * margin;
    }

    if (!GuiIsLocked() &&
        0 <= scene->activeCameraIndex &&
        scene->activeCameraIndex < arrlen(scene->cameras)) {
        Camera *camera = &scene->cameras[scene->activeCameraIndex];
        static bool mouseOrbiting = false;
        static bool mousePanning = false;
        Vector2 mouse = GetMousePosition();
        bool mouseOverViewport = !CheckCollisionPointRec(mouse, uiRect) &&
                                 !CheckCollisionPointRec(mouse, (Rectangle){0, 0, 20, 20});
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mouseOrbiting = mouseOverViewport && kbTarget != KB_MODEL;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
            IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            mousePanning = mouseOverViewport;
        }
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) mouseOrbiting = false;
        if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT) &&
            !IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) mousePanning = false;

        if (mouseOrbiting && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 d = GetMouseDelta();
            float sens = 0.005f;
            OrbitCamera(camera, d.x * sens, -d.y * sens);
        }
        if (mousePanning && (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ||
                             IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))) {
            Vector2 d = GetMouseDelta();
            PanCamera(camera, d.x, d.y, GetScreenHeight());
        }
        if (mouseOverViewport) {
            ZoomCamera(camera, GetMouseWheelMove());
        }
    }

    if (showCameraControls)
    {

        {
            int right = uiRect.x + uiRect.width - margin;
            Rectangle rect = (Rectangle){right - iconButtonSize, uiTop, iconButtonSize, iconButtonSize};

            if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                tooltip = "Control active camera with keyboard";
            }

            bool controlActiveCameraWithKeyboard = (kbTarget == KB_CAMERA);
            GuiToggle(rect, GuiIconText(ICON_PLAYER, NULL), &controlActiveCameraWithKeyboard);
            if (controlActiveCameraWithKeyboard) kbTarget = KB_CAMERA;  // turning it on takes control from the model
            else if (kbTarget == KB_CAMERA) kbTarget = KB_NONE;          // turning it off releases control
            static bool moveCameraRelativeToView = false;
            rect = (Rectangle){right - iconButtonSize - 66, uiTop, 64, iconButtonSize};
            if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                tooltip = "Choose world or view-relative keyboard movement";
            }
            GuiToggle(rect, moveCameraRelativeToView ? "View" : "World",
                      &moveCameraRelativeToView);

            if (!GuiIsLocked() &&
                0 <= scene->activeCameraIndex &&
                scene->activeCameraIndex < arrlen(scene->cameras)) {
                Camera *camera = &scene->cameras[scene->activeCameraIndex];
                if (controlActiveCameraWithKeyboard) {
                    Vec3 movement = {0};
                    if (IsKeyDown(KEY_W)) movement.z += scene->step;
                    if (IsKeyDown(KEY_S)) movement.z -= scene->step;
                    if (IsKeyDown(KEY_A)) movement.x -= scene->step;
                    if (IsKeyDown(KEY_D)) movement.x += scene->step;
                    if (IsKeyDown(KEY_Q)) movement.y += scene->step;
                    if (IsKeyDown(KEY_E)) movement.y -= scene->step;
                    if (moveCameraRelativeToView) {
                        movement = CameraRelativeMovement(camera, movement);
                    }
                    camera->Position = V3Add(camera->Position, movement);
                    camera->Target = V3Add(camera->Target, movement);

                    // Rotation around the camera's own axes: yaw around 'up', pitch around 'right'
                    Vec3 fwd   = V3Normalize(V3Sub(camera->Target, camera->Position));
                    Vec3 right = V3Normalize(V3Cross(fwd, camera->up));

                    // Arrow keys
                    float rot = 0.03f; // radians per frame (~1.8 deg)
                    if (IsKeyDown(KEY_LEFT))  RotateCameraView(camera, camera->up,  rot);
                    if (IsKeyDown(KEY_RIGHT)) RotateCameraView(camera, camera->up, -rot);
                    if (IsKeyDown(KEY_UP))    RotateCameraView(camera, right,  rot);
                    if (IsKeyDown(KEY_DOWN))  RotateCameraView(camera, right, -rot);
                }

            }
        }

        { // Transform controls for the active camera
            Camera dummyCamera = {0};
            Camera *activeCamera = NULL;
            static bool lock = false;

            if (0 < arrlen(scene->cameras) && 0 <= scene->activeCameraIndex && scene->activeCameraIndex < arrlen(scene->cameras))
            {
                activeCamera = &scene->cameras[scene->activeCameraIndex];
                lock = false;
            }
            else
            {
                // If there's no active model selected, we want to draw the same UI but locked
                // so we have a dummy model to prevent illegal pointer access when drawing the GUI
                activeCamera = &dummyCamera;

                // We want to lock only if the gui is not already locked so we don't unlock it later
                // if it was intended to stay locked (for example if the file dialog is open)
                lock = !GuiIsLocked();
            }
            if (lock)
                GuiLock();

            float rowHeight = 16;
            float controlTop = uiTop;

            float left = uiLeft + margin;
            {
                const char *label = "position: ";
                Rectangle rect = (Rectangle){left, uiTop, 100, 16};
                GuiDrawText(label, rect, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT)));
                left += rect.width + 2;

                BOX_FLOAT(&activeCamera->Position.x, &left, uiTop, "Drag vertically to edit the X position");
                BOX_FLOAT(&activeCamera->Position.y, &left, uiTop, "Drag vertically to edit the Y position");
                BOX_FLOAT(&activeCamera->Position.z, &left, uiTop, "Drag vertically to edit the Z position");
            }
            {
                const char *label = "target: ";
                Rectangle rect = (Rectangle){uiLeft + margin, uiTop + rowHeight, 100, 16};
                GuiDrawText(label, rect, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT)));

                float targetLeft = rect.x + rect.width + 2;
                BOX_FLOAT(&activeCamera->Target.x, &targetLeft, uiTop + rowHeight, NULL);
                BOX_FLOAT(&activeCamera->Target.y, &targetLeft, uiTop + rowHeight, NULL);
                BOX_FLOAT(&activeCamera->Target.z, &targetLeft, uiTop + rowHeight, NULL);
            }
            uiTop += rowHeight + rowHeight;
            {
                Rectangle rect = (Rectangle){uiLeft + margin, uiTop + rowHeight, 100, 16};
                {
                    int projectionRows;

                    if (activeCamera->projectionType == PERSPECTIVE)
                    {
                        Rectangle rect = (Rectangle){uiLeft + margin, uiTop, 100, 16};
                        GuiDrawText("FOV (radians):", rect, TEXT_ALIGN_LEFT,
                                    GetColor(GuiGetStyle(DEFAULT, TEXT)));

                        float fieldLeft = rect.x + rect.width + 2;
                        BOX_FLOAT(&activeCamera->fov, &fieldLeft, uiTop,
                                  "Field of view in radians");

                        projectionRows = 1;
                    }
                    else
                    {
                        Rectangle rect = (Rectangle){uiLeft + margin, uiTop, 100, 16};
                        float fieldLeft = rect.x + rect.width + 2;

                        GuiDrawText("X left/right:", rect, TEXT_ALIGN_LEFT,
                                    GetColor(GuiGetStyle(DEFAULT, TEXT)));
                        BOX_FLOAT(&activeCamera->xBounds.x, &fieldLeft, uiTop, "Left bound");
                        BOX_FLOAT(&activeCamera->xBounds.y, &fieldLeft, uiTop, "Right bound");

                        rect.y += rowHeight;
                        fieldLeft = rect.x + rect.width + 2;
                        GuiDrawText("Y bottom/top:", rect, TEXT_ALIGN_LEFT,
                                    GetColor(GuiGetStyle(DEFAULT, TEXT)));
                        BOX_FLOAT(&activeCamera->yBounds.x, &fieldLeft, uiTop + rowHeight, "Bottom bound");
                        BOX_FLOAT(&activeCamera->yBounds.y, &fieldLeft, uiTop + rowHeight, "Top bound");

                        rect.y += rowHeight;
                        fieldLeft = rect.x + rect.width + 2;
                        GuiDrawText("Z far/near:", rect, TEXT_ALIGN_LEFT,
                                    GetColor(GuiGetStyle(DEFAULT, TEXT)));
                        BOX_FLOAT(&activeCamera->zBounds.x, &fieldLeft, uiTop + 2 * rowHeight, "Far plane");
                        BOX_FLOAT(&activeCamera->zBounds.y, &fieldLeft, uiTop + 2 * rowHeight, "Near plane");

                        projectionRows = 3;
                    }

                    uiTop += projectionRows * rowHeight;
                }
            }

                // TODO(student) ...DONE ROTATION
                {
                    static float rotationDegrees = 90.f;
                    const char *label = "Rotate (deg):";
                    left = uiLeft + margin;
                    Rectangle labelRect = (Rectangle){left, uiTop, 88, 16};
                    GuiDrawText(label, labelRect, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT)));
                    left += labelRect.width + 2;
                    BOX_FLOAT(&rotationDegrees, &left, uiTop, "Rotation angle in degrees");
                    float angle = rotationDegrees * 0.01745329252f;

                    Rectangle rect = (Rectangle){left, uiTop, 24, 16};
                    if (GuiButton(rect, "X+"))
                        RotateCameraView(activeCamera, (Vec3){1.f, 0.f, 0.f}, angle);
                    left += rect.width + 1;
                    rect.x = left;
                    if (GuiButton(rect, "X-"))
                        RotateCameraView(activeCamera, (Vec3){1.f, 0.f, 0.f}, -angle);
                    left += rect.width + 1;

                    rect.x = left;
                    if (GuiButton(rect, "Y+"))
                        RotateCameraView(activeCamera, (Vec3){0.f, 1.f, 0.f}, angle);
                    left += rect.width + 1;
                    rect.x = left;
                    if (GuiButton(rect, "Y-"))
                        RotateCameraView(activeCamera, (Vec3){0.f, 1.f, 0.f}, -angle);
                    left += rect.width + 1;

                    rect.x = left;
                    if (GuiButton(rect, "Z+"))
                        RotateCameraView(activeCamera, (Vec3){0.f, 0.f, 1.f}, angle);
                    left += rect.width + 1;
                    rect.x = left;
                    if (GuiButton(rect, "Z-"))
                        RotateCameraView(activeCamera, (Vec3){0.f, 0.f, 1.f}, -angle);
                    left += rect.width + 3;

                    rect = (Rectangle){left, uiTop, 88, 16};
                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect)) {
                        tooltip = "Frame the selected model (F)";
                    }
                    bool hasActiveModel = 0 <= scene->activeModelIndex &&
                                         scene->activeModelIndex < arrlen(scene->models);
                    if (GuiButton(rect, "Frame Model") && hasActiveModel) {
                        FrameCameraOnModel(activeCamera, &scene->models[scene->activeModelIndex]);
                    }
                }
            uiTop += rowHeight;

            if (activeCamera == &dummyCamera)
            {
                Rectangle rect = (Rectangle){uiLeft - margin, controlTop - margin / 2, uiWidth, uiTop - controlTop + margin};
                int textAlignStyle = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL);

                GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);

                Color backcolor = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_DISABLED));
                backcolor.a = 230;
                GuiDrawRectangle(rect, 2,
                                 GetColor(GuiGetStyle(BORDER, BORDER_COLOR_DISABLED)),
                                 backcolor);
                GuiDrawText("No camera selected", rect, TEXT_ALIGN_CENTER,
                            GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));

                GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, textAlignStyle);
            }
            uiTop += margin;

            if (lock)
                GuiUnlock();
        }

        { // Camera list controls
            int rowHeight = 2 * margin;
            int itemCount = 6;
            int cameraCount = arrlen(scene->cameras);

            { // Always-visible "Add Camera" button
                Rectangle rect = (Rectangle){uiLeft + margin, uiTop, 100, 2 * margin};
                if (GuiButton(rect, "Add Camera")) {
                    bool hasActive = 0 <= scene->activeCameraIndex && scene->activeCameraIndex < cameraCount;
                    Camera camera;
                    if (hasActive) {
                        // Copy the active camera and shift it 2 units to its right, so its widget is visible
                        camera = scene->cameras[scene->activeCameraIndex];
                        Vec3 fwd   = V3Normalize(V3Sub(camera.Target, camera.Position));
                        Vec3 right = V3Normalize(V3Cross(fwd, camera.up));
                        Vec3 offset = V3MulS(right, 2.f);
                        camera.Position = V3Add(camera.Position, offset);
                        camera.Target   = V3Add(camera.Target, offset);
                        camera.hidden = false;
                    } else {
                        camera = DefaultCamera();
                    }
                    AddCamera(scene, camera);
                    cameraCount = arrlen(scene->cameras);
                    // Keep the current view; only make the new camera active if nothing was active
                    if (!hasActive) scene->activeCameraIndex = cameraCount - 1;
                }
                uiTop += 2 * margin + 2;
            }

            static Rectangle cameraPanelView = {0};
            Rectangle scrollRect = (Rectangle){uiLeft - 1, uiTop - 1, uiWidth - 2 * margin + 2, itemCount * (rowHeight + 2)};
            Rectangle contentRect = scrollRect;
            contentRect.x += 1;
            contentRect.y += 1;
            int scrollBarWidth = GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) + 2 * GuiGetStyle(DEFAULT, BORDER_WIDTH);
            contentRect.width -= scrollBarWidth + 2;
            contentRect.height = cameraCount * (rowHeight + 2);
            static Vector2 panelScroll = {0, 0};
            GuiScrollPanel(scrollRect, NULL, contentRect, &panelScroll, &cameraPanelView);
            int scrollTop = contentRect.y + panelScroll.y;

            if (contentRect.height < scrollRect.height)
            {
                GuiDrawRectangle((Rectangle){scrollRect.x + scrollRect.width - scrollBarWidth, scrollRect.y + 1, scrollBarWidth - 1, scrollRect.height - 2}, 0, WHITE, GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)));
            }

            uiTop += scrollRect.height + 2;
            bool lock = !CheckCollisionPointRec(GetMousePosition(), scrollRect) && !GuiIsLocked();
            if (lock)
                GuiLock();

            BeginScissorMode(cameraPanelView.x, cameraPanelView.y, cameraPanelView.width, cameraPanelView.height);

            for (int cameraIndex = 0; cameraIndex < cameraCount; ++cameraIndex)
            {
                Camera *camera = &scene->cameras[cameraIndex];

                int left = contentRect.x;
                int right = contentRect.x + contentRect.width;

                {
                    Rectangle rect = (Rectangle){left, scrollTop, 2 * margin, 2 * margin};
                    left += rect.width + 2;
                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect))
                    {
                        tooltip = "Hide/Show Camera";
                    }

                    GuiToggle(rect, GuiIconText(camera->hidden ? ICON_EYE_OFF : ICON_EYE_ON, NULL), &camera->hidden);

                }
                {
                    Rectangle rect = (Rectangle){left, scrollTop, 2 * margin, 2 * margin};
                    left += rect.width + 2;
                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect))
                    {
                        tooltip = "Projection type: Perspective/Orthographic";
                    }
                    bool isPerspective = camera->projectionType == PERSPECTIVE;
                    GuiToggle(rect, isPerspective ? "P" : "O", &isPerspective); // short label: full name is in the tooltip
                    camera->projectionType = isPerspective ? PERSPECTIVE : ORTHOGRAPHIC;
                }
                {
                    Rectangle rect = (Rectangle){right - 2 * margin, scrollTop, 2 * margin, 2 * margin};
                    right -= rect.width + 2;

                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect))
                    {
                        tooltip = "Delete Camera";
                    }

                    if (GuiButton(rect, GuiIconText(ICON_BIN, NULL)))
                    {
                        arrdel(scene->cameras, cameraIndex);
                        --cameraCount;

                        if (cameraCount == 0)
                        {
                            scene->activeCameraIndex = -1;
                        }
                        else if (scene->activeCameraIndex == cameraIndex)
                        {
                            scene->activeCameraIndex =
                                cameraIndex < cameraCount ? cameraIndex : cameraCount - 1;
                        }
                        else if (scene->activeCameraIndex > cameraIndex)
                        {
                            --scene->activeCameraIndex;
                        }

                        // Stop here: the next camera's delete button would sit at the same spot and also
                        // fire this frame (the mouse release is still active), deleting everything.
                        break;
                    }
                }
                {
                    const char *label = TextFormat(" Camera %3d", cameraIndex);
                    Rectangle rect = (Rectangle){left, scrollTop, right - left, 2 * margin};

                    bool hovered = false;
                    if (!GuiIsLocked() && CheckCollisionPointRec(GetMousePosition(), rect))
                    {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                        {
                            if (scene->activeCameraIndex != cameraIndex)
                            {
                                scene->activeCameraIndex = cameraIndex;
                            }
                            else
                            {
                                scene->activeCameraIndex = -1;
                            }
                        }

                        hovered = true;
                    }

                    bool active = cameraIndex == scene->activeCameraIndex;

                    Color backColor = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
                    Color textColor = GetColor(GuiGetStyle(DEFAULT, TEXT));
                    int border = 0;
                    if (active)
                    {
                        backColor = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED));
                        textColor = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED));
                    }
                    else if (hovered)
                    {
                        backColor = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
                        textColor = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
                    }
                    if (hovered)
                        border = 2;

                    GuiDrawRectangle(rect, border, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED)), backColor);
                    GuiDrawText(label, rect, TEXT_ALIGN_LEFT, textColor);
                }

                scrollTop += rowHeight + 2;
            }
            EndScissorMode();

            if (lock)
                GuiUnlock();
        }
    }

    // Global GUI elements that are drawn after all the other stuff (and maybe on top of it).
    // Those elements should also be always responsive, so we always unlock before drawing it.
    GuiUnlock();
    if (0 <= scene->activeCameraIndex &&
        scene->activeCameraIndex < arrlen(scene->cameras)) {
        int width = GetScreenWidth();
        int height = GetScreenHeight();
        if (width - 72 > uiRect.x + uiRect.width + 8) {
            DrawCameraOrientationGizmo(&scene->cameras[scene->activeCameraIndex], width - 42, 48);
        }

        const char *navigationHint = "LMB: Orbit  |  Wheel: Zoom  |  RMB: Pan  |  F: Frame";
        int hintWidth = MeasureText(navigationHint, 12);
        int hintX = width - hintWidth - 20;
        int hintY = height - 26;
        if (hintX > uiRect.x + uiRect.width + 8) {
            DrawRectangle(hintX - 6, hintY - 4, hintWidth + 12, 21, (Color){20, 24, 32, 210});
            DrawText(navigationHint, hintX, hintY, 12, RAYWHITE);
        }
    }
    if(tooltip != NULL) {
        Vector2 mouse = GetMousePosition();
        Tooltip(tooltip, mouse.x, mouse.y);
    }
    
    if (colorPickerValue != NULL) {
        int padding =  GuiGetStyle(COLORPICKER, HUEBAR_PADDING) + GuiGetStyle(COLORPICKER, HUEBAR_WIDTH);
        int size = 100;
        int x = MIN(MAX(0, colorPickerX - size/2), settings->width  - size - padding);
        int y = MIN(MAX(0, colorPickerY - size/2), settings->height - size - padding);
        Rectangle rect = (Rectangle) { x, y, size, size};
        Rectangle bounds = (Rectangle) { x, y, size + padding, size + padding};
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), bounds)) {
            colorPickerValue = NULL;
        } else {
            GuiDrawRectangle(bounds, 1,
                             GetColor(GuiGetStyle(BORDER, BORDER_COLOR_NORMAL)),
                             GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)));
            static float hue = 0.f;
            Vector3 hsv = ConvertRGBtoHSV((Vector3){ colorPickerValue->r, colorPickerValue->g, colorPickerValue->b });
            GuiColorPanelHSV(rect, NULL, &hsv);
            if (hsv.y > 0.f && hsv.z > 0.f) {
                hue = hsv.y;
            } else {
                hsv.y = hue;
            }
            
            Rectangle rectHue = { (float)rect.x + rect.width + GuiGetStyle(COLORPICKER, HUEBAR_PADDING), (float)rect.y, (float)GuiGetStyle(COLORPICKER, HUEBAR_WIDTH), (float)rect.height };
            Rectangle rectAlpha = { rect.x, rect.y + rect.height + GuiGetStyle(COLORPICKER, HUEBAR_PADDING), rect.width, GuiGetStyle(COLORPICKER, HUEBAR_WIDTH) };
            
            GuiColorBarHue(rectHue, NULL, &hsv.x);
            GuiColorBarAlpha(rectAlpha, NULL, &colorPickerValue->a);
            Vector3 rgb = ConvertHSVtoRGB(hsv);
            colorPickerValue->rgb = (Vec3){ rgb.x, rgb.y, rgb.z };
        }
    }
    
    {
        if (IsKeyPressed(KEY_ESCAPE)) {
            fileDialogState.windowActive = false;
        }
        GuiWindowFileDialog(&fileDialogState);
        
        if (fileDialogState.SelectFilePressed) {
            switch (fileDialogOperation) {
                case FILE_DIALOG_LOAD_MODEL: {
                    // Note that 'TextFormat' uses global storage for the strings it returns, so don't save pointers
                    // to those directly if you need to store them long-term, copy the result into a persistant buffer.
                    TraceLog(LOG_INFO, TextFormat("Selected file: %s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                    //TODO(student)DONE Load the model from the selected file and add it to the scene
                    const char *modelPath = TextFormat("%s" PATH_SEPERATOR "%s",
                                                       fileDialogState.dirPathText,
                                                       fileDialogState.fileNameText);
                    if (!FileExists(modelPath)) {
                        TraceLog(LOG_ERROR, "Selected model file does not exist: %s", modelPath);
                    } else {
                        Model model = LoadModelFromObjFile(modelPath);
                        model.vertexCount = arrlen(model.vertices);
                        AddModel(scene, model);
                        scene->activeModelIndex = arrlen(scene->models) - 1;
                        if (0 <= scene->activeCameraIndex &&
                            scene->activeCameraIndex < arrlen(scene->cameras)) {
                            FrameCameraOnModel(&scene->cameras[scene->activeCameraIndex],
                                               &scene->models[scene->activeModelIndex]);
                        }
                    }
                } break;
                default: {
                    TraceLog(LOG_WARNING, "Unsupported file dialog operation: %d", fileDialogOperation);
                } break;
            }
            fileDialogState.SelectFilePressed = false;
        }
    }
}

#endif // INTERFACE_IMPLEMENTED
#endif // INTERFACE_IMPLEMENTATION