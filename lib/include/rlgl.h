/**********************************************************************************************
*
*   rlgl v6.0-technion - A multi-OpenGL abstraction layer with an immediate-mode style API
*
*   DESCRIPTION:
*       An abstraction layer for multiple OpenGL versions (1.1, 2.1, 3.3 Core, 4.3 Core, ES 2.0, ES 3.0)
*       that provides a pseudo-OpenGL 1.1 immediate-mode style API (rlVertex, rlTranslate, rlRotate...)
*
*   ADDITIONAL NOTES:
*       When choosing an OpenGL backend different than OpenGL 1.1, some internal buffers are
*       initialized on rlglInit() to accumulate vertex data
*
*       When an internal state change is required all the stored vertex data is rendered in a batch,
*       additionally, rlDrawRenderBatchActive() could be called to force flushing of the batch
*
*       Some resources are also loaded for convenience, here the complete list:
*          - Default batch (RLGL.defaultBatch): RenderBatch system to accumulate vertex data
*          - Default texture (RLGL.defaultTextureId): 1x1 white pixel R8G8B8A8
*          - Default shader (RLGL.State.defaultShaderId, RLGL.State.defaultShaderLocs)
*
*       Internal buffer (and resources) must be manually unloaded calling rlglClose()
*
*   CONFIGURATION:
*       #define GRAPHICS_API_OPENGL_SOFTWARE
*       #define GRAPHICS_API_OPENGL_11
*       #define GRAPHICS_API_OPENGL_21
*       #define GRAPHICS_API_OPENGL_33
*       #define GRAPHICS_API_OPENGL_43
*       #define GRAPHICS_API_OPENGL_ES2
*       #define GRAPHICS_API_OPENGL_ES3
*           Use selected OpenGL graphics backend, should be supported by platform
*           Those preprocessor defines are only used on the rlgl module, if OpenGL version is
*           required by any other module, use rlGetVersion() to check it
*
*       #define RLGL_IMPLEMENTATION
*           Generates the implementation of the library into the included file
*           If not defined, the library is in header only mode and can be included in other headers
*           or source files without problems. But only ONE file should hold the implementation
*
*       #if RLGL_SHOW_GL_DETAILS_INFO
*           Show OpenGL extensions and capabilities detailed logs on init
*
*       #if RLGL_ENABLE_OPENGL_DEBUG_CONTEXT
*           Enable debug context (only available on OpenGL 4.3)
*
*   DEPENDENCIES:
*      - OpenGL libraries (depending on platform and OpenGL version selected)
*      - GLAD OpenGL extensions loading library (only for OpenGL 3.3 Core, 4.3 Core)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2026 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RLGL_H
#define RLGL_H

#define RLGL_VERSION  "6.0-technion"

// Function specifiers in case library is build/used as a shared library
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
// NOTE: visibility(default) attribute makes symbols "visible" when compiled with -fvisibility=hidden
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
    #define RLAPI __declspec(dllexport)     // Building the library as a Win32 shared library (.dll)
#elif defined(BUILD_LIBTYPE_SHARED)
    #define RLAPI __attribute__((visibility("default"))) // Building the library as a Unix shared library (.so/.dylib)
#elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
    #define RLAPI __declspec(dllimport)     // Using the library as a Win32 shared library (.dll)
#endif

// Function specifiers definition
#ifndef RLAPI
    #define RLAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif

// Support TRACELOG macros
#ifndef TRACELOG
    #define TRACELOG(level, ...) (void)0
#endif

// Allow custom memory allocators
#ifndef RL_MALLOC
    #define RL_MALLOC(sz)     malloc(sz)
#endif
#ifndef RL_CALLOC
    #define RL_CALLOC(n,sz)   calloc(n,sz)
#endif
#ifndef RL_REALLOC
    #define RL_REALLOC(n,sz)  realloc(n,sz)
#endif
#ifndef RL_FREE
    #define RL_FREE(p)        free(p)
#endif

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_SOFTWARE) && \
    !defined(GRAPHICS_API_OPENGL_11) && \
    !defined(GRAPHICS_API_OPENGL_21) && \
    !defined(GRAPHICS_API_OPENGL_33) && \
    !defined(GRAPHICS_API_OPENGL_43) && \
    !defined(GRAPHICS_API_OPENGL_ES2) && \
    !defined(GRAPHICS_API_OPENGL_ES3)
        #define GRAPHICS_API_OPENGL_33
#endif

// Security check in case multiple GRAPHICS_API_OPENGL_* defined
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_SOFTWARE)
    #if defined(GRAPHICS_API_OPENGL_21)
        #undef GRAPHICS_API_OPENGL_21
    #endif
    #if defined(GRAPHICS_API_OPENGL_33)
        #undef GRAPHICS_API_OPENGL_33
    #endif
    #if defined(GRAPHICS_API_OPENGL_43)
        #undef GRAPHICS_API_OPENGL_43
    #endif
    #if defined(GRAPHICS_API_OPENGL_ES2)
        #undef GRAPHICS_API_OPENGL_ES2
    #endif
#endif

// Software implementation uses OpenGL 1.1 functionality
#if defined(GRAPHICS_API_OPENGL_SOFTWARE)
    #define GRAPHICS_API_OPENGL_11
#endif

// OpenGL 2.1 uses most of OpenGL 3.3 Core functionality
// WARNING: Specific parts are checked with #if defines
#if defined(GRAPHICS_API_OPENGL_21)
    #define GRAPHICS_API_OPENGL_33
#endif

// OpenGL 4.3 uses OpenGL 3.3 Core functionality
#if defined(GRAPHICS_API_OPENGL_43)
    #define GRAPHICS_API_OPENGL_33
#endif

// OpenGL ES 3.0 uses OpenGL ES 2.0 functionality (and more)
#if defined(GRAPHICS_API_OPENGL_ES3)
    #define GRAPHICS_API_OPENGL_ES2
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Projection matrix culling
#ifndef RL_CULL_DISTANCE_NEAR
    #define RL_CULL_DISTANCE_NEAR                0.05f      // Default near cull distance
#endif
#ifndef RL_CULL_DISTANCE_FAR
    #define RL_CULL_DISTANCE_FAR               4000.0f      // Default far cull distance
#endif

// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S                       0x2802      // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T                       0x2803      // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER                   0x2800      // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER                   0x2801      // GL_TEXTURE_MIN_FILTER

#define RL_TEXTURE_FILTER_NEAREST               0x2600      // GL_NEAREST
#define RL_TEXTURE_FILTER_LINEAR                0x2601      // GL_LINEAR
#define RL_TEXTURE_FILTER_MIP_NEAREST           0x2700      // GL_NEAREST_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR    0x2702      // GL_NEAREST_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST    0x2701      // GL_LINEAR_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_MIP_LINEAR            0x2703      // GL_LINEAR_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_ANISOTROPIC           0x3000      // Anisotropic filter (custom identifier)
#define RL_TEXTURE_MIPMAP_BIAS_RATIO            0x4000      // Texture mipmap bias, percentage ratio (custom identifier)

#define RL_TEXTURE_WRAP_REPEAT                  0x2901      // GL_REPEAT
#define RL_TEXTURE_WRAP_CLAMP                   0x812F      // GL_CLAMP_TO_EDGE
#define RL_TEXTURE_WRAP_MIRROR_REPEAT           0x8370      // GL_MIRRORED_REPEAT
#define RL_TEXTURE_WRAP_MIRROR_CLAMP            0x8742      // GL_MIRROR_CLAMP_EXT

// Matrix modes (equivalent to OpenGL)
#define RL_MODELVIEW                            0x1700      // GL_MODELVIEW
#define RL_PROJECTION                           0x1701      // GL_PROJECTION
#define RL_TEXTURE                              0x1702      // GL_TEXTURE

// Primitive assembly draw modes
#define RL_LINES                                0x0001      // GL_LINES
#define RL_TRIANGLES                            0x0004      // GL_TRIANGLES
#define RL_QUADS                                0x0007      // GL_QUADS

// GL equivalent data types
#define RL_UNSIGNED_BYTE                        0x1401      // GL_UNSIGNED_BYTE
#define RL_FLOAT                                0x1406      // GL_FLOAT

// GL buffer usage hint
#define RL_STREAM_DRAW                          0x88E0      // GL_STREAM_DRAW
#define RL_STREAM_READ                          0x88E1      // GL_STREAM_READ
#define RL_STREAM_COPY                          0x88E2      // GL_STREAM_COPY
#define RL_STATIC_DRAW                          0x88E4      // GL_STATIC_DRAW
#define RL_STATIC_READ                          0x88E5      // GL_STATIC_READ
#define RL_STATIC_COPY                          0x88E6      // GL_STATIC_COPY
#define RL_DYNAMIC_DRAW                         0x88E8      // GL_DYNAMIC_DRAW
#define RL_DYNAMIC_READ                         0x88E9      // GL_DYNAMIC_READ
#define RL_DYNAMIC_COPY                         0x88EA      // GL_DYNAMIC_COPY

// GL Shader type
#define RL_FRAGMENT_SHADER                      0x8B30      // GL_FRAGMENT_SHADER
#define RL_VERTEX_SHADER                        0x8B31      // GL_VERTEX_SHADER
#define RL_COMPUTE_SHADER                       0x91B9      // GL_COMPUTE_SHADER

// GL blending factors
#define RL_ZERO                                 0           // GL_ZERO
#define RL_ONE                                  1           // GL_ONE
#define RL_SRC_COLOR                            0x0300      // GL_SRC_COLOR
#define RL_ONE_MINUS_SRC_COLOR                  0x0301      // GL_ONE_MINUS_SRC_COLOR
#define RL_SRC_ALPHA                            0x0302      // GL_SRC_ALPHA
#define RL_ONE_MINUS_SRC_ALPHA                  0x0303      // GL_ONE_MINUS_SRC_ALPHA
#define RL_DST_ALPHA                            0x0304      // GL_DST_ALPHA
#define RL_ONE_MINUS_DST_ALPHA                  0x0305      // GL_ONE_MINUS_DST_ALPHA
#define RL_DST_COLOR                            0x0306      // GL_DST_COLOR
#define RL_ONE_MINUS_DST_COLOR                  0x0307      // GL_ONE_MINUS_DST_COLOR
#define RL_SRC_ALPHA_SATURATE                   0x0308      // GL_SRC_ALPHA_SATURATE
#define RL_CONSTANT_COLOR                       0x8001      // GL_CONSTANT_COLOR
#define RL_ONE_MINUS_CONSTANT_COLOR             0x8002      // GL_ONE_MINUS_CONSTANT_COLOR
#define RL_CONSTANT_ALPHA                       0x8003      // GL_CONSTANT_ALPHA
#define RL_ONE_MINUS_CONSTANT_ALPHA             0x8004      // GL_ONE_MINUS_CONSTANT_ALPHA

// GL blending functions/equations
#define RL_FUNC_ADD                             0x8006      // GL_FUNC_ADD
#define RL_MIN                                  0x8007      // GL_MIN
#define RL_MAX                                  0x8008      // GL_MAX
#define RL_FUNC_SUBTRACT                        0x800A      // GL_FUNC_SUBTRACT
#define RL_FUNC_REVERSE_SUBTRACT                0x800B      // GL_FUNC_REVERSE_SUBTRACT
#define RL_BLEND_EQUATION                       0x8009      // GL_BLEND_EQUATION
#define RL_BLEND_EQUATION_RGB                   0x8009      // GL_BLEND_EQUATION_RGB   // (Same as BLEND_EQUATION)
#define RL_BLEND_EQUATION_ALPHA                 0x883D      // GL_BLEND_EQUATION_ALPHA
#define RL_BLEND_DST_RGB                        0x80C8      // GL_BLEND_DST_RGB
#define RL_BLEND_SRC_RGB                        0x80C9      // GL_BLEND_SRC_RGB
#define RL_BLEND_DST_ALPHA                      0x80CA      // GL_BLEND_DST_ALPHA
#define RL_BLEND_SRC_ALPHA                      0x80CB      // GL_BLEND_SRC_ALPHA
#define RL_BLEND_COLOR                          0x8005      // GL_BLEND_COLOR

#define RL_READ_FRAMEBUFFER                     0x8CA8      // GL_READ_FRAMEBUFFER
#define RL_DRAW_FRAMEBUFFER                     0x8CA9      // GL_DRAW_FRAMEBUFFER

// Default shader vertex attribute locations
// NOTE: Locations can be redefined by user if required
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION    0
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD    1
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL      2
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR       3
#endif
    #ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT         4
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2   5
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES     6
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES 7
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS 8
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM 9
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if !defined(__cplusplus)
#if (defined(__STDC__) && __STDC_VERSION__ >= 199901L) || (defined(_MSC_VER) && _MSC_VER >= 1800)
    #include <stdbool.h>
#elif !defined(bool)
    typedef enum bool { false = 0, true = !false } bool;
    #define RL_BOOL_TYPE
#endif
#endif

#if !defined(RL_MATRIX_TYPE)
// Matrix, 4x4 components, column major, OpenGL style, right handed
typedef struct Matrix {
    float m0, m4, m8, m12;      // Matrix first row (4 components)
    float m1, m5, m9, m13;      // Matrix second row (4 components)
    float m2, m6, m10, m14;     // Matrix third row (4 components)
    float m3, m7, m11, m15;     // Matrix fourth row (4 components)
} Matrix;
#define RL_MATRIX_TYPE
#endif

// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct rlVertexBuffer {
    int elementCount;           // Number of elements in the buffer (QUADS)

    float *vertices;            // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *normals;             // Vertex normal (XYZ - 3 components per vertex) (shader-location = 2)
    unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    unsigned int *indices;      // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
    unsigned int vaoId;         // OpenGL Vertex Array Object id
    unsigned int vboId[5];      // OpenGL Vertex Buffer Objects id (5 types of vertex data)
} rlVertexBuffer;

// Draw call type
// NOTE: Only texture changes register a new draw, other state-change-related elements are not
// used at this moment (vaoId, shaderId, matrices), raylib forces a batch draw call if any
// of those state-change happens (this is done in core module)
typedef struct rlDrawCall {
    int mode;                   // Drawing mode: LINES, TRIANGLES, QUADS
    int vertexCount;            // Number of vertex of the draw
    int vertexAlignment;        // Number of vertex required for index alignment (LINES, TRIANGLES)
    //unsigned int vaoId;       // Vertex array id to be used on the draw -> Using RLGL.currentBatch->vertexBuffer.vaoId
    //unsigned int shaderId;    // Shader id to be used on the draw -> Using RLGL.currentShaderId
    unsigned int textureId;     // Texture id to be used on the draw -> Use to create new draw call if changes

    //Matrix projection;        // Projection matrix for this draw -> Using RLGL.projection by default
    //Matrix modelview;         // Modelview matrix for this draw -> Using RLGL.modelview by default
} rlDrawCall;

// rlRenderBatch type
typedef struct rlRenderBatch {
    int bufferCount;            // Number of vertex buffers (multi-buffering support)
    int currentBuffer;          // Current buffer tracking in case of multi-buffering
    rlVertexBuffer *vertexBuffer; // Dynamic buffer(s) for vertex data

    rlDrawCall *draws;          // Draw calls array, depends on textureId
    int drawCounter;            // Draw calls counter
    float currentDepth;         // Current depth value for next draw
} rlRenderBatch;

// OpenGL version
typedef enum {
    RL_OPENGL_SOFTWARE = 0,  // Software rendering
    RL_OPENGL_11,               // OpenGL 1.1
    RL_OPENGL_21,               // OpenGL 2.1 (GLSL 120)
    RL_OPENGL_33,               // OpenGL 3.3 (GLSL 330)
    RL_OPENGL_43,               // OpenGL 4.3 (using GLSL 330)
    RL_OPENGL_ES_20,            // OpenGL ES 2.0 (GLSL 100)
    RL_OPENGL_ES_30             // OpenGL ES 3.0 (GLSL 300 es)
} rlGlVersion;

// Trace log level
// NOTE: Organized by priority level
typedef enum {
    RL_LOG_ALL = 0,             // Display all logs
    RL_LOG_TRACE,               // Trace logging, intended for internal use only
    RL_LOG_DEBUG,               // Debug logging, used for internal debugging, it should be disabled on release builds
    RL_LOG_INFO,                // Info logging, used for program execution info
    RL_LOG_WARNING,             // Warning logging, used on recoverable failures
    RL_LOG_ERROR,               // Error logging, used on unrecoverable failures
    RL_LOG_FATAL,               // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    RL_LOG_NONE                 // Disable logging
} rlTraceLogLevel;

// Texture pixel formats
// NOTE: Support depends on OpenGL version
typedef enum {
    RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,        // 8*2 bpp (2 channels)
    RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,            // 16 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,            // 24 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          // 32 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R32,               // 32 bpp (1 channel - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,         // 32*3 bpp (3 channels - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      // 32*4 bpp (4 channels - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16,               // 16 bpp (1 channel - half float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,         // 16*3 bpp (3 channels - half float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,      // 16*4 bpp (4 channels - half float)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,           // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,           // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC1_RGB,            // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_RGB,            // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGB,            // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,           // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
} rlPixelFormat;

// Texture parameters: filter mode
// NOTE 1: Filtering considers mipmaps if available in the texture
// NOTE 2: Filter is accordingly set for minification and magnification
typedef enum {
    RL_TEXTURE_FILTER_POINT = 0,        // No filter, pixel approximation
    RL_TEXTURE_FILTER_BILINEAR,         // Linear filtering
    RL_TEXTURE_FILTER_TRILINEAR,        // Trilinear filtering (linear with mipmaps)
    RL_TEXTURE_FILTER_ANISOTROPIC_4X,   // Anisotropic filtering 4x
    RL_TEXTURE_FILTER_ANISOTROPIC_8X,   // Anisotropic filtering 8x
    RL_TEXTURE_FILTER_ANISOTROPIC_16X,  // Anisotropic filtering 16x
} rlTextureFilter;

// Color blending modes (pre-defined)
typedef enum {
    RL_BLEND_ALPHA = 0,                 // Blend textures considering alpha (default)
    RL_BLEND_ADDITIVE,                  // Blend textures adding colors
    RL_BLEND_MULTIPLIED,                // Blend textures multiplying colors
    RL_BLEND_ADD_COLORS,                // Blend textures adding colors (alternative)
    RL_BLEND_SUBTRACT_COLORS,           // Blend textures subtracting colors (alternative)
    RL_BLEND_ALPHA_PREMULTIPLY,         // Blend premultiplied textures considering alpha
    RL_BLEND_CUSTOM,                    // Blend textures using custom src/dst factors (use rlSetBlendFactors())
    RL_BLEND_CUSTOM_SEPARATE            // Blend textures using custom src/dst factors (use rlSetBlendFactorsSeparate())
} rlBlendMode;

// Shader location point type
typedef enum {
    RL_SHADER_LOC_VERTEX_POSITION = 0,  // Shader location: vertex attribute: position
    RL_SHADER_LOC_VERTEX_TEXCOORD01,    // Shader location: vertex attribute: texcoord01
    RL_SHADER_LOC_VERTEX_TEXCOORD02,    // Shader location: vertex attribute: texcoord02
    RL_SHADER_LOC_VERTEX_NORMAL,        // Shader location: vertex attribute: normal
    RL_SHADER_LOC_VERTEX_TANGENT,       // Shader location: vertex attribute: tangent
    RL_SHADER_LOC_VERTEX_COLOR,         // Shader location: vertex attribute: color
    RL_SHADER_LOC_MATRIX_MVP,           // Shader location: matrix uniform: model-view-projection
    RL_SHADER_LOC_MATRIX_VIEW,          // Shader location: matrix uniform: view (camera transform)
    RL_SHADER_LOC_MATRIX_PROJECTION,    // Shader location: matrix uniform: projection
    RL_SHADER_LOC_MATRIX_MODEL,         // Shader location: matrix uniform: model (transform)
    RL_SHADER_LOC_MATRIX_NORMAL,        // Shader location: matrix uniform: normal
    RL_SHADER_LOC_VECTOR_VIEW,          // Shader location: vector uniform: view
    RL_SHADER_LOC_COLOR_DIFFUSE,        // Shader location: vector uniform: diffuse color
    RL_SHADER_LOC_COLOR_SPECULAR,       // Shader location: vector uniform: specular color
    RL_SHADER_LOC_COLOR_AMBIENT,        // Shader location: vector uniform: ambient color
    RL_SHADER_LOC_MAP_ALBEDO,           // Shader location: sampler2d texture: albedo (same as: RL_SHADER_LOC_MAP_DIFFUSE)
    RL_SHADER_LOC_MAP_METALNESS,        // Shader location: sampler2d texture: metalness (same as: RL_SHADER_LOC_MAP_SPECULAR)
    RL_SHADER_LOC_MAP_NORMAL,           // Shader location: sampler2d texture: normal
    RL_SHADER_LOC_MAP_ROUGHNESS,        // Shader location: sampler2d texture: roughness
    RL_SHADER_LOC_MAP_OCCLUSION,        // Shader location: sampler2d texture: occlusion
    RL_SHADER_LOC_MAP_EMISSION,         // Shader location: sampler2d texture: emission
    RL_SHADER_LOC_MAP_HEIGHT,           // Shader location: sampler2d texture: height
    RL_SHADER_LOC_MAP_CUBEMAP,          // Shader location: samplerCube texture: cubemap
    RL_SHADER_LOC_MAP_IRRADIANCE,       // Shader location: samplerCube texture: irradiance
    RL_SHADER_LOC_MAP_PREFILTER,        // Shader location: samplerCube texture: prefilter
    RL_SHADER_LOC_MAP_BRDF              // Shader location: sampler2d texture: brdf
} rlShaderLocationIndex;

#define RL_SHADER_LOC_MAP_DIFFUSE       RL_SHADER_LOC_MAP_ALBEDO
#define RL_SHADER_LOC_MAP_SPECULAR      RL_SHADER_LOC_MAP_METALNESS

// Shader uniform data type
typedef enum {
    RL_SHADER_UNIFORM_FLOAT = 0,        // Shader uniform type: float
    RL_SHADER_UNIFORM_VEC2,             // Shader uniform type: vec2 (2 float)
    RL_SHADER_UNIFORM_VEC3,             // Shader uniform type: vec3 (3 float)
    RL_SHADER_UNIFORM_VEC4,             // Shader uniform type: vec4 (4 float)
    RL_SHADER_UNIFORM_INT,              // Shader uniform type: int
    RL_SHADER_UNIFORM_IVEC2,            // Shader uniform type: ivec2 (2 int)
    RL_SHADER_UNIFORM_IVEC3,            // Shader uniform type: ivec3 (3 int)
    RL_SHADER_UNIFORM_IVEC4,            // Shader uniform type: ivec4 (4 int)
    RL_SHADER_UNIFORM_UINT,             // Shader uniform type: unsigned int
    RL_SHADER_UNIFORM_UIVEC2,           // Shader uniform type: uivec2 (2 unsigned int)
    RL_SHADER_UNIFORM_UIVEC3,           // Shader uniform type: uivec3 (3 unsigned int)
    RL_SHADER_UNIFORM_UIVEC4,           // Shader uniform type: uivec4 (4 unsigned int)
    RL_SHADER_UNIFORM_SAMPLER2D         // Shader uniform type: sampler2d
} rlShaderUniformDataType;

// Shader attribute data types
typedef enum {
    RL_SHADER_ATTRIB_FLOAT = 0,         // Shader attribute type: float
    RL_SHADER_ATTRIB_VEC2,              // Shader attribute type: vec2 (2 float)
    RL_SHADER_ATTRIB_VEC3,              // Shader attribute type: vec3 (3 float)
    RL_SHADER_ATTRIB_VEC4               // Shader attribute type: vec4 (4 float)
} rlShaderAttributeDataType;

// Framebuffer attachment type
// NOTE: By default up to 8 color channels defined, but it can be more
typedef enum {
    RL_ATTACHMENT_COLOR_CHANNEL0 = 0,       // Framebuffer attachment type: color 0
    RL_ATTACHMENT_COLOR_CHANNEL1 = 1,       // Framebuffer attachment type: color 1
    RL_ATTACHMENT_COLOR_CHANNEL2 = 2,       // Framebuffer attachment type: color 2
    RL_ATTACHMENT_COLOR_CHANNEL3 = 3,       // Framebuffer attachment type: color 3
    RL_ATTACHMENT_COLOR_CHANNEL4 = 4,       // Framebuffer attachment type: color 4
    RL_ATTACHMENT_COLOR_CHANNEL5 = 5,       // Framebuffer attachment type: color 5
    RL_ATTACHMENT_COLOR_CHANNEL6 = 6,       // Framebuffer attachment type: color 6
    RL_ATTACHMENT_COLOR_CHANNEL7 = 7,       // Framebuffer attachment type: color 7
    RL_ATTACHMENT_DEPTH = 100,              // Framebuffer attachment type: depth
    RL_ATTACHMENT_STENCIL = 200,            // Framebuffer attachment type: stencil
} rlFramebufferAttachType;

// Framebuffer texture attachment type
typedef enum {
    RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,   // Framebuffer texture attachment type: cubemap, +X side
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_X = 1,   // Framebuffer texture attachment type: cubemap, -X side
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Y = 2,   // Framebuffer texture attachment type: cubemap, +Y side
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y = 3,   // Framebuffer texture attachment type: cubemap, -Y side
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Z = 4,   // Framebuffer texture attachment type: cubemap, +Z side
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z = 5,   // Framebuffer texture attachment type: cubemap, -Z side
    RL_ATTACHMENT_TEXTURE2D = 100,          // Framebuffer texture attachment type: texture2d
    RL_ATTACHMENT_RENDERBUFFER = 200,       // Framebuffer texture attachment type: renderbuffer
} rlFramebufferAttachTextureType;

// Face culling mode
typedef enum {
    RL_CULL_FACE_FRONT = 0,
    RL_CULL_FACE_BACK
} rlCullMode;

//------------------------------------------------------------------------------------
// Functions Declaration - Matrix operations
//------------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

RLAPI void rlMatrixMode(int mode);                      // Choose the current matrix to be transformed
RLAPI void rlPushMatrix(void);                          // Push the current matrix to stack
RLAPI void rlPopMatrix(void);                           // Pop latest inserted matrix from stack
RLAPI void rlLoadIdentity(void);                        // Reset current matrix to identity matrix
RLAPI void rlTranslatef(float x, float y, float z);     // Multiply the current matrix by a translation matrix
RLAPI void rlRotatef(float angle, float x, float y, float z); // Multiply the current matrix by a rotation matrix
RLAPI void rlScalef(float x, float y, float z);         // Multiply the current matrix by a scaling matrix
RLAPI void rlMultMatrixf(const float *matf);            // Multiply the current matrix by another matrix
RLAPI void rlFrustum(float left, float right, float bottom, float top, float znear, float zfar);
RLAPI void rlOrtho(float left, float right, float bottom, float top, float znear, float zfar);
RLAPI void rlViewport(int x, int y, int width, int height); // Set the viewport area
RLAPI void rlSetClipPlanes(float nearPlane, float farPlane); // Set clip planes distances
RLAPI float rlGetCullDistanceNear(void);                // Get cull plane distance near
RLAPI float rlGetCullDistanceFar(void);                 // Get cull plane distance far

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
RLAPI void rlBegin(int mode);                           // Initialize drawing mode (how to organize vertex)
RLAPI void rlEnd(void);                                 // Finish vertex providing
RLAPI void rlVertex2i(int x, int y);                    // Define one vertex (position) - 2 int
RLAPI void rlVertex2f(float x, float y);                // Define one vertex (position) - 2 float
RLAPI void rlVertex3f(float x, float y, float z);       // Define one vertex (position) - 3 float
RLAPI void rlTexCoord2f(float x, float y);              // Define one vertex (texture coordinate) - 2 float
RLAPI void rlNormal3f(float x, float y, float z);       // Define one vertex (normal) - 3 float
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // Define one vertex (color) - 4 byte
RLAPI void rlColor3f(float x, float y, float z);        // Define one vertex (color) - 3 float
RLAPI void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL style functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer,
// some of them are direct wrappers over OpenGL calls, some others are custom
//------------------------------------------------------------------------------------

// Vertex buffers state
RLAPI bool rlEnableVertexArray(unsigned int vaoId);     // Enable vertex array (VAO, if supported)
RLAPI void rlDisableVertexArray(void);                  // Disable vertex array (VAO, if supported)
RLAPI void rlEnableVertexBuffer(unsigned int id);       // Enable vertex buffer (VBO)
RLAPI void rlDisableVertexBuffer(void);                 // Disable vertex buffer (VBO)
RLAPI void rlEnableVertexBufferElements(unsigned int id); // Enable vertex buffer element (VBO element)
RLAPI void rlDisableVertexBufferElements(void);          // Disable vertex buffer element (VBO element)
RLAPI void rlEnableVertexAttribute(unsigned int index); // Enable vertex attribute index
RLAPI void rlDisableVertexAttribute(unsigned int index); // Disable vertex attribute index
RLAPI void rlEnableStatePointer(int vertexAttribType, void *buffer); // Enable attribute state pointer
RLAPI void rlDisableStatePointer(int vertexAttribType); // Disable attribute state pointer

// Textures state
RLAPI void rlActiveTextureSlot(int slot);               // Select and active a texture slot
RLAPI void rlEnableTexture(unsigned int id);            // Enable texture
RLAPI void rlDisableTexture(void);                      // Disable texture
RLAPI void rlEnableTextureCubemap(unsigned int id);     // Enable texture cubemap
RLAPI void rlDisableTextureCubemap(void);               // Disable texture cubemap
RLAPI void rlTextureParameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)
RLAPI void rlCubemapParameters(unsigned int id, int param, int value); // Set cubemap parameters (filter, wrap)

// Shader state
RLAPI void rlEnableShader(unsigned int id);             // Enable shader program
RLAPI void rlDisableShader(void);                       // Disable shader program

// Framebuffer state
RLAPI void rlEnableFramebuffer(unsigned int id);        // Enable render texture (fbo)
RLAPI void rlDisableFramebuffer(void);                  // Disable render texture (fbo), return to default framebuffer
RLAPI unsigned int rlGetActiveFramebuffer(void);        // Get the currently active render texture (fbo), 0 for default framebuffer
RLAPI void rlActiveDrawBuffers(int count);              // Activate multiple draw color buffers
RLAPI void rlBlitFramebuffer(int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight, int bufferMask); // Blit active framebuffer to main framebuffer
RLAPI void rlBindFramebuffer(unsigned int target, unsigned int framebuffer); // Bind framebuffer (FBO)

// General render state
RLAPI void rlEnableColorBlend(void);                    // Enable color blending
RLAPI void rlDisableColorBlend(void);                   // Disable color blending
RLAPI void rlEnableDepthTest(void);                     // Enable depth test
RLAPI void rlDisableDepthTest(void);                    // Disable depth test
RLAPI void rlEnableDepthMask(void);                     // Enable depth write
RLAPI void rlDisableDepthMask(void);                    // Disable depth write
RLAPI void rlEnableBackfaceCulling(void);               // Enable backface culling
RLAPI void rlDisableBackfaceCulling(void);              // Disable backface culling
RLAPI void rlColorMask(bool r, bool g, bool b, bool a); // Color mask control
RLAPI void rlSetCullFace(int mode);                     // Set face culling mode
RLAPI void rlEnableScissorTest(void);                   // Enable scissor test
RLAPI void rlDisableScissorTest(void);                  // Disable scissor test
RLAPI void rlScissor(int x, int y, int width, int height); // Scissor test
RLAPI void rlEnablePointMode(void);                     // Enable point mode
RLAPI void rlDisablePointMode(void);                    // Disable point mode
RLAPI void rlSetPointSize(float size);                  // Set the point drawing size
RLAPI float rlGetPointSize(void);                       // Get the point drawing size
RLAPI void rlEnableWireMode(void);                      // Enable wire mode
RLAPI void rlDisableWireMode(void);                     // Disable wire mode
RLAPI void rlSetLineWidth(float width);                 // Set the line drawing width
RLAPI float rlGetLineWidth(void);                       // Get the line drawing width
RLAPI void rlEnableSmoothLines(void);                   // Enable line aliasing
RLAPI void rlDisableSmoothLines(void);                  // Disable line aliasing
RLAPI void rlEnableStereoRender(void);                  // Enable stereo rendering
RLAPI void rlDisableStereoRender(void);                 // Disable stereo rendering
RLAPI bool rlIsStereoRenderEnabled(void);               // Check if stereo render is enabled

RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // Clear color buffer with color
RLAPI void rlClearScreenBuffers(void);                  // Clear used screen buffers (color and depth)
RLAPI bool rlCheckErrors(void);                         // Check and log OpenGL error codes
RLAPI void rlSetBlendMode(int mode);                    // Set blending mode
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation); // Set blending mode factor and equation (using OpenGL factors)
RLAPI void rlSetBlendFactorsSeparate(int glSrcRGB, int glDstRGB, int glSrcAlpha, int glDstAlpha, int glEqRGB, int glEqAlpha); // Set blending mode factors and equations separately (using OpenGL factors)

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
// rlgl initialization functions
RLAPI void rlglInit(int width, int height);             // Initialize rlgl (buffers, shaders, textures, states)
RLAPI void rlglClose(void);                             // De-initialize rlgl (buffers, shaders, textures)
RLAPI void rlLoadExtensions(void *loader);              // Load OpenGL extensions (loader function required)
RLAPI void *rlGetProcAddress(const char *procName);     // Get OpenGL procedure address
RLAPI int rlGetVersion(void);                           // Get current OpenGL version
RLAPI void rlSetFramebufferWidth(int width);            // Set current framebuffer width
RLAPI int rlGetFramebufferWidth(void);                  // Get default framebuffer width
RLAPI void rlSetFramebufferHeight(int height);          // Set current framebuffer height
RLAPI int rlGetFramebufferHeight(void);                 // Get default framebuffer height

RLAPI unsigned int rlGetTextureIdDefault(void);         // Get default texture id
RLAPI unsigned int rlGetShaderIdDefault(void);          // Get default shader id
RLAPI int *rlGetShaderLocsDefault(void);                // Get default shader locations

// Render batch management
// NOTE: rlgl provides a default render batch to behave like OpenGL 1.1 immediate mode
// but this render batch API is exposed in case of custom batches are required
RLAPI rlRenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements); // Load a render batch system
RLAPI void rlUnloadRenderBatch(rlRenderBatch batch);    // Unload render batch system
RLAPI void rlDrawRenderBatch(rlRenderBatch *batch);     // Draw render batch data (Update->Draw->Reset)
RLAPI void rlSetRenderBatchActive(rlRenderBatch *batch); // Set the active render batch for rlgl (NULL for default internal)
RLAPI void rlDrawRenderBatchActive(void);               // Update and draw internal render batch
RLAPI bool rlCheckRenderBatchLimit(int vCount);         // Check internal buffer overflow for a given number of vertex

RLAPI void rlSetTexture(unsigned int id);               // Set current texture for render batch and check buffers limits

//------------------------------------------------------------------------------------------------------------------------

// Vertex buffers management
RLAPI unsigned int rlLoadVertexArray(void);             // Load vertex array (vao) if supported
RLAPI unsigned int rlLoadVertexBuffer(const void *buffer, int size, bool dynamic); // Load a vertex buffer object
RLAPI unsigned int rlLoadVertexBufferElements(const void *buffer, int size, bool dynamic); // Load vertex buffer elements object
RLAPI void rlUpdateVertexBuffer(unsigned int bufferId, const void *data, int dataSize, int offset); // Update vertex buffer object data on GPU buffer
RLAPI void rlUpdateVertexBufferElements(unsigned int id, const void *data, int dataSize, int offset); // Update vertex buffer elements data on GPU buffer
RLAPI void rlUnloadVertexArray(unsigned int vaoId);     // Unload vertex array (vao)
RLAPI void rlUnloadVertexBuffer(unsigned int vboId);    // Unload vertex buffer object
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, int offset); // Set vertex attribute data configuration
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor); // Set vertex attribute data divisor
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count); // Set vertex attribute default value, when attribute to provided
RLAPI void rlDrawVertexArray(int offset, int count);    // Draw vertex array (currently active vao)
RLAPI void rlDrawVertexArrayElements(int offset, int count, const void *buffer); // Draw vertex array elements
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances); // Draw vertex array (currently active vao) with instancing
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, const void *buffer, int instances); // Draw vertex array elements with instancing

// Textures management
RLAPI unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount); // Load texture data
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer); // Load depth texture/renderbuffer (to be attached to fbo)
RLAPI unsigned int rlLoadTextureCubemap(const void *data, int size, int format, int mipmapCount); // Load texture cubemap data
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data); // Update texture with new data on GPU
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType); // Get OpenGL internal formats
RLAPI const char *rlGetPixelFormatName(unsigned int format);              // Get name string for pixel format
RLAPI void rlUnloadTexture(unsigned int id);                              // Unload texture from GPU memory
RLAPI void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int *mipmaps); // Generate mipmap data for selected texture
RLAPI void *rlReadTexturePixels(unsigned int id, int width, int height, int format); // Read texture pixel data
RLAPI unsigned char *rlReadScreenPixels(int width, int height);           // Read screen pixel data (color buffer)

// Framebuffer management (fbo)
RLAPI unsigned int rlLoadFramebuffer(void);                               // Load an empty framebuffer
RLAPI void rlFramebufferAttach(unsigned int id, unsigned int texId, int attachType, int texType, int mipLevel); // Attach texture/renderbuffer to a framebuffer
RLAPI bool rlFramebufferComplete(unsigned int id);                        // Verify framebuffer is complete
RLAPI void rlUnloadFramebuffer(unsigned int id);                          // Delete framebuffer from GPU
// WARNING: Copy and resize framebuffer functionality only defined for software backend
RLAPI void rlCopyFramebuffer(int x, int y, int width, int height, int format, void *pixels); // Copy framebuffer pixel data to internal buffer
RLAPI void rlResizeFramebuffer(int width, int height);                    // Resize internal framebuffer

// Shaders management
RLAPI unsigned int rlLoadShader(const char *code, int type);                    // Load (compile) shader and return shader id (type: RL_VERTEX_SHADER, RL_FRAGMENT_SHADER, RL_COMPUTE_SHADER)
RLAPI unsigned int rlLoadShaderProgram(const char *vsCode, const char *fsCode); // Load shader from code strings
RLAPI unsigned int rlLoadShaderProgramEx(unsigned int vsId, unsigned int fsId); // Load shader program, using already loaded shader ids
RLAPI unsigned int rlLoadShaderProgramCompute(unsigned int csId);               // Load compute shader program
RLAPI void rlUnloadShader(unsigned int id);                                     // Unload shader, loaded with rlLoadShader()
RLAPI void rlUnloadShaderProgram(unsigned int id);                              // Unload shader program
RLAPI int rlGetLocationUniform(unsigned int id, const char *uniformName);       // Get shader location uniform, requires shader program id
RLAPI int rlGetLocationAttrib(unsigned int id, const char *attribName);         // Get shader location attribute, requires shader program id
RLAPI void rlSetUniform(int locIndex, const void *value, int uniformType, int count); // Set shader value uniform
RLAPI void rlSetUniformMatrix(int locIndex, const float mat[16]);                     // Set shader value 4x4 matrix
RLAPI void rlSetUniformMatrices(int locIndex, const Matrix *mat, int count);    // Set shader value matrices
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId);           // Set shader value sampler
RLAPI void rlSetShader(unsigned int id, int *locs);                             // Set shader currently active (id and locations)

// Compute shader management
RLAPI void rlComputeShaderDispatch(unsigned int groupX, unsigned int groupY, unsigned int groupZ); // Dispatch compute shader (equivalent to *draw* for graphics pipeline)

// Shader buffer storage object management (ssbo)
RLAPI unsigned int rlLoadShaderBuffer(unsigned int size, const void *data, int usageHint); // Load shader storage buffer object (SSBO)
RLAPI void rlUnloadShaderBuffer(unsigned int ssboId);                           // Unload shader storage buffer object (SSBO)
RLAPI void rlUpdateShaderBuffer(unsigned int id, const void *data, unsigned int dataSize, unsigned int offset); // Update SSBO buffer data
RLAPI void rlBindShaderBuffer(unsigned int id, unsigned int index);             // Bind SSBO buffer
RLAPI void rlReadShaderBuffer(unsigned int id, void *dest, unsigned int count, unsigned int offset); // Read SSBO buffer data (GPU->CPU)
RLAPI void rlCopyShaderBuffer(unsigned int destId, unsigned int srcId, unsigned int destOffset, unsigned int srcOffset, unsigned int count); // Copy SSBO data between buffers
RLAPI unsigned int rlGetShaderBufferSize(unsigned int id);                      // Get SSBO buffer size

// Buffer management
RLAPI void rlBindImageTexture(unsigned int id, unsigned int index, int format, bool readonly);  // Bind image texture

// Matrix state management
RLAPI Matrix rlGetMatrixModelview(void);                                  // Get internal modelview matrix
RLAPI Matrix rlGetMatrixProjection(void);                                 // Get internal projection matrix
RLAPI Matrix rlGetMatrixTransform(void);                                  // Get internal accumulated transform matrix
RLAPI Matrix rlGetMatrixProjectionStereo(int eye);                        // Get internal projection matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye);                        // Get internal view offset matrix for stereo render (selected eye)
RLAPI void rlSetMatrixProjection(Matrix proj);                            // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void rlSetMatrixModelview(Matrix view);                             // Set a custom modelview matrix (replaces internal modelview matrix)
RLAPI void rlSetMatrixProjectionStereo(Matrix right, Matrix left);        // Set eyes projection matrices for stereo rendering
RLAPI void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left);        // Set eyes view offsets matrices for stereo rendering

// Quick and dirty cube/quad buffers load->draw->unload
RLAPI void rlLoadDrawCube(void);     // Load and draw a cube
RLAPI void rlLoadDrawQuad(void);     // Load and draw a quad

#if defined(__cplusplus)
}
#endif

#endif // RLGL_H

/***********************************************************************************
*
*   RLGL IMPLEMENTATION
*
************************************************************************************/

