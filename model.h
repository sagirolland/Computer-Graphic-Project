#if !defined(MODEL_HEADER)
#define MODEL_HEADER

#include "math.h"

typedef struct Model {
    // Model geometry data
    int vertexCount;
    Vec3 *vertices;
    Vec3 *normals;
    // TODO(student): Add more vertex attributes ... 
    Vec2 *texturecoords;

    // Model transformation data
    Vec3 position;
    bool hidden;
    // TODO(student) ...
    
    // GPU handles (HW3)
    unsigned int gpuVertexBufferId;
    unsigned int gpuNormalBufferId;
    
    unsigned int gpuVertexArrayId;
} Model;

Model LoadModelFromFile(const char *filename);
void FreeModel(Model *model);

#endif // MODEL_HEADER


// Model Implementation
#if defined(MODEL_IMPLEMENTATION)
#if !defined(MODEL_IMPLEMENTED)
#define MODEL_IMPLEMENTED

#include "stb_ds.h" // For dynamic arrays

//
// Provided helper functions
//
static int ParseNextInt(const char *str, int *start, int len) {
    int sign = 1;
    int result = 0;
    
    int i = *start;
    for(; i < len; ++i) {
        if(str[i] == '-') {
            sign = -1;
            break;
        } else if(str[i] >= '0' && str[i] <= '9') {
            break;
        } else if(str[i] == EOF || str[i] == '\n') {
            return 0;
        }
    }
    
    for(; i < len; ++i) {
        if(str[i] < '0' || str[i] > '9') break;
        
        result *= 10;
        result += (int)(str[i] - '0');
    }
    
    result *= sign;
    *start = i;
    
    return result;
}

static float ParseNextFloat(const char *str, int *start, int len) {
    bool parse_before_dot = true;
    int sign = 1;
    
    int i = *start;
    for(; i < len; ++i) {
        if(str[i] == '.') {
            ++i;
            parse_before_dot = false;
            break;
        } else if(str[i] == '-') {
            ++i;
            sign = -1;
            break;
        } else if(str[i] >= '0' && str[i] <= '9') {
            break;
        } else if(str[i] == EOF || str[i] == '\n') {
            return 0.f;
        }
    }
    
    int before_dot = 0;
    int after_dot = 0;
    int fraction_digits = 0;
    
    if(parse_before_dot) {
        for(; i < len; ++i) {
            if(str[i] < '0' || str[i] > '9') break;
            
            before_dot *= 10;
            before_dot += (int)(str[i] - '0');
        }
    }
    if(str[i] == '.') {
        ++i;
        for(; i < len; ++i) {
            if(str[i] < '0' || str[i] > '9') break;
            
            after_dot *= 10;
            after_dot += (int)(str[i] - '0');
            fraction_digits++;
        }
    }
    
    float result = (float)before_dot + (float)after_dot/pow(10, fraction_digits);
    
    result *= (float)sign;
    *start = i;
    
    return result;
}

static Vec2 ParseNextVec2(const char *str, int *start, int len) {
    Vec2 v;
    int i = *start;
    v.x = ParseNextFloat(str, &i, len);
    v.y = ParseNextFloat(str, &i, len);
    *start = i;
    return v;
}

static Vec3 ParseNextVec3(const char *str, int *start, int len) {
    Vec3 v;
    int i = *start;
    v.x = ParseNextFloat(str, &i, len);
    v.y = ParseNextFloat(str, &i, len);
    v.z = ParseNextFloat(str, &i, len);
    *start = i;
    return v;
}
// Helper functions end


Model LoadModelFromObjFile(const char *filename) {
    Model model = {};
    
    int dataSize = -1;
    const char *data = (char *)LoadFileData(filename, &dataSize);
    
    int lineIndex = 0;
    for (int i = 0; i < dataSize; ++i) {
        // Skip whitespace
        while ((data[i] == ' '  || data[i] == '\t' || data[i] == '\r') && (i < dataSize)) ++i;
        
        switch (data[i]) {
            case '#': {
                // OBJ comment, ignore line
            } break;
            
            case 'v': {
                ++i;
                if (data[i] == ' ') {
                    Vec3 v = ParseNextVec3(data, &i, dataSize);
                    arrput(model.vertices, v);
                } else if (data[i] == 'n') {
                    // TODO(student)
                } else {
                    TraceLog(LOG_WARNING, "%s:%d: OBJ Loader: Unhandled character '%c' when parsing a vertex, skipping the rest of the line", filename, lineIndex, data[i]);
                }
            } break;
            
            case 'f': {
                ++i;
                // TODO(student): No need for arbitrary polygons, only triangles are fine
            } break;
            
            default: {
                TraceLog(LOG_WARNING, "%s:%d: OBJ Loader: Unhandled character '%c' when parsing a line, skipping the rest of the line", filename, lineIndex, data[i]);
            } break;
        }
        
        // Ignore the rest of the line
        while((data[i] != '\n') && (i < dataSize)) ++i;
        
        lineIndex += 1;
    }
    
    UnloadFileData((unsigned char*)data);
    
    return model;
}

void FreeModel(Model *model) {
    arrfree(model->vertices);
}

#endif // MODEL_IMPLEMENTED
#endif // MODEL_IMPLEMENTATION