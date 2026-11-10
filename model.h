#if !defined(MODEL_HEADER)
#define MODEL_HEADER

#include "math.h"

typedef struct Model {
    // Model geometry data
    int vertexCount;
    Vec3 *vertices;
    Vec3 *normals;
    // DONE(student): Add more vertex attributes ... 
    Vec2 *texturecoords;
    ObjTriangle* triangles;
    // Model transformation data
    Vec3 position;
    bool hidden;
    // DONE(student) ...
    Mat4 Rx;
    Mat4 Ry;
    Mat4 Rz;
    Vec3 scale;

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

typedef struct ObjTriangle {
    Vec3i corners[3]; // each is {v, vt, vn}
} ObjTriangle;

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

static int ParseObjIndex(const char *str, int *start, int len) {
    int i = *start;
    int sign = 1;
    int result = 0;

    if (i < len && str[i] == '-') {
        sign = -1;
        ++i;
    }
    while (i < len && str[i] >= '0' && str[i] <= '9') {
        result = result*10 + (str[i] - '0');
        ++i;
    }

    *start = i;
    return sign*result;
}

static Vec3i ParseNextCorner(const char *str, int *start, int len) {
    Vec3i corner = {0};
    int i = *start;

    while (i < len && (str[i] == ' ' || str[i] == '\t')) ++i;

    for (int field = 0; field < 3; ++field) {
        corner.values[field] = ParseObjIndex(str, &i, len);

        if (i < len && str[i] == '/') ++i; // move on to the next field
        else break;                        // ' ', '\r', '\n' or end: corner is done
    }

    *start = i;
    return corner;
}



static ObjTriangle ParseNextTriangle(const char *str, int *start, int len) {
    ObjTriangle triangle = {0};
    int i = *start;

    for (int c = 0; c < 3; ++c) {
        triangle.corners[c] = ParseNextCorner(str, &i, len);
    }

    *start = i;
    return triangle;
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
                    // DONE(student)
                    i++;
                    Vec3 vn = ParseNextVec3(data, &i, dataSize);
                    arrput(model.normals, vn);
                } else if (data[i] == 't') {
                    // DONE(student)
                    i++;
                    Vec2 vt = ParseNextVec2(data, &i, dataSize);
                    arrput(model.texturecoords, vt);
                } else {
                    TraceLog(LOG_WARNING, "%s:%d: OBJ Loader: Unhandled character '%c' when parsing a vertex, skipping the rest of the line", filename, lineIndex, data[i]);
                }
            } break;
            
            case 'f': {
                ++i; //skip f
                // DONE(student): No need for arbitrary polygons, only triangles are fine
                ObjTriangle new_triangle = ParseNextTriangle(data, &i, dataSize);
                arrput(model.triangles, new_triangle);
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
    arrfree(model->normals);
    arrfree(model->texturecoords);
    arrfree(model->triangles);
}

#endif // MODEL_IMPLEMENTED
#endif // MODEL_IMPLEMENTATION