#if defined(RLGL_IMPLEMENTATION)

// Expose OpenGL functions from glad in raylib
#if defined(BUILD_LIBTYPE_SHARED)
    #define GLAD_API_CALL_EXPORT
    #define GLAD_API_CALL_EXPORT_BUILD
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(GRAPHICS_API_OPENGL_SOFTWARE)
        #define RLSW_IMPLEMENTATION
        #define SW_MALLOC(sz) RL_MALLOC(sz)
        #define SW_CALLOC(n,sz) RL_CALLOC(n, sz)
        #define SW_REALLOC(ptr, newSz) RL_REALLOC(ptr, newSz)
        #define SW_FREE(ptr) RL_FREE(ptr)
        #include "external/rlsw.h"      // OpenGL 1.1 software implementation
    #else
        #if defined(__APPLE__)
            #include <OpenGL/gl.h>      // OpenGL 1.1 library for OSX
            #include <OpenGL/glext.h>   // OpenGL extensions library
        #else
            // APIENTRY for OpenGL function pointer declarations is required
            #if !defined(APIENTRY)
                #if defined(_WIN32)
                    #define APIENTRY __stdcall
                #else
                    #define APIENTRY
                #endif
            #endif
            // WINGDIAPI definition. Some Windows OpenGL headers need it
            #if !defined(WINGDIAPI) && defined(_WIN32)
                #define WINGDIAPI __declspec(dllimport)
            #endif

            #include <GL/gl.h>          // OpenGL 1.1 library
        #endif
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_33)
    #define GLAD_MALLOC RL_MALLOC
    #define GLAD_FREE RL_FREE

    #define GLAD_GL_IMPLEMENTATION
    #include "external/glad.h"          // GLAD extensions loading library, includes OpenGL headers
#endif

#if defined(GRAPHICS_API_OPENGL_ES3)
    #include <GLES3/gl3.h>              // OpenGL ES 3.0 library
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2ext.h>           // OpenGL ES 2.0 extensions library
#elif defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: OpenGL ES 2.0 can be enabled on Desktop platforms,
    // in that case, functions are loaded from a custom glad for OpenGL ES 2.0
    // TODO: rlgl OpenGL ES 2.0 selection shouldn't depend on raylib platform macros, a specific GLAD shouldn't be neither required
    #if defined(PLATFORM_DESKTOP_GLFW) || defined(PLATFORM_DESKTOP_SDL)
        #define GLAD_GLES2_IMPLEMENTATION
        #include "external/glad_gles2.h"
    #else
        #define GL_GLEXT_PROTOTYPES
        //#include <EGL/egl.h>          // EGL library -> not required, platform layer
        #include <GLES2/gl2.h>          // OpenGL ES 2.0 library
        #include <GLES2/gl2ext.h>       // OpenGL ES 2.0 extensions library
    #endif

    // It seems OpenGL ES 2.0 instancing entry points are not defined on Raspberry Pi
    // provided headers (despite being defined in official Khronos GLES2 headers)
    // TODO: Avoid raylib platform-dependent code on rlgl, it should be a completely portable library
    #if defined(PLATFORM_DRM)           // Use CONFIG_DRM?
    typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
    #endif
#endif

#include <stdlib.h>             // Required for: calloc(), free()
#include <string.h>             // Required for: strcmp(), strlen() [Used in rlglInit(), on extensions loading]
#include <math.h>               // Required for: sqrtf(), sinf(), cosf(), floor(), log()
#include <limits.h>             // Required for: INT_MAX

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef PI
    #define PI 3.14159265358979323846f
#endif
#ifndef DEG2RAD
    #define DEG2RAD (PI/180.0f)
#endif
#ifndef RAD2DEG
    #define RAD2DEG (180.0f/PI)
#endif

#ifndef GL_SHADING_LANGUAGE_VERSION
    #define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#endif

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3
#endif
#ifndef GL_ETC1_RGB8_OES
    #define GL_ETC1_RGB8_OES                    0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
    #define GL_COMPRESSED_RGB8_ETC2             0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
    #define GL_COMPRESSED_RGBA8_ETC2_EAC        0x9278
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  0x8C00
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    #define GL_COMPRESSED_RGBA_ASTC_4x4_KHR     0x93b0
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x8_KHR
    #define GL_COMPRESSED_RGBA_ASTC_8x8_KHR     0x93b7
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
#endif

#ifndef GL_PROGRAM_POINT_SIZE
    #define GL_PROGRAM_POINT_SIZE               0x8642
#endif

#ifndef GL_LINE_WIDTH
    #define GL_LINE_WIDTH                       0x0B21
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    #define GL_UNSIGNED_SHORT_5_6_5             0x8363
    #define GL_UNSIGNED_SHORT_5_5_5_1           0x8034
    #define GL_UNSIGNED_SHORT_4_4_4_4           0x8033
#endif

#if defined(GRAPHICS_API_OPENGL_21)
    #define GL_LUMINANCE                        0x1909
    #define GL_LUMINANCE_ALPHA                  0x190A
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    #define glClearDepth                 glClearDepthf
    #if !defined(GRAPHICS_API_OPENGL_ES3)
        #define GL_READ_FRAMEBUFFER         GL_FRAMEBUFFER
        #define GL_DRAW_FRAMEBUFFER         GL_FRAMEBUFFER
        #define GL_DRAW_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING
    #endif
#endif

// Default shader vertex attribute names to set location points
// WARNING: Pre-defined names can not be changed, they are used by default shaders and all raylib examples shaders, they are just listed here for reference
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION          "vertexPosition"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD          "vertexTexCoord"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL            "vertexNormal"      // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR             "vertexColor"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT           "vertexTangent"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2         "vertexTexCoord2"   // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEINDICES       "vertexBoneIndices" // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS       "vertexBoneWeights" // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS
#define RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCETRANSFORM "instanceTransform" // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM

#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP              "mvp"               // model-view-projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW             "matView"           // view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION       "matProjection"     // projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL            "matModel"          // model matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL           "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR            "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define RL_DEFAULT_SHADER_UNIFORM_NAME_BONEMATRICES     "boneMatrices"      // bone matrices (required for GPU skinning)

#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0       "texture0"          // texture0 (texture slot active 0)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1       "texture1"          // texture1 (texture slot active 1)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2       "texture2"          // texture2 (texture slot active 2)

//----------------------------------------------------------------------------------
// Module Types and Structures Definition
//----------------------------------------------------------------------------------

#if defined(GRAPHICS_API_OPENGL_ES2) && !defined(GRAPHICS_API_OPENGL_ES3)
// VAO functions entry points
// NOTE: VAO functionality is exposed through extensions (OES)
static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray = NULL;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays = NULL;

// Instancing functionality entry points
// NOTE: Instancing functionality could be available through extensions
static PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstanced = NULL;
static PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstanced = NULL;
static PFNGLVERTEXATTRIBDIVISOREXTPROC glVertexAttribDivisor = NULL;
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// Set the viewport area (transformation from normalized device coordinates to window coordinates)
void rlViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

// Set clip planes distances
void rlSetClipPlanes(float nearPlane, float farPlane)
{
    rlCullDistanceNear = nearPlane;
    rlCullDistanceFar = farPlane;
}

// Get cull plane distance near
float rlGetCullDistanceNear(void)
{
    return rlCullDistanceNear;
}

// Get cull plane distance far
float rlGetCullDistanceFar(void)
{
    return rlCullDistanceFar;
}

//--------------------------------------------------------------------------------------
// Module Functions Definition - OpenGL style functions (common to 1.1, 3.3+, ES2)
//--------------------------------------------------------------------------------------

// Select and active a texture slot
void rlActiveTextureSlot(int slot)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glActiveTexture(GL_TEXTURE0 + slot);
#endif
}

// Enable texture
void rlEnableTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, id);
}

// Disable texture
void rlDisableTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Enable texture cubemap
void rlEnableTextureCubemap(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
#endif
}

// Disable texture cubemap
void rlDisableTextureCubemap(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif
}

// Set texture parameters (wrap mode/filter mode)
void rlTextureParameters(unsigned int id, int param, int value)
{
    glBindTexture(GL_TEXTURE_2D, id);

    switch (param)
    {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
        {
            if (value == RL_TEXTURE_WRAP_MIRROR_CLAMP)
            {
#if !defined(GRAPHICS_API_OPENGL_11)
                if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_2D, param, value);
                else TRACELOG(RL_LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
#endif
            }
            else glTexParameteri(GL_TEXTURE_2D, param, value);
        } break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_2D, param, value); break;
        case RL_TEXTURE_FILTER_ANISOTROPIC:
        {
#if !defined(GRAPHICS_API_OPENGL_11)
            // Reset anisotropy filter, in case it was set
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);

            if (value <= RLGL.ExtSupported.maxAnisotropyLevel) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            else if (RLGL.ExtSupported.maxAnisotropyLevel > 0.0f)
            {
                TRACELOG(RL_LOG_WARNING, "GL: Maximum anisotropic filter level supported is %iX", id, (int)RLGL.ExtSupported.maxAnisotropyLevel);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            }
            else TRACELOG(RL_LOG_WARNING, "GL: Anisotropic filtering not supported");
#endif
        } break;
#if defined(GRAPHICS_API_OPENGL_33)
        case RL_TEXTURE_MIPMAP_BIAS_RATIO: glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, value/100.0f);
#endif
        default: break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Set cubemap parameters (wrap mode/filter mode)
void rlCubemapParameters(unsigned int id, int param, int value)
{
#if !defined(GRAPHICS_API_OPENGL_11)
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    // Reset anisotropy filter, in case it was set
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);

    switch (param)
    {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
        {
            if (value == RL_TEXTURE_WRAP_MIRROR_CLAMP)
            {
                if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_CUBE_MAP, param, value);
                else TRACELOG(RL_LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
            }
            else glTexParameteri(GL_TEXTURE_CUBE_MAP, param, value);
        } break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_CUBE_MAP, param, value); break;
        case RL_TEXTURE_FILTER_ANISOTROPIC:
        {
            if (value <= RLGL.ExtSupported.maxAnisotropyLevel) glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            else if (RLGL.ExtSupported.maxAnisotropyLevel > 0.0f)
            {
                TRACELOG(RL_LOG_WARNING, "GL: Maximum anisotropic filter level supported is %iX", id, (int)RLGL.ExtSupported.maxAnisotropyLevel);
                glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            }
            else TRACELOG(RL_LOG_WARNING, "GL: Anisotropic filtering not supported");
        } break;
#if defined(GRAPHICS_API_OPENGL_33)
        case RL_TEXTURE_MIPMAP_BIAS_RATIO: glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_LOD_BIAS, value/100.0f);
#endif
        default: break;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif
}

// Enable shader program
void rlEnableShader(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(id);
#endif
}

// Disable shader program
void rlDisableShader(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(0);
#endif
}

// Enable rendering to texture (fbo)
void rlEnableFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    glBindFramebuffer(GL_FRAMEBUFFER, id);
#endif
}

// return the active render texture (fbo)
unsigned int rlGetActiveFramebuffer(void)
{
    GLint fboId = 0;
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboId);
#endif
    return fboId;
}

// Disable rendering to texture
void rlDisableFramebuffer(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Blit active framebuffer to main framebuffer
void rlBlitFramebuffer(int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight, int bufferMask)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES3))
    glBlitFramebuffer(srcX, srcY, srcWidth, srcHeight, dstX, dstY, dstWidth, dstHeight, bufferMask, GL_NEAREST);
#endif
}

// Bind framebuffer object (fbo)
void rlBindFramebuffer(unsigned int target, unsigned int framebuffer)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    glBindFramebuffer(target, framebuffer);
#endif
}

// Activate multiple draw color buffers
// NOTE: One color buffer is always active by default
void rlActiveDrawBuffers(int count)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES3))
    // NOTE: Maximum number of draw buffers supported is implementation dependent,
    // it can be queried with glGet*() but it must be at least 8
    //GLint maxDrawBuffers = 0;
    //glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    if (count > 0)
    {
        if (count > 8) TRACELOG(RL_LOG_WARNING, "GL: Max color buffers limited to 8");
        else
        {
            unsigned int buffers[8] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3,
                GL_COLOR_ATTACHMENT4,
                GL_COLOR_ATTACHMENT5,
                GL_COLOR_ATTACHMENT6,
                GL_COLOR_ATTACHMENT7,
            };

            glDrawBuffers(count, buffers);
        }
    }
    else TRACELOG(RL_LOG_WARNING, "GL: One color buffer active by default");
#endif
}

//----------------------------------------------------------------------------------
// General render state configuration
//----------------------------------------------------------------------------------

// Enable color blending
void rlEnableColorBlend(void) { glEnable(GL_BLEND); }

// Disable color blending
void rlDisableColorBlend(void) { glDisable(GL_BLEND); }

// Enable depth test
void rlEnableDepthTest(void) { glEnable(GL_DEPTH_TEST); }

// Disable depth test
void rlDisableDepthTest(void) { glDisable(GL_DEPTH_TEST); }

// Enable depth write
void rlEnableDepthMask(void) { glDepthMask(GL_TRUE); }

// Disable depth write
void rlDisableDepthMask(void) { glDepthMask(GL_FALSE); }

// Enable backface culling
void rlEnableBackfaceCulling(void) { glEnable(GL_CULL_FACE); }

// Disable backface culling
void rlDisableBackfaceCulling(void) { glDisable(GL_CULL_FACE); }

// Set color mask active for screen read/draw
void rlColorMask(bool r, bool g, bool b, bool a) { glColorMask(r, g, b, a); }

// Set face culling mode
void rlSetCullFace(int mode)
{
    switch (mode)
    {
        case RL_CULL_FACE_BACK: glCullFace(GL_BACK); break;
        case RL_CULL_FACE_FRONT: glCullFace(GL_FRONT); break;
        default: break;
    }
}

// Enable scissor test
void rlEnableScissorTest(void) { glEnable(GL_SCISSOR_TEST); }

// Disable scissor test
void rlDisableScissorTest(void) { glDisable(GL_SCISSOR_TEST); }

// Scissor test
void rlScissor(int x, int y, int width, int height) { glScissor(x, y, width, height); }

// Enable wire mode
void rlEnableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
}

// Disable wire mode
void rlDisableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

// Enable point mode
void rlEnablePointMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif
}

// Disable point mode
void rlDisablePointMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

// Set the line drawing width
void rlSetLineWidth(float width) { glLineWidth(width); }

// Get the line drawing width
float rlGetLineWidth(void)
{
    float width = 0;
    glGetFloatv(GL_LINE_WIDTH, &width);
    return width;
}

// Set the point drawing size
void rlSetPointSize(float size)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glPointSize(size);
#endif
}

// Get the point drawing size
float rlGetPointSize(void)
{
    float size = 1;
#if defined(GRAPHICS_API_OPENGL_11)
    glGetFloatv(GL_POINT_SIZE, &size);
#endif
    return size;

}

// Enable line aliasing
void rlEnableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_LINE_SMOOTH);
#endif
}

// Disable line aliasing
void rlDisableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_LINE_SMOOTH);
#endif
}

// Enable stereo rendering
void rlEnableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = true;
#endif
}

// Disable stereo rendering
void rlDisableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = false;
#endif
}

// Check if stereo render is enabled
bool rlIsStereoRenderEnabled(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    return RLGL.State.stereoRender;
#else
    return false;
#endif
}

// Clear color buffer with color
void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    // Color values clamp to 0.0f(0) and 1.0f(255)
    float cr = (float)r/255;
    float cg = (float)g/255;
    float cb = (float)b/255;
    float ca = (float)a/255;

    glClearColor(cr, cg, cb, ca);
        }

// Clear used screen buffers (color and depth)
void rlClearScreenBuffers(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers: Color and Depth (Depth is used for 3D)
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     // Stencil buffer not used...
    }

// Check and log OpenGL error codes
bool rlCheckErrors(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    bool had_errors = false;

    int check = 1;
    while (check)
    {
        const GLenum err = glGetError();
        if (err != GL_NO_ERROR) had_errors = true;
        switch (err)
    {
            case GL_NO_ERROR: check = 0; break;
            case 0x0500: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_ENUM"); break;
            case 0x0501: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_VALUE"); break;
            case 0x0502: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_OPERATION"); break;
            case 0x0503: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_STACK_OVERFLOW"); break;
            case 0x0504: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_STACK_UNDERFLOW"); break;
            case 0x0505: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_OUT_OF_MEMORY"); break;
            case 0x0506: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_FRAMEBUFFER_OPERATION"); break;
            default: TRACELOG(RL_LOG_WARNING, "GL: Error detected: Unknown error code: %x", err); break;
        }
    }

    return had_errors;
#endif
}

// Set blend mode
void rlSetBlendMode(int mode)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (mode)
    {
        case RL_BLEND_ALPHA: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
        case RL_BLEND_ADDITIVE: glBlendFunc(GL_SRC_ALPHA, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
        case RL_BLEND_MULTIPLIED: glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
        case RL_BLEND_ADD_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
        case RL_BLEND_SUBTRACT_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_SUBTRACT); break;
        case RL_BLEND_ALPHA_PREMULTIPLY: glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
        default: break;
    }
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - OpenGL Debug
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_43) && RLGL_ENABLE_OPENGL_DEBUG_CONTEXT
static void GLAPIENTRY rlDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    // Ignore non-significant error/warning codes (NVidia drivers)
    // NOTE: Here there are the details with a sample output:
    // - #131169 - Framebuffer detailed info: The driver allocated storage for renderbuffer 2. (severity: low)
    // - #131185 - Buffer detailed info: Buffer object 1 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_ENUM_88e4)
    //             will use VIDEO memory as the source for buffer object operations. (severity: low)
    // - #131218 - Program/shader state performance warning: Vertex shader in program 7 is being recompiled based on GL state. (severity: medium)
    // - #131204 - Texture state usage warning: The texture object (0) bound to texture image unit 0 does not have
    //             a defined base level and cannot be used for texture mapping. (severity: low)
    if ((id == 131169) || (id == 131185) || (id == 131218) || (id == 131204)) return;
    
    const char *msgSource = NULL;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API: msgSource = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: msgSource = "WINDOW_SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: msgSource = "SHADER_COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: msgSource = "THIRD_PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION: msgSource = "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER: msgSource = "OTHER"; break;
        default: break;
}

    const char *msgType = NULL;
    switch (type)
{
        case GL_DEBUG_TYPE_ERROR: msgType = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: msgType = "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: msgType = "UNDEFINED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY: msgType = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: msgType = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_MARKER: msgType = "MARKER"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: msgType = "PUSH_GROUP"; break;
        case GL_DEBUG_TYPE_POP_GROUP: msgType = "POP_GROUP"; break;
        case GL_DEBUG_TYPE_OTHER: msgType = "OTHER"; break;
        default: break;
    }

    const char *msgSeverity = "DEFAULT";
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_LOW: msgSeverity = "LOW"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: msgSeverity = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_HIGH: msgSeverity = "HIGH"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: msgSeverity = "NOTIFICATION"; break;
        default: break;
    }
    
    TRACELOG(RL_LOG_WARNING, "GL: OpenGL debug message: %s", message);
    TRACELOG(RL_LOG_WARNING, "    > Type: %s", msgType);
    TRACELOG(RL_LOG_WARNING, "    > Source = %s", msgSource);
    TRACELOG(RL_LOG_WARNING, "    > Severity = %s", msgSeverity);
}
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - rlgl functionality
//----------------------------------------------------------------------------------

// Get current OpenGL version
int rlGetVersion(void)
{
    int glVersion = 0;

#if defined(GRAPHICS_API_OPENGL_SOFTWARE)
    glVersion = RL_OPENGL_SOFTWARE;
#elif defined(GRAPHICS_API_OPENGL_11)
    glVersion = RL_OPENGL_11;
#endif
#if defined(GRAPHICS_API_OPENGL_21)
    glVersion = RL_OPENGL_21;
#elif defined(GRAPHICS_API_OPENGL_43)
    glVersion = RL_OPENGL_43;
#elif defined(GRAPHICS_API_OPENGL_33)
    glVersion = RL_OPENGL_33;
#endif
#if defined(GRAPHICS_API_OPENGL_ES3)
    glVersion = RL_OPENGL_ES_30;
#elif defined(GRAPHICS_API_OPENGL_ES2)
    glVersion = RL_OPENGL_ES_20;
#endif

    return glVersion;
}

// Textures data management
//-----------------------------------------------------------------------------------------
// Convert image data to OpenGL texture (returns OpenGL valid Id)
unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount)
{
    unsigned int id = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return id; }

    glBindTexture(GL_TEXTURE_2D, 0);    // Free any old binding

    // Check texture format support by OpenGL 1.1 (compressed textures not supported)
#if defined(GRAPHICS_API_OPENGL_11)
    if (format >= RL_PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        TRACELOG(RL_LOG_WARNING, "GL: OpenGL 1.1 does not support GPU compressed texture formats");
        return id;
    }
#else
    if ((!RLGL.ExtSupported.texCompDXT) && ((format == RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA) ||
        (format == RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA) || (format == RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: DXT compressed texture format not supported");
        return id;
    }
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((!RLGL.ExtSupported.texCompETC1) && (format == RL_PIXELFORMAT_COMPRESSED_ETC1_RGB))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ETC1 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompETC2) && ((format == RL_PIXELFORMAT_COMPRESSED_ETC2_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ETC2 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompPVRT) && ((format == RL_PIXELFORMAT_COMPRESSED_PVRT_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: PVRT compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompASTC) && ((format == RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA) || (format == RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ASTC compressed texture format not supported");
        return id;
    }
#endif
#endif // GRAPHICS_API_OPENGL_11

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &id);              // Generate texture id

    glBindTexture(GL_TEXTURE_2D, id);

    int mipWidth = width;
    int mipHeight = height;
    int mipOffset = 0;          // Mipmap data offset, only used for tracelog
    (void)mipOffset;            // Used to avoid gcc warnings about unused variable

    // NOTE: Added pointer math separately from function to avoid UBSAN complaining
    unsigned char *dataPtr = NULL;
    if (data != NULL) dataPtr = (unsigned char *)data;

    // Load the different mipmap levels
    for (int i = 0; i < mipmapCount; i++)
    {
        unsigned int mipSize = rlGetPixelDataSize(mipWidth, mipHeight, format);

        unsigned int glInternalFormat, glFormat, glType;
        rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

        TRACELOG(RL_LOG_DEBUG, "TEXTURE: Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

        if (glInternalFormat != 0)
        {
            if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, dataPtr);
#if !defined(GRAPHICS_API_OPENGL_11)
            else glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, mipSize, dataPtr);
#endif

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += mipSize; // Increment offset position to next mipmap
        if (data != NULL) dataPtr += mipSize; // Increment data pointer to next mipmap

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    // Texture parameters configuration
    // NOTE: glTexParameteri does NOT affect texture uploading
#if defined(GRAPHICS_API_OPENGL_ES2)

    // Check if texture is power-of-two (POT)
    bool texIsPOT = false;

    if (((width > 0) && ((width & (width - 1)) == 0)) &&
        ((height > 0) && ((height & (height - 1)) == 0))) texIsPOT = true;

    // NOTE: OpenGL ES 2.0 with no GL_OES_texture_npot support (i.e. WebGL) has limited NPOT support, so CLAMP_TO_EDGE must be used
    if ((texIsPOT) || (RLGL.ExtSupported.texNPOT))
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
    }
    else
    {
        // NOTE: If using negative texture coordinates (LoadOBJ()), it does not work!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);       // Set texture to clamp on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);       // Set texture to clamp on y-axis
    }
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
#endif

    // Magnification and minification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR

#if defined(GRAPHICS_API_OPENGL_33)
    if (mipmapCount > 1)
    {
        // Activate trilinear filtering if mipmaps are available
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        // Define the maximum number of mipmap levels to be used, 0 is base texture size
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapCount - 1);

        // Check if the loaded texture with mipmaps is complete,
        // uncomplete textures will draw in black if mipmap filtering is required
        //GLint complete = 0;
        //glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_FORMAT, &complete);
    }
#endif

    // At this point texture is loaded in GPU and texture parameters configured

    // NOTE: If mipmaps were not in data, they are not generated automatically

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    if (id > 0) TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Texture loaded successfully (%ix%i | %s | %i mipmaps)", id, width, height, rlGetPixelFormatName(format), mipmapCount);
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load texture");

    return id;
}

// Load depth texture/renderbuffer (to be attached to fbo)
// WARNING: OpenGL ES 2.0 requires GL_OES_depth_texture and WebGL requires WEBGL_depth_texture extensions
unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer)
{
    unsigned int id = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return id; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // In case depth textures were not supported, force renderbuffer usage
    if (!RLGL.ExtSupported.texDepth) useRenderBuffer = true;

    // NOTE: Letting the implementation to choose the best bit-depth
    // Possible formats: GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32 and GL_DEPTH_COMPONENT32F
    unsigned int glInternalFormat = GL_DEPTH_COMPONENT;

#if defined(GRAPHICS_API_OPENGL_ES2)
    // WARNING: WebGL platform requires unsized internal format definition (GL_DEPTH_COMPONENT)
    // while other platforms using OpenGL ES 2.0 require/support sized internal formats depending on the GPU capabilities
    if (!RLGL.ExtSupported.texDepthWebGL || useRenderBuffer)
    {
        if (RLGL.ExtSupported.maxDepthBits == 32) glInternalFormat = GL_DEPTH_COMPONENT32_OES;
        else if (RLGL.ExtSupported.maxDepthBits == 24) glInternalFormat = GL_DEPTH_COMPONENT24_OES;
        else glInternalFormat = GL_DEPTH_COMPONENT16;
    }
#endif
#if defined(GRAPHICS_API_OPENGL_ES3)
    // NOTE: This sized internal format should also work for WebGL 2.0
    // WARNING: Specification only allows GL_DEPTH_COMPONENT32F for GL_FLOAT type
    // REF: https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml
    if (RLGL.ExtSupported.maxDepthBits == 24) glInternalFormat = GL_DEPTH_COMPONENT24;
    else glInternalFormat = GL_DEPTH_COMPONENT16;
#endif

    if (!useRenderBuffer && RLGL.ExtSupported.texDepth)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        TRACELOG(RL_LOG_INFO, "TEXTURE: Depth texture loaded successfully");
    }
    else
    {
        // Create the renderbuffer that will serve as the depth attachment for the framebuffer
        // NOTE: A renderbuffer is simpler than a texture and could offer better performance on embedded devices
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Depth renderbuffer loaded successfully (%i bits)", id, (RLGL.ExtSupported.maxDepthBits >= 24)? RLGL.ExtSupported.maxDepthBits : 16);
    }
#elif defined(GRAPHICS_API_OPENGL_SOFTWARE)
    // NOTE: Renderbuffers are the same type of object as textures in rlsw
    // WARNING: Ensure that the depth format is the one specified at rlsw compilation
    glGenRenderbuffers(1, &id);
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
#endif

    return id;
}

// Load texture cubemap
// NOTE: Cubemap data is expected to be 6 images in a single data array (one after the other),
// expected the following convention: +X, -X, +Y, -Y, +Z, -Z
unsigned int rlLoadTextureCubemap(const void *data, int size, int format, int mipmapCount)
{
    unsigned int id = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return id; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    int mipSize = size;

    // NOTE: Added pointer math separately from function to avoid UBSAN complaining
    unsigned char *dataPtr = NULL;
    if (data != NULL) dataPtr = (unsigned char *)data;

    unsigned int dataSize = rlGetPixelDataSize(size, size, format);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if (glInternalFormat != 0)
    {
        // Load cubemap faces/mipmaps
        for (int i = 0; i < 6*mipmapCount; i++)
        {
            int mipmapLevel = i/6;
            int face = i%6;

            if (data == NULL)
            {
                if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB)
                {
                    if ((format == RL_PIXELFORMAT_UNCOMPRESSED_R32) ||
                        (format == RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32) ||
                        (format == RL_PIXELFORMAT_UNCOMPRESSED_R16) ||
                        (format == RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16)) TRACELOG(RL_LOG_WARNING, "TEXTURES: Cubemap requested format not supported");
                    else glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipmapLevel, glInternalFormat, mipSize, mipSize, 0, glFormat, glType, NULL);
                }
                else TRACELOG(RL_LOG_WARNING, "TEXTURES: Empty cubemap creation does not support compressed format");
            }
            else
            {
                if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipmapLevel, glInternalFormat, mipSize, mipSize, 0, glFormat, glType, dataPtr + face*dataSize);
                else glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipmapLevel, glInternalFormat, mipSize, mipSize, 0, dataSize, dataPtr + face*dataSize);
            }

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
            if (face == 5)
            {
                mipSize /= 2;
                if (data != NULL) dataPtr += dataSize*6; // Increment data pointer to next mipmap

                // Security check for NPOT textures
                if (mipSize < 1) mipSize = 1;

                dataSize = rlGetPixelDataSize(mipSize, mipSize, format);
            }
        }
    }

    // Set cubemap texture sampling parameters
    if (mipmapCount > 1) glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GRAPHICS_API_OPENGL_33)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  // Flag not supported on OpenGL ES 2.0
#endif

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif

    if (id > 0) TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Cubemap texture loaded successfully (%ix%i)", id, size, size);
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load cubemap texture");

    return id;
}

// Update already loaded texture in GPU with new data
// WARNING: Not possible to know safely if internal texture format is the expected one...
void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data)
{
    glBindTexture(GL_TEXTURE_2D, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if ((glInternalFormat != 0) && (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, glFormat, glType, data);
    }
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] Failed to update for current texture format (%i)", id, format);
}

// Get OpenGL internal formats and data type from raylib PixelFormat
void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType)
{
    *glInternalFormat = 0;
    *glFormat = 0;
    *glType = 0;

    switch (format)
    {
    #if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_21) || defined(GRAPHICS_API_OPENGL_ES2)
        // NOTE: on OpenGL ES 2.0 (WebGL), internalFormat must match format and options allowed are: GL_LUMINANCE, GL_RGB, GL_RGBA
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_LUMINANCE_ALPHA; *glFormat = GL_LUMINANCE_ALPHA; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        #if !defined(GRAPHICS_API_OPENGL_11)
        #if defined(GRAPHICS_API_OPENGL_ES3)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F_EXT; *glFormat = GL_RED_EXT; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F_EXT; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F_EXT; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_R16F_EXT; *glFormat = GL_RED_EXT; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB16F_EXT; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA16F_EXT; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT; break;
        #else
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_FLOAT; break;            // NOTE: Requires extension OES_texture_float
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_FLOAT; break;                  // NOTE: Requires extension OES_texture_float
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;             // NOTE: Requires extension OES_texture_float
        #if defined(GRAPHICS_API_OPENGL_21)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_HALF_FLOAT_ARB; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT_ARB; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT_ARB; break;
        #else // GRAPHICS_API_OPENGL_ES2
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_HALF_FLOAT_OES; break;   // NOTE: Requires extension OES_texture_half_float
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT_OES; break;         // NOTE: Requires extension OES_texture_half_float
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT_OES; break;    // NOTE: Requires extension OES_texture_half_float
        #endif
        #endif
        #endif
    #elif defined(GRAPHICS_API_OPENGL_33)
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_R8; *glFormat = GL_RED; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_RG8; *glFormat = GL_RG; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB565; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB8; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGB5_A1; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA4; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA8; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F; *glFormat = GL_RED; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_R16F; *glFormat = GL_RED; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB16F; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA16F; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT; break;
    #endif
    #if !defined(GRAPHICS_API_OPENGL_11)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB: if (RLGL.ExtSupported.texCompETC1) *glInternalFormat = GL_ETC1_RGB8_OES; break;                      // NOTE: Requires OpenGL ES 2.0 or OpenGL 4.3
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGB8_ETC2; break;               // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; break;     // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;    // NOTE: Requires PowerVR GPU
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;  // NOTE: Requires PowerVR GPU
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
    #endif
        default: TRACELOG(RL_LOG_WARNING, "TEXTURE: Current format not supported (%i)", format); break;
    }
}

// Unload texture from GPU memory
void rlUnloadTexture(unsigned int id)
{
    glDeleteTextures(1, &id);
}

// Generate mipmap data for selected texture
// NOTE: Only supports GPU mipmap generation
void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int *mipmaps)
{
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_2D, id);

    // Check if texture is power-of-two (POT)
    bool texIsPOT = false;

    if (((width > 0) && ((width & (width - 1)) == 0)) &&
        ((height > 0) && ((height & (height - 1)) == 0))) texIsPOT = true;

    if ((texIsPOT) || (RLGL.ExtSupported.texNPOT))
    {
        //glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);   // Hint for mipmaps generation algorithm: GL_FASTEST, GL_NICEST, GL_DONT_CARE
        glGenerateMipmap(GL_TEXTURE_2D);    // Generate mipmaps automatically

        #define MIN(a,b) (((a)<(b))? (a):(b))
        #define MAX(a,b) (((a)>(b))? (a):(b))

        *mipmaps = 1 + (int)floor(log(MAX(width, height))/log(2));
        TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Mipmaps generated automatically, total: %i", id, *mipmaps);
    }
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps", id);

    glBindTexture(GL_TEXTURE_2D, 0);
#else
    TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] GPU mipmap generation not supported", id);
#endif
}

// Read texture pixel data
void *rlReadTexturePixels(unsigned int id, int width, int height, int format)
{
    void *pixels = NULL;

#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    glBindTexture(GL_TEXTURE_2D, id);

    // NOTE: Using texture id, some texture info can be retrieved (but not on OpenGL ES 2.0)
    // Possible texture info: GL_TEXTURE_RED_SIZE, GL_TEXTURE_GREEN_SIZE, GL_TEXTURE_BLUE_SIZE, GL_TEXTURE_ALPHA_SIZE
    //int width, height, format;
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

    // NOTE: Each row written to or read from by OpenGL pixel operations like glGetTexImage are aligned to a 4 byte boundary by default, which may add some padding
    // Use glPixelStorei to modify padding with the GL_[UN]PACK_ALIGNMENT setting
    // GL_PACK_ALIGNMENT affects operations that read from OpenGL memory (glReadPixels, glGetTexImage, etc.)
    // GL_UNPACK_ALIGNMENT affects operations that write to OpenGL memory (glTexImage, etc.)
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);
    unsigned int size = rlGetPixelDataSize(width, height, format);

    if ((glInternalFormat != 0) && (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        pixels = RL_CALLOC(size, 1);
        glGetTexImage(GL_TEXTURE_2D, 0, glFormat, glType, pixels);
    }
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] Data retrieval not suported for pixel format (%i)", id, format);

    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    // glGetTexImage() is not available on OpenGL ES 2.0
    // Texture width and height are required on OpenGL ES 2.0, there is no way to get it from texture id
    // Two possible Options:
    // 1 - Bind texture to color fbo attachment and glReadPixels()
    // 2 - Create an fbo, activate it, render quad with texture, glReadPixels()
    // Using Option 1, care for texture format on retrieval
    // NOTE: This behaviour could be conditioned by graphic driver...
    unsigned int fboId = rlLoadFramebuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach our texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);

#if defined(FBO_READ_TEXTURE_AS_RGBA)
    // Reading data as RGBA because FBO texture is configured as RGBA, despite binding another texture format
    pixels = RL_CALLOC(rlGetPixelDataSize(width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8), 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#else
    // Reading data as original texture format, in some platforms (RPI, Wasm) it works
    pixels = (unsigned char *)RL_MALLOC(rlGetPixelDataSize(width, height, format));
    unsigned int glInternalFormat = 0, glFormat = 0, glType = 0;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);
    glReadPixels(0, 0, width, height, glFormat, glType, pixels);
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clean up temporal fbo
    rlUnloadFramebuffer(fboId);
#endif

    return pixels;
}

// Copy framebuffer pixel data to internal buffer
void rlCopyFramebuffer(int x, int y, int width, int height, int format, void *pixels)
{
#if defined(GRAPHICS_API_OPENGL_SOFTWARE)
    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType); // Get OpenGL texture format
    swReadPixels(x, y, width, height, glFormat, glType, pixels);
#endif
}

// Resize internal framebuffer
void rlResizeFramebuffer(int width, int height)
{
#if defined(GRAPHICS_API_OPENGL_SOFTWARE)
    swResize(width, height);
#endif
}

// Read screen pixel data (color buffer)
unsigned char *rlReadScreenPixels(int width, int height)
{
    unsigned char *imgData = (unsigned char *)RL_CALLOC(width*height*4, sizeof(unsigned char));

    // NOTE: Buffer retrieved is GL_FRONT in single-buffered configurations
    // and GL_BACK in double-buffered configurations, make sure to call it at the end of frame
    //glReadBuffer(GL_BACK);

    // NOTE: glReadPixels() returns image flipped vertically -> (0,0) is the bottom left corner of the framebuffer
    // WARNING: Getting alpha channel! Be careful, it can be transparent if not cleared properly!
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgData);

    // Flip image vertically
    // NOTE: Alpha value has already been applied to RGB in framebuffer, not needed anymore
    for (int y = height - 1; y >= height/2; y--)
    {
        for (int x = 0; x < (width*4); x += 4)
        {
            unsigned int s = ((height - 1) - y)*width*4 + x;
            unsigned int e = y*width*4 + x;

            unsigned char r = imgData[s];
            unsigned char g = imgData[s+1];
            unsigned char b = imgData[s+2];

            imgData[s] = imgData[e];
            imgData[s+1] = imgData[e+1];
            imgData[s+2] = imgData[e+2];
            imgData[s+3] = 255; // Set alpha component value to 255 (no trasparent image retrieval)

            imgData[e] = r;
            imgData[e+1] = g;
            imgData[e+2] = b;
            imgData[e+3] = 255; // Ditto
        }
    }

    return imgData;     // NOTE: image data should be freed
}

// Framebuffer management (fbo)
//-----------------------------------------------------------------------------------------
// Load a framebuffer to be used for rendering
// NOTE: No textures attached
unsigned int rlLoadFramebuffer(void)
{
    unsigned int fboId = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return fboId; }

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    glGenFramebuffers(1, &fboId);         // Create the framebuffer object
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind any framebuffer
#endif

    return fboId;
}

// Attach color buffer texture to a framebuffer object (unloads previous attachment)
// NOTE: Attach type: 0-Color, 1-Depth renderbuffer, 2-Depth texture
void rlFramebufferAttach(unsigned int id, unsigned int texId, int attachType, int texType, int mipLevel)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    switch (attachType)
    {
        case RL_ATTACHMENT_COLOR_CHANNEL0:
        case RL_ATTACHMENT_COLOR_CHANNEL1:
        case RL_ATTACHMENT_COLOR_CHANNEL2:
        case RL_ATTACHMENT_COLOR_CHANNEL3:
        case RL_ATTACHMENT_COLOR_CHANNEL4:
        case RL_ATTACHMENT_COLOR_CHANNEL5:
        case RL_ATTACHMENT_COLOR_CHANNEL6:
        case RL_ATTACHMENT_COLOR_CHANNEL7:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_RENDERBUFFER, texId);
            else if (texType >= RL_ATTACHMENT_CUBEMAP_POSITIVE_X) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_CUBE_MAP_POSITIVE_X + texType, texId, mipLevel);
        } break;
        case RL_ATTACHMENT_DEPTH:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texId);
        } break;
        case RL_ATTACHMENT_STENCIL:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, texId);
        } break;
        default: break;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Verify render texture is complete
bool rlFramebufferComplete(unsigned int id)
{
    bool result = false;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_UNSUPPORTED: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer is unsupported", id); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete attachment", id); break;
#if defined(GRAPHICS_API_OPENGL_ES2)
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete dimensions", id); break;
#endif
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has a missing attachment", id); break;
            default: break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = (status == GL_FRAMEBUFFER_COMPLETE);
#endif

    return result;
}

// Unload framebuffer from GPU memory
// NOTE: All attached textures/cubemaps/renderbuffers are also deleted
void rlUnloadFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_SOFTWARE))
    // Query depth attachment to automatically delete texture/renderbuffer
    int depthType = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, id);   // Bind framebuffer to query depth texture type
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType);

    // WARNING: WebGL: INVALID_ENUM: getFramebufferAttachmentParameter: invalid parameter name
    // REF: https://registry.khronos.org/webgl/specs/latest/1.0/
    int depthId = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthId);

    unsigned int depthIdU = (unsigned int)depthId;
    if (depthType == GL_RENDERBUFFER) glDeleteRenderbuffers(1, &depthIdU);
    else if (depthType == GL_TEXTURE) glDeleteTextures(1, &depthIdU);

    // NOTE: If a texture object is deleted while its image is attached to the *currently bound* framebuffer,
    // the texture image is automatically detached from the currently bound framebuffer

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &id);

    TRACELOG(RL_LOG_INFO, "FBO: [ID %i] Unloaded framebuffer from VRAM (GPU)", id);
#endif
}

// Vertex data management
//-----------------------------------------------------------------------------------------
// Load a new attributes buffer
unsigned int rlLoadVertexBuffer(const void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return id; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}

// Load a new attributes element buffer
unsigned int rlLoadVertexBufferElements(const void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return id; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}

// Enable vertex buffer (VBO)
void rlEnableVertexBuffer(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, id);
#endif
}

// Disable vertex buffer (VBO)
void rlDisableVertexBuffer(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

// Enable vertex buffer element (VBO element)
void rlEnableVertexBufferElements(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
#endif
}

// Disable vertex buffer element (VBO element)
void rlDisableVertexBufferElements(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

// Update vertex buffer with new data
// NOTE: dataSize and offset must be provided in bytes
void rlUpdateVertexBuffer(unsigned int id, const void *data, int dataSize, int offset)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, dataSize, data);
#endif
}

// Update vertex buffer elements with new data
// NOTE: dataSize and offset must be provided in bytes
void rlUpdateVertexBufferElements(unsigned int id, const void *data, int dataSize, int offset)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, dataSize, data);
#endif
}

// Enable vertex array object (VAO)
bool rlEnableVertexArray(unsigned int vaoId)
{
    bool result = false;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(vaoId);
        result = true;
    }
#endif
    return result;
}

// Disable vertex array object (VAO)
void rlDisableVertexArray(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
#endif
}

// Enable vertex attribute index
void rlEnableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glEnableVertexAttribArray(index);
#endif
}

// Disable vertex attribute index
void rlDisableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDisableVertexAttribArray(index);
#endif
}

// Draw vertex array
void rlDrawVertexArray(int offset, int count)
{
    glDrawArrays(GL_TRIANGLES, offset, count);
}

// Draw vertex array elements
void rlDrawVertexArrayElements(int offset, int count, const void *buffer)
{
    // NOTE: Added pointer math separately from function to avoid UBSAN complaining
    unsigned short *bufferPtr = (unsigned short *)buffer;
    if (offset > 0) bufferPtr += offset;

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (const unsigned short *)bufferPtr);
}

// Draw vertex array instanced
void rlDrawVertexArrayInstanced(int offset, int count, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDrawArraysInstanced(GL_TRIANGLES, offset, count, instances);
#endif
}

// Draw vertex array elements instanced
void rlDrawVertexArrayElementsInstanced(int offset, int count, const void *buffer, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: Added pointer math separately from function to avoid UBSAN complaining
    unsigned short *bufferPtr = (unsigned short *)buffer;
    if (offset > 0) bufferPtr += offset;

    glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (const unsigned short *)bufferPtr, instances);
#endif
}

// Enable vertex state pointer
void rlEnableStatePointer(int vertexAttribType, void *buffer)
{
#if defined(GRAPHICS_API_OPENGL_11)
    if (buffer != NULL) glEnableClientState(vertexAttribType);
    switch (vertexAttribType)
    {
        case GL_VERTEX_ARRAY: glVertexPointer(3, GL_FLOAT, 0, buffer); break;
        case GL_TEXTURE_COORD_ARRAY: glTexCoordPointer(2, GL_FLOAT, 0, buffer); break;
        case GL_NORMAL_ARRAY: if (buffer != NULL) glNormalPointer(GL_FLOAT, 0, buffer); break;
        case GL_COLOR_ARRAY: if (buffer != NULL) glColorPointer(4, GL_UNSIGNED_BYTE, 0, buffer); break;
        //case GL_INDEX_ARRAY: if (buffer != NULL) glIndexPointer(GL_SHORT, 0, buffer); break; // Indexed colors
        default: break;
    }
#endif
}

// Disable vertex state pointer
void rlDisableStatePointer(int vertexAttribType)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glDisableClientState(vertexAttribType);
#endif
}

// Load vertex array object (VAO)
unsigned int rlLoadVertexArray(void)
{
    unsigned int vaoId = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return vaoId; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao) glGenVertexArrays(1, &vaoId);
#endif

    return vaoId;
}

// Set vertex attribute
void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, int offset)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: Data type could be: GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT
    // Additional types (depends on OpenGL version or extensions):
    //  - GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_FIXED,
    //  - GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV

    size_t offsetNative = offset;
    glVertexAttribPointer(index, compSize, type, normalized, stride, (void *)offsetNative);
#endif
}

// Set vertex attribute divisor
void rlSetVertexAttributeDivisor(unsigned int index, int divisor)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribDivisor(index, divisor);
#endif
}

// Unload vertex array object (VAO)
void rlUnloadVertexArray(unsigned int vaoId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vaoId);
        TRACELOG(RL_LOG_INFO, "VAO: [ID %i] Unloaded vertex array data from VRAM (GPU)", vaoId);
    }
#endif
}

// Unload vertex buffer (VBO)
void rlUnloadVertexBuffer(unsigned int vboId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteBuffers(1, &vboId);
    //TRACELOG(RL_LOG_INFO, "VBO: Unloaded vertex data from VRAM (GPU)");
#endif
}

// Shaders management
//-----------------------------------------------------------------------------------------------
// Load (compile) shader and return shader id
unsigned int rlLoadShader(const char *code, int type)
{
    unsigned int shaderId = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    shaderId = glCreateShader(type);
    glShaderSource(shaderId, 1, &code, NULL);

    GLint success = 0;
    glCompileShader(shaderId);
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile vertex shader code", shaderId); break;
            case GL_FRAGMENT_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile fragment shader code", shaderId); break;
            //case GL_GEOMETRY_SHADER:
        #if defined(GRAPHICS_API_OPENGL_43)
            case GL_COMPUTE_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile compute shader code", shaderId); break;
        #elif defined(GRAPHICS_API_OPENGL_33)
            case GL_COMPUTE_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: Compute shaders not enabled. Define GRAPHICS_API_OPENGL_43", shaderId); break;
        #endif
            default: break;
        }

        int maxLength = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)RL_CALLOC(maxLength, sizeof(char));
            glGetShaderInfoLog(shaderId, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shaderId, log);
            RL_FREE(log);
        }

        // Unload object allocated by glCreateShader(),
        // despite failing in the compilation process
        glDeleteShader(shaderId);
        shaderId = 0;
    }
    else
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Vertex shader compiled successfully", shaderId); break;
            case GL_FRAGMENT_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Fragment shader compiled successfully", shaderId); break;
            //case GL_GEOMETRY_SHADER:
        #if defined(GRAPHICS_API_OPENGL_43)
            case GL_COMPUTE_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Compute shader compiled successfully", shaderId); break;
        #elif defined(GRAPHICS_API_OPENGL_33)
            case GL_COMPUTE_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: Compute shaders not enabled. Define GRAPHICS_API_OPENGL_43", shaderId); break;
        #endif
            default: break;
        }
    }
#endif

    return shaderId;
}

// Load shader program from code strings
// NOTE: If shader string is NULL, using default vertex/fragment shaders
unsigned int rlLoadShaderProgram(const char *vsCode, const char *fsCode)
{
    unsigned int id = 0; // Shader program id
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return id; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int vertexShaderId = 0;
    unsigned int fragmentShaderId = 0;

    // Compile vertex shader (if provided)
    // NOTE: If not vertex shader is provided, use default one
    if (vsCode != NULL) vertexShaderId = rlLoadShader(vsCode, GL_VERTEX_SHADER);
    else vertexShaderId = RLGL.State.defaultVShaderId;

    // Compile fragment shader (if provided)
    // NOTE: If not vertex shader is provided, use default one
    if (fsCode != NULL) fragmentShaderId = rlLoadShader(fsCode, GL_FRAGMENT_SHADER);
    else fragmentShaderId = RLGL.State.defaultFShaderId;

    // In case vertex and fragment shader are the default ones, no need to recompile, assign the default shader program id
    if ((vertexShaderId == RLGL.State.defaultVShaderId) && (fragmentShaderId == RLGL.State.defaultFShaderId)) id = RLGL.State.defaultShaderId;
    else if ((vertexShaderId > 0) && (fragmentShaderId > 0))
    {
        // One of or both shader are new, a new shader program needs to be compiled
        id = rlLoadShaderProgramEx(vertexShaderId, fragmentShaderId);

        // Detaching and deleting vertex/fragment shaders (if not default ones)
        // WARNING: Detach shader before deletion to make sure memory is freed
        if (vertexShaderId != RLGL.State.defaultVShaderId)
        {
            // WARNING: Shader program linkage could fail and returned id is 0
            if (id > 0) glDetachShader(id, vertexShaderId);
            glDeleteShader(vertexShaderId);
        }
        if (fragmentShaderId != RLGL.State.defaultFShaderId)
        {
            // WARNING: Shader program linkage could fail and returned id is 0
            if (id > 0) glDetachShader(id, fragmentShaderId);
            glDeleteShader(fragmentShaderId);
        }

        // In case shader program loading failed, assign default shader
        if (id == 0)
        {
            // In case shader loading fails, reassigning default shader
            TRACELOG(RL_LOG_WARNING, "SHADER: Failed to load custom shader code, using default shader");
            id = RLGL.State.defaultShaderId;
        }
        /*
        else
        {
            // Get available shader uniforms
            // NOTE: This information is useful for debug...
            int uniformCount = -1;
            glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformCount);

            for (int i = 0; i < uniformCount; i++)
            {
                int namelen = -1;
                int num = -1;
                char name[256] = { 0 };     // Assume no variable names longer than 256
                GLenum type = GL_ZERO;

                // Get the name of the uniforms
                glGetActiveUniform(id, i, sizeof(name) - 1, &namelen, &num, &type, name);

                name[namelen] = 0;
                TRACELOG(RL_LOG_DEBUG, "SHADER: [ID %i] Active uniform (%s) set at location: %i", id, name, glGetUniformLocation(id, name));
            }
        }
        */
    }
#endif

    return id;
}

// Load shader program from already loaded shader ids
unsigned int rlLoadShaderProgramEx(unsigned int vsId, unsigned int fsId)
{
    unsigned int programId = 0;
    if (!isGpuReady) { TRACELOG(RL_LOG_WARNING, "GL: GPU is not ready to load data, trying to load before InitWindow()?"); return programId; }

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    GLint success = 0;
    programId = glCreateProgram();

    glAttachShader(programId, vsId);
    glAttachShader(programId, fsId);

    // Default attribute shader locations must be bound before linking
    // NOTE: There is no problem with binding a generic attribute index to an attribute variable name
    // that is never used; if some attrib name is no found on the shader, it locations becomes -1
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM, RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCETRANSFORM);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEINDICES);
    glBindAttribLocation(programId, RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS);

    glLinkProgram(programId);

    // NOTE: All uniform variables are intitialised to 0 when a program links

    glGetProgramiv(programId, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to link shader program", programId);

        int maxLength = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)RL_CALLOC(maxLength, sizeof(char));
            glGetProgramInfoLog(programId, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Link error: %s", programId, log);
            RL_FREE(log);
        }

        glDeleteProgram(programId);

        programId = 0;
    }
    else
    {
        // Get the size of compiled shader program (not available on OpenGL ES 2.0)
        // NOTE: If GL_LINK_STATUS is GL_FALSE, program binary length is zero
        //GLint binarySize = 0;
        //glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binarySize);

        TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Program shader loaded successfully", programId);
    }
#endif
    return programId;
}

// Load compute shader program
unsigned int rlLoadShaderProgramCompute(unsigned int csId)
{
    unsigned int programId = 0;

#if defined(GRAPHICS_API_OPENGL_43)
    GLint success = 0;
    programId = glCreateProgram();

    glAttachShader(programId, csId);

    glLinkProgram(programId);

    // NOTE: All uniform variables are intitialised to 0 when a program links

    glGetProgramiv(programId, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to link compute shader program", programId);

        int maxLength = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)RL_CALLOC(maxLength, sizeof(char));
            glGetProgramInfoLog(programId, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Link error: %s", programId, log);
            RL_FREE(log);
        }

        glDeleteProgram(programId);

        programId = 0;
    }
    else
    {
        // Get the size of compiled shader program (not available on OpenGL ES 2.0)
        // NOTE: If GL_LINK_STATUS is GL_FALSE, program binary length is zero
        //GLint binarySize = 0;
        //glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binarySize);

        TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Compute shader program loaded successfully", programId);
    }
#else
    TRACELOG(RL_LOG_WARNING, "SHADER: Compute shaders not supported, enable GRAPHICS_API_OPENGL_43");
#endif

    return programId;
}

// Delete shader
void rlUnloadShader(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteShader(id);

    TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Unloaded shader data from VRAM (GPU)", id);
#endif
}

// Unload shader program
void rlUnloadShaderProgram(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteProgram(id);

    TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Unloaded shader program data from VRAM (GPU)", id);
#endif
}

// Get shader location uniform
// NOTE: First parameter refers to shader program id
int rlGetLocationUniform(unsigned int id, const char *uniformName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetUniformLocation(id, uniformName);

    //if (location == -1) TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to find shader uniform: %s", shaderId, uniformName);
    //else TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Shader uniform (%s) set at location: %i", shaderId, uniformName, location);
#endif
    return location;
}

// Get shader location attribute
// NOTE: First parameter refers to shader program id
int rlGetLocationAttrib(unsigned int id, const char *attribName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetAttribLocation(id, attribName);

    //if (location == -1) TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to find shader attribute: %s", shaderId, attribName);
    //else TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Shader attribute (%s) set at location: %i", shaderId, attribName, location);
#endif
    return location;
}

// Set shader value uniform
void rlSetUniform(int locIndex, const void *value, int uniformType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (uniformType)
    {
        case RL_SHADER_UNIFORM_FLOAT: glUniform1fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_VEC2: glUniform2fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_VEC3: glUniform3fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_VEC4: glUniform4fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_INT: glUniform1iv(locIndex, count, (int *)value); break;
        case RL_SHADER_UNIFORM_IVEC2: glUniform2iv(locIndex, count, (int *)value); break;
        case RL_SHADER_UNIFORM_IVEC3: glUniform3iv(locIndex, count, (int *)value); break;
        case RL_SHADER_UNIFORM_IVEC4: glUniform4iv(locIndex, count, (int *)value); break;
    #if !defined(GRAPHICS_API_OPENGL_ES2)
        case RL_SHADER_UNIFORM_UINT: glUniform1uiv(locIndex, count, (unsigned int *)value); break;
        case RL_SHADER_UNIFORM_UIVEC2: glUniform2uiv(locIndex, count, (unsigned int *)value); break;
        case RL_SHADER_UNIFORM_UIVEC3: glUniform3uiv(locIndex, count, (unsigned int *)value); break;
        case RL_SHADER_UNIFORM_UIVEC4: glUniform4uiv(locIndex, count, (unsigned int *)value); break;
    #endif
        case RL_SHADER_UNIFORM_SAMPLER2D: glUniform1iv(locIndex, count, (int *)value); break;
        default: TRACELOG(RL_LOG_WARNING, "SHADER: Failed to set uniform value, data type not recognized");
    }
#endif
}

// Set shader value attribute
void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (attribType)
    {
        case RL_SHADER_ATTRIB_FLOAT: if (count == 1) glVertexAttrib1fv(locIndex, (float *)value); break;
        case RL_SHADER_ATTRIB_VEC2: if (count == 2) glVertexAttrib2fv(locIndex, (float *)value); break;
        case RL_SHADER_ATTRIB_VEC3: if (count == 3) glVertexAttrib3fv(locIndex, (float *)value); break;
        case RL_SHADER_ATTRIB_VEC4: if (count == 4) glVertexAttrib4fv(locIndex, (float *)value); break;
        default: TRACELOG(RL_LOG_WARNING, "SHADER: Failed to set attrib default value, data type not recognized");
    }
#endif
}

// Set shader value uniform matrix
void rlSetUniformMatrix(int locIndex, const float mat[16])
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glUniformMatrix4fv(locIndex, 1, false, mat);
#endif
}

// Set shader value uniform matrix
void rlSetUniformMatrices(int locIndex, const Matrix *matrices, int count)
{
#if defined(GRAPHICS_API_OPENGL_33)
    glUniformMatrix4fv(locIndex, count, true, (const float *)matrices);
#elif defined(GRAPHICS_API_OPENGL_ES2)
    // WARNING: WebGL does not support Matrix transpose ("true" parameter)
    // REF: https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/uniformMatrix
    glUniformMatrix4fv(locIndex, count, false, (const float *)matrices);
#endif
}

// Set shader value uniform sampler
void rlSetUniformSampler(int locIndex, unsigned int textureId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Check if texture is already active
    for (int i = 0; i < RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++)
    {
        if (RLGL.State.activeTextureId[i] == textureId)
        {
            glUniform1i(locIndex, 1 + i);
            return;
        }
    }

    // Register a new active texture for the internal batch system
    // NOTE: Default texture is always activated as GL_TEXTURE0
    for (int i = 0; i < RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++)
    {
        if (RLGL.State.activeTextureId[i] == 0)
        {
            glUniform1i(locIndex, 1 + i);              // Activate new texture unit
            RLGL.State.activeTextureId[i] = textureId; // Save texture id for binding on drawing
            break;
        }
    }
#endif
}

// Set shader currently active (id and locations)
void rlSetShader(unsigned int id, int *locs)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.State.currentShaderId != id)
    {
        rlDrawRenderBatch(RLGL.currentBatch);
        RLGL.State.currentShaderId = id;
        RLGL.State.currentShaderLocs = locs;
    }
#endif
}

// Dispatch compute shader (equivalent to *draw* for graphics pilepine)
void rlComputeShaderDispatch(unsigned int groupX, unsigned int groupY, unsigned int groupZ)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glDispatchCompute(groupX, groupY, groupZ);
#endif
}

// Load shader storage buffer object (SSBO)
unsigned int rlLoadShaderBuffer(unsigned int size, const void *data, int usageHint)
{
    unsigned int ssbo = 0;

#if defined(GRAPHICS_API_OPENGL_43)
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usageHint? usageHint : RL_STREAM_COPY);
    if (data == NULL) glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);    // Clear buffer data to 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#else
    TRACELOG(RL_LOG_WARNING, "SSBO: SSBO not enabled. Define GRAPHICS_API_OPENGL_43");
#endif

    return ssbo;
}

// Unload shader storage buffer object (SSBO)
void rlUnloadShaderBuffer(unsigned int ssboId)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glDeleteBuffers(1, &ssboId);
#else
    TRACELOG(RL_LOG_WARNING, "SSBO: SSBO not enabled. Define GRAPHICS_API_OPENGL_43");
#endif
}

// Update SSBO buffer data
void rlUpdateShaderBuffer(unsigned int id, const void *data, unsigned int dataSize, unsigned int offset)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, dataSize, data);
#endif
}

// Get SSBO buffer size
unsigned int rlGetShaderBufferSize(unsigned int id)
{
    unsigned int result = 0;
#if defined(GRAPHICS_API_OPENGL_43)
    GLint64 size = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glGetBufferParameteri64v(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &size);
    if (size > 0) result = (unsigned int)size;
#endif
    return result;
}

// Read SSBO buffer data (GPU->CPU)
void rlReadShaderBuffer(unsigned int id, void *dest, unsigned int count, unsigned int offset)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, count, dest);
#endif
}

// Bind SSBO buffer
void rlBindShaderBuffer(unsigned int id, unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
#endif
}

// Copy SSBO buffer data
void rlCopyShaderBuffer(unsigned int destId, unsigned int srcId, unsigned int destOffset, unsigned int srcOffset, unsigned int count)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBuffer(GL_COPY_READ_BUFFER, srcId);
    glBindBuffer(GL_COPY_WRITE_BUFFER, destId);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, destOffset, count);
#endif
}

// Bind image texture
void rlBindImageTexture(unsigned int id, unsigned int index, int format, bool readonly)
{
#if defined(GRAPHICS_API_OPENGL_43)
    unsigned int glInternalFormat = 0, glFormat = 0, glType = 0;

    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);
    glBindImageTexture(index, id, 0, 0, 0, readonly? GL_READ_ONLY : GL_READ_WRITE, glInternalFormat);
#else
    TRACELOG(RL_LOG_WARNING, "TEXTURE: Image texture binding not enabled. Define GRAPHICS_API_OPENGL_43");
#endif
}

// Get name string for pixel format
const char *rlGetPixelFormatName(unsigned int format)
{
    switch (format)
    {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: return "GRAYSCALE"; break;         // 8 bit per pixel (no alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: return "GRAY_ALPHA"; break;       // 8*2 bpp (2 channels)
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: return "R5G6B5"; break;               // 16 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: return "R8G8B8"; break;               // 24 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: return "R5G5B5A1"; break;           // 16 bpp (1 bit alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: return "R4G4B4A4"; break;           // 16 bpp (4 bit alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: return "R8G8B8A8"; break;           // 32 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: return "R32"; break;                     // 32 bpp (1 channel - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: return "R32G32B32"; break;         // 32*3 bpp (3 channels - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: return "R32G32B32A32"; break;   // 32*4 bpp (4 channels - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: return "R16"; break;                     // 16 bpp (1 channel - half float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: return "R16G16B16"; break;         // 16*3 bpp (3 channels - half float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: return "R16G16B16A16"; break;   // 16*4 bpp (4 channels - half float)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB: return "DXT1_RGB"; break;             // 4 bpp (no alpha)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: return "DXT1_RGBA"; break;           // 4 bpp (1 bit alpha)
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: return "DXT3_RGBA"; break;           // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: return "DXT5_RGBA"; break;           // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB: return "ETC1_RGB"; break;             // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB: return "ETC2_RGB"; break;             // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: return "ETC2_RGBA"; break;       // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB: return "PVRT_RGB"; break;             // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: return "PVRT_RGBA"; break;           // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: return "ASTC_4x4_RGBA"; break;   // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: return "ASTC_8x8_RGBA"; break;   // 2 bpp
        default: return "UNKNOWN"; break;
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

#if RLGL_SHOW_GL_DETAILS_INFO
// Get compressed format official GL identifier name
static const char *rlGetCompressedFormatName(int format)
{
    switch (format)
    {
        // GL_EXT_texture_compression_s3tc
        case 0x83F0: return "GL_COMPRESSED_RGB_S3TC_DXT1_EXT"; break;
        case 0x83F1: return "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT"; break;
        case 0x83F2: return "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT"; break;
        case 0x83F3: return "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT"; break;
        // GL_3DFX_texture_compression_FXT1
        case 0x86B0: return "GL_COMPRESSED_RGB_FXT1_3DFX"; break;
        case 0x86B1: return "GL_COMPRESSED_RGBA_FXT1_3DFX"; break;
        // GL_IMG_texture_compression_pvrtc
        case 0x8C00: return "GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG"; break;
        case 0x8C01: return "GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG"; break;
        case 0x8C02: return "GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG"; break;
        case 0x8C03: return "GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG"; break;
        // GL_OES_compressed_ETC1_RGB8_texture
        case 0x8D64: return "GL_ETC1_RGB8_OES"; break;
        // GL_ARB_texture_compression_rgtc
        case 0x8DBB: return "GL_COMPRESSED_RED_RGTC1"; break;
        case 0x8DBC: return "GL_COMPRESSED_SIGNED_RED_RGTC1"; break;
        case 0x8DBD: return "GL_COMPRESSED_RG_RGTC2"; break;
        case 0x8DBE: return "GL_COMPRESSED_SIGNED_RG_RGTC2"; break;
        // GL_ARB_texture_compression_bptc
        case 0x8E8C: return "GL_COMPRESSED_RGBA_BPTC_UNORM_ARB"; break;
        case 0x8E8D: return "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB"; break;
        case 0x8E8E: return "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB"; break;
        case 0x8E8F: return "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB"; break;
        // GL_ARB_ES3_compatibility
        case 0x9274: return "GL_COMPRESSED_RGB8_ETC2"; break;
        case 0x9275: return "GL_COMPRESSED_SRGB8_ETC2"; break;
        case 0x9276: return "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2"; break;
        case 0x9277: return "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2"; break;
        case 0x9278: return "GL_COMPRESSED_RGBA8_ETC2_EAC"; break;
        case 0x9279: return "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC"; break;
        case 0x9270: return "GL_COMPRESSED_R11_EAC"; break;
        case 0x9271: return "GL_COMPRESSED_SIGNED_R11_EAC"; break;
        case 0x9272: return "GL_COMPRESSED_RG11_EAC"; break;
        case 0x9273: return "GL_COMPRESSED_SIGNED_RG11_EAC"; break;
        // GL_KHR_texture_compression_astc_hdr
        case 0x93B0: return "GL_COMPRESSED_RGBA_ASTC_4x4_KHR"; break;
        case 0x93B1: return "GL_COMPRESSED_RGBA_ASTC_5x4_KHR"; break;
        case 0x93B2: return "GL_COMPRESSED_RGBA_ASTC_5x5_KHR"; break;
        case 0x93B3: return "GL_COMPRESSED_RGBA_ASTC_6x5_KHR"; break;
        case 0x93B4: return "GL_COMPRESSED_RGBA_ASTC_6x6_KHR"; break;
        case 0x93B5: return "GL_COMPRESSED_RGBA_ASTC_8x5_KHR"; break;
        case 0x93B6: return "GL_COMPRESSED_RGBA_ASTC_8x6_KHR"; break;
        case 0x93B7: return "GL_COMPRESSED_RGBA_ASTC_8x8_KHR"; break;
        case 0x93B8: return "GL_COMPRESSED_RGBA_ASTC_10x5_KHR"; break;
        case 0x93B9: return "GL_COMPRESSED_RGBA_ASTC_10x6_KHR"; break;
        case 0x93BA: return "GL_COMPRESSED_RGBA_ASTC_10x8_KHR"; break;
        case 0x93BB: return "GL_COMPRESSED_RGBA_ASTC_10x10_KHR"; break;
        case 0x93BC: return "GL_COMPRESSED_RGBA_ASTC_12x10_KHR"; break;
        case 0x93BD: return "GL_COMPRESSED_RGBA_ASTC_12x12_KHR"; break;
        case 0x93D0: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR"; break;
        case 0x93D1: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR"; break;
        case 0x93D2: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR"; break;
        case 0x93D3: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR"; break;
        case 0x93D4: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR"; break;
        case 0x93D5: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR"; break;
        case 0x93D6: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR"; break;
        case 0x93D7: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR"; break;
        case 0x93D8: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR"; break;
        case 0x93D9: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR"; break;
        case 0x93DA: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR"; break;
        case 0x93DB: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR"; break;
        case 0x93DC: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR"; break;
        case 0x93DD: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR"; break;
        default: return "GL_COMPRESSED_UNKNOWN"; break;
    }
}
#endif

#endif // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

// Get pixel data size in bytes (image or texture)
// NOTE: Size depends on pixel format
static int rlGetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel
    
    if ((width <= 0) || (height <= 0)) return dataSize;

    switch (format)
    {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: bpp = 16; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: bpp = 16*3; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: bpp = 16*4; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: // 8 bytes per each 4x4 block
        {
            int blockWidth = (width + 3)/4;
            int blockHeight = (height + 3)/4;
            unsigned long long dataSizeBytes = (unsigned long long)blockWidth*blockHeight*8;
            if (dataSizeBytes < INT_MAX) dataSize = (int)dataSizeBytes;

        } break;
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: // 16 bytes per each 4x4 block
        {
            int blockWidth = (width + 3)/4;
            int blockHeight = (height + 3)/4;
            unsigned long long dataSizeBytes = (unsigned long long)blockWidth*blockHeight*16;
            if (dataSizeBytes < INT_MAX) dataSize = (int)dataSizeBytes;

        } break;
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: // 4 bytes per each 4x4 block
        {
            int blockWidth = (width + 3)/4;
            int blockHeight = (height + 3)/4;
            unsigned long long dataSizeBytes = (unsigned long long)blockWidth*blockHeight*4;
            if (dataSizeBytes < INT_MAX) dataSize = (int)dataSizeBytes;

        } break;
        default: break;
    }

    // Compute dataSize for uncompressed texture data (no blocks)
    if ((format >= RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) &&
        (format <= RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16))
    {
        unsigned long long dataSizeBytes = ((unsigned long long)width*height*bpp) >> 3;  // Get size in bytes (dividing by 8)
        if (dataSizeBytes < INT_MAX) dataSize = (int)dataSizeBytes;
        else TRACELOG(LOG_WARNING, "Requested image size is larger than 2GB, it can not be allocated");
    }

    return dataSize;
}

#endif // RLGL_IMPLEMENTATION
