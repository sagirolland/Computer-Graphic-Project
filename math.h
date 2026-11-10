#if !defined(MATH_HEADER)
#define MATH_HEADER

#define EPSILON 1e-8f
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(a) (((a) >= 0) ? (a) : -(a))

#define CLAMP(a, min, max) MIN(MAX((a), (min)), (max))
#define LERP(a, t, b) ((1.f - (t))*(a) + (t)*(b))

typedef union Vec2i {
    struct { int x, y; };
    struct { int u, v; };
    struct { int s, t; };
    
    struct { int left, right; };
    struct { int width, height; };
    
    int values[2];
} Vec2i;

typedef union Vec3i {
    struct { int x, y, z; };
    struct { int r, g, b; };
    struct { int u, v, w; };
    
    struct { Vec2i xy; int _ignored0; };
    struct { Vec2i rg; int _ignored1; };
    struct { Vec2i uv; int _ignored2; };
    struct { int _ignored3; Vec2i yz; };
    struct { int _ignored4; Vec2i gb; };
    struct { int _ignored5; Vec2i vw; };
    
    int values[3];
} Vec3i;

typedef union Vec4i {
    struct { int x, y, z, w; };
    struct { int r, g, b, a; };
    
    struct { Vec3i xyz; int _ignored0; };
    struct { Vec3i rgb; int _ignored1; };
    
    struct { Vec2i xy; Vec2i zw; };
    struct { int _ignored2; Vec2i yz; int _ignored3; };
    struct { Vec2i rg; Vec2i ba; };
    struct { int _ignored4; Vec2i gb; int _ignored5; };
    
    int values[4];
} Vec4i;


//
// Integer vector functions
//

static Vec2i V2iNeg(Vec2i v) { return (Vec2i){-v.x, -v.y}; }

static Vec2i V2iAdd(Vec2i v1, Vec2i v2) { return (Vec2i){v1.x + v2.x, v1.y + v2.y}; }
static Vec2i V2iSub(Vec2i v1, Vec2i v2) { return (Vec2i){v1.x - v2.x, v1.y - v2.y}; }

static Vec2i V2iMulS(Vec2i v, int s)  { return (Vec2i){v.x*s, v.y*s}; }
static Vec2i V2iDivS(Vec2i v, int s)  { return (Vec2i){v.x/s, v.y/s}; }

static Vec2i V2iMul(Vec2i v1, Vec2i v2) { return (Vec2i){v1.x*v2.x, v1.y*v2.y}; }
static Vec2i V2iDiv(Vec2i v1, Vec2i v2) { return (Vec2i){v1.x/v2.x, v1.y/v2.y}; }

static bool V2iEq(Vec2i v1, Vec2i v2) { return ((v1.x == v2.x) && (v1.y == v2.y)); }

static Vec2i V2iClamp(Vec2i v, int min, int max) {
    Vec2i result;
    
    result.x = CLAMP(v.x, min, max);
    result.y = CLAMP(v.y, min, max);
    
    return result;
}


static Vec3i V3iNeg(Vec3i v) { return (Vec3i){-v.x, -v.y, -v.z}; }

static Vec3i V3iAdd(Vec3i v1, Vec3i v2) { return (Vec3i){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
static Vec3i V3iSub(Vec3i v1, Vec3i v2) { return (Vec3i){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }

static Vec3i V3iMulS(Vec3i v, int s) { return (Vec3i){v.x*s, v.y*s, v.z*s}; }
static Vec3i V3iDivS(Vec3i v, int s) { return (Vec3i){v.x/s, v.y/s, v.z/s}; }

static Vec3i V3iMul(Vec3i v1, Vec3i v2) { return (Vec3i){v1.x*v2.x, v1.y*v2.y, v1.z*v2.z}; }
static Vec3i V3iDiv(Vec3i v1, Vec3i v2) { return (Vec3i){v1.x/v2.x, v1.y/v2.y, v1.z/v2.z}; }

static bool V3iEq(Vec3i v1, Vec3i v2) { return ((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z)); }

static Vec3i V3iClamp(Vec3i v, int min, int max) {
    Vec3i result;
    
    result.x = CLAMP(v.x, min, max);
    result.y = CLAMP(v.y, min, max);
    result.z = CLAMP(v.z, min, max);
    
    return result;
}


static Vec4i V4iNeg(Vec4i v) { return (Vec4i){-v.x, -v.y, -v.z, -v.w}; }

static Vec4i V4iAdd(Vec4i v1, Vec4i v2) { return (Vec4i){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w}; }
static Vec4i V4iSub(Vec4i v1, Vec4i v2) { return (Vec4i){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w}; }

static Vec4i V4iMulS(Vec4i v, int s) { return (Vec4i){v.x*s, v.y*s, v.z*s, v.w*s}; }
static Vec4i V4iDivS(Vec4i v, int s) { return (Vec4i){v.x/s, v.y/s, v.z/s, v.w/s}; }

static Vec4i V4iMul(Vec4i v1, Vec4i v2) { return (Vec4i){v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w}; }
static Vec4i V4iDiv(Vec4i v1, Vec4i v2) { return (Vec4i){v1.x/v2.x, v1.y/v2.y, v1.z/v2.z, v1.w/v2.w}; }

static bool V4iEq(Vec4i v1, Vec4i v2) { return ((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z) && (v1.w == v2.w)); }

static Vec4i V4iClamp(Vec4i v, int min, int max) {
    Vec4i result;
    
    result.x = CLAMP(v.x, min, max);
    result.y = CLAMP(v.y, min, max);
    result.z = CLAMP(v.z, min, max);
    result.w = CLAMP(v.w, min, max);
    
    return result;
}

//
// Float vectors
//

typedef union Vec2 {
    struct { float x, y; };
    struct { float u, v; };
    struct { float s, t; };
    
    struct { float left, right; };
    struct { float width, height; };
    
    float values[2];
} Vec2;

typedef union Vec3 {
    struct { float x, y, z; };
    struct { float r, g, b; };
    struct { float u, v, w; };
    
    struct { Vec2 xy; float _ignored0; };
    struct { Vec2 rg; float _ignored1; };
    struct { Vec2 uv; float _ignored2; };
    struct { float _ignored3; Vec2 yz; };
    struct { float _ignored4; Vec2 gb; };
    struct { float _ignored5; Vec2 vw; };
    
    float values[3];
} Vec3;

typedef union Vec4 {
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    
    struct { Vec3 xyz; float _ignored0; };
    struct { Vec3 rgb; float _ignored1; };
    
    struct { Vec2 xy; Vec2 zw; };
    struct { float _ignored2; Vec2 yz; float _ignored3; };
    struct { Vec2 rg; Vec2 ba; };
    struct { float _ignored4; Vec2 gb; float _ignored5; };
    
    float values[4];
} Vec4;

//
// Float vector functions
//

static Vec2 V2Neg(Vec2 v) { return (Vec2){-v.x, -v.y}; }

static Vec2 V2Add(Vec2 v1, Vec2 v2) { return (Vec2){v1.x + v2.x, v1.y + v2.y}; } // TODO(student)
static Vec2 V2Sub(Vec2 v1, Vec2 v2) { return (Vec2){v1.x - v2.x, v1.y - v2.y}; }

static Vec2 V2MulS(Vec2 v, float s) { return (Vec2){v.x*s, v.y*s}; }
static Vec2 V2DivS(Vec2 v, float s) { return (Vec2){v.x/s, v.y/s}; } // TODO(student)

static Vec2 V2Mul(Vec2 v1, Vec2 v2) { return (Vec2){v1.x*v2.x, v1.y*v2.y}; }
static Vec2 V2Div(Vec2 v1, Vec2 v2) { return (Vec2){v1.x/v2.x, v1.y/v2.y}; }

static bool V2Eq(Vec2 v1, Vec2 v2) { return ((v1.x == v2.x) && (v1.y == v2.y)); }

static Vec2 V2Lerp(Vec2 a, float t, Vec2 b) {
    return V2Add(V2MulS(a, (1.f - t)), V2MulS(b, t));
}

static float V2Dot(Vec2 v1, Vec2 v2) {
    float result;
    result = (v1.x * v2.x) + (v1.y * v2.y);// TODO(student)
    return result;
}

static float V2Length(Vec2 v) {
    float result;
    result = sqrt((v.x * v.x) + (v.y * v.y)); // TODO(student)
    return result;
}

static Vec2 V2Normalize(Vec2 v) {
    Vec2 result = {0.f, 0.f};
    // TODO(student)
    return result;
}

static Vec3 V3Neg(Vec3 v) { return (Vec3){-v.x, -v.y, -v.z}; }

static Vec3 V3Add(Vec3 v1, Vec3 v2) { return (Vec3){0.f, 0.f, 0.f}; } // TODO(student)
static Vec3 V3Sub(Vec3 v1, Vec3 v2) { return (Vec3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }

static Vec3 V3MulS(Vec3 v, float s) { return (Vec3){0.f, 0.f, 0.f}; } // TODO(student)
static Vec3 V3DivS(Vec3 v, float s) { return (Vec3){0.f, 0.f, 0.f}; } // TODO(student)

static Vec3 V3Mul(Vec3 v1, Vec3 v2) { return (Vec3){0.f, 0.f}; } // TODO(student)
static Vec3 V3Div(Vec3 v1, Vec3 v2) { return (Vec3){0.f, 0.f}; } // TODO(student)

static bool V3Eq(Vec3 v1, Vec3 v2) { return false; } // TODO(student)

static Vec3 V3Lerp(Vec3 a, float t, Vec3 b) {
    return V3Add(V3MulS(a, (1.f - t)), V3MulS(b, t));
}

static float V3Dot(Vec3 v1, Vec3 v2) {
    float result = 0.f;
    // TODO(student)
    return result;
}

static float V3LengthSquared(Vec3 v) {
    float result = 0.f;
    // TODO(student)
    return result;
}

static float V3Length(Vec3 v) {
    return sqrt(V3LengthSquared(v));
}

static Vec3 V3Normalize(Vec3 v) {
    Vec3 result;
    float length = V3Length(v);
    if(length < EPSILON) return v;
    
    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;
    
    return result;
}

static Vec3 V3Cross(Vec3 v1, Vec3 v2) {
    Vec3 result = {};
    // TODO(student)
    return result;
}


static Vec4 V4Neg(Vec4 v) { return (Vec4){-v.x, -v.y, -v.z, -v.w}; }

static Vec4 V4Add(Vec4 v1, Vec4 v2) { return (Vec4){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w}; }
static Vec4 V4Sub(Vec4 v1, Vec4 v2) { return (Vec4){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w}; }

static Vec4 V4MulS(Vec4 v, float s) { return (Vec4){v.x*s, v.y*s, v.z*s, v.w*s}; }
static Vec4 V4DivS(Vec4 v, float s) { return (Vec4){v.x/s, v.y/s, v.z/s, v.w/s}; }

static Vec4 V4Mul(Vec4 v1, Vec4 v2) { return (Vec4){v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w}; }
static Vec4 V4Div(Vec4 v1, Vec4 v2) { return (Vec4){v1.x/v2.x, v1.y/v2.y, v1.z/v2.z, v1.w/v2.w}; }

static bool V4Eq(Vec4 v1, Vec4 v2) { return ((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z) && (v1.w == v2.w)); }

static Vec4 V4Lerp(Vec4 a, float t, Vec4 b) {
    return V4Add(V4MulS(a, (1.f - t)), V4MulS(b, t));
}

static float V4Dot(Vec4 v1, Vec4 v2) {
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}

static float V4LengthSquared(Vec4 v) {
    return V4Dot(v, v);
}

static float V4Length(Vec4 v) {
    return sqrt(V4LengthSquared(v));
}

static Vec4 V4Normalize(Vec4 v) {
    Vec4 result;
    float length = V4Length(v);
    if(length < EPSILON) return v;
    
    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;
    result.w = v.w / length;
    
    return result;
}

//
// Float Matrices
//

typedef union Mat2 {
    struct {
        float s00, s01;
        float s10, s11;
    };
    
    struct { Vec2 r0, r1; };
    Vec2 rows[2];
    Vec4 value;
} Mat2;

typedef union Mat3 {
    struct {
        float s00, s01, s02;
        float s10, s11, s12;
        float s20, s21, s22;
    };
    
    struct { Vec3 r0, r1, r2; };
    Vec3 rows[3];
} Mat3;

typedef union Mat4 {
    struct {
        float s00, s01, s02, s03;
        float s10, s11, s12, s13;
        float s20, s21, s22, s23;
        float s30, s31, s32, s33;
    };
    
    struct { Vec4 r0, r1, r2, r3; };
    Vec4 rows[4];
} Mat4;

//
// Matrix functions
//

static Mat2 M2Neg(Mat2 m) {
    Mat2 result;
    result.r0 = V2Neg(m.r0);
    result.r1 = V2Neg(m.r1);
    return result;
}

static Mat2 M2Add(Mat2 m1, Mat2 m2) {
    Mat2 result = {};
    // TODO(student)
    return result;
}

static Mat2 M2Sub(Mat2 m1, Mat2 m2) {
    Mat2 result;
    result.r0 = V2Sub(m1.r0, m2.r0);
    result.r1 = V2Sub(m1.r1, m2.r1);
    return result;
}

static Mat2 M2MulS(Mat2 m, float s) {
    Mat2 result;
    result.r0 = V2MulS(m.r0, s);
    result.r1 = V2MulS(m.r1, s);
    return result;
}

static Mat2 M2DivS(Mat2 m, float s) {
    Mat2 result;
    result.r0 = V2DivS(m.r0, s);
    result.r1 = V2DivS(m.r1, s);
    return result;
}

static Vec2 M2MulV2(Mat2 m, Vec2 v) {
    Vec2 result = {};
    // TODO(student)
    return result;
}

static Mat2 M2MulM2(Mat2 m1, Mat2 m2) {
    Mat2 result = {};
    // TODO(student)
    return result;
}

static bool M2Eq(Mat2 m1, Mat2 m2) {
    return V2Eq(m1.r0, m2.r0) && V2Eq(m1.r1, m2.r1);
}

static Mat2 M2Transpose(Mat2 m) {
    Mat2 result = {};
    // TODO(student)
    return result;
}

static Mat2 M2Diag(float s00, float s11)  {
    Mat2 result;
    result.s00 = s00;
    result.s01 = 0.0f;
    result.s10 = 0.0f;
    result.s11 = s11;
    return result;
}

static float M2Det(Mat2 m) {
    float result = {};
    // TODO(student)
    return result;
}

static Mat2 M2Rotate(float theta) {
    Mat2 result = {};
    // TODO(student)
    return result;
}

static Mat2 M2Inverse(Mat2 m) {
    Mat2 result = {};
    // TODO(student)
    return result;
}

static Mat3 M3Neg(Mat3 m) {
    Mat3 result;
    result.r0 = V3Neg(m.r0);
    result.r1 = V3Neg(m.r1);
    result.r2 = V3Neg(m.r2);
    return result;
}

static Mat3 M3Add(Mat3 m1, Mat3 m2) {
    Mat3 result = {};
    // TODO(student)
    return result;
}

static Mat3 M3Sub(Mat3 m1, Mat3 m2) {
    Mat3 result = {};
    // TODO(student)
    return result;
}

static Mat3 M3MulS(Mat3 m, float s) {
    Mat3 result;
    result.r0 = V3MulS(m.r0, s);
    result.r1 = V3MulS(m.r1, s);
    result.r2 = V3MulS(m.r2, s);
    return result;
}

static Mat3 M3DivS(Mat3 m, float s) {
    Mat3 result;
    result.r0 = V3DivS(m.r0, s);
    result.r1 = V3DivS(m.r1, s);
    result.r2 = V3DivS(m.r2, s);
    return result;
}

static Vec3 M3MulV3(Mat3 m, Vec3 v) {
    Vec3 result = {};
    // TODO(student)
    return result;
}

static Mat3 M3MulM3(Mat3 m1, Mat3 m2) {
    Mat3 result = {};
    // TODO(student)
    return result;
}

static bool M3Eq(Mat3 m1, Mat3 m2) {
    return V3Eq(m1.r0, m2.r0) && V3Eq(m1.r1, m2.r1) && V3Eq(m1.r2, m2.r2);
}

static Mat3 M3Transpose(Mat3 m) {
    Mat3 result = {};
    // TODO(student)
    return result;
}

static Mat3 M3Diag(float s00, float s11, float s22)  {
    Mat3 result;
    
    result.s00 = s00;
    result.s01 = 0.f;
    result.s02 = 0.f;
    
    result.s10 = 0.f;
    result.s11 = s11;
    result.s12 = 0.f;
    
    result.s20 = 0.f;
    result.s21 = 0.f;
    result.s22 = s22;
    
    return result;
}

static Mat3 M3DiagS(float s) {
    Mat3 result = M3Diag(s, s, s);
    return result;
}

static float M3Det(Mat3 m) {
    float result = 0.f;
    // TODO(student)
    return result;
}

// Rotation of angle theta around a 3D axis
static Mat3 M3Rotate(float theta, Vec3 axis) {
    Mat3 result = {};
    // TODO(student)
    return result;
}

// Rotates from 'source' into 'dest'
static Mat3 M3RotateV(Vec3 source, Vec3 dest) {
    Mat3 result = {};
    // TODO(student)
    return result;
}

static Mat3 M3Inverse(Mat3 m) {
    Mat3 result = {};
    // TODO(student)
    
    return result;
}

static Mat4 M4Neg(Mat4 m) {
    Mat4 result;
    result.r0 = V4Neg(m.r0);
    result.r1 = V4Neg(m.r1);
    result.r2 = V4Neg(m.r2);
    result.r3 = V4Neg(m.r3);
    return result;
}

static Mat4 M4Add(Mat4 m1, Mat4 m2) {
    Mat4 result;
    result.r0 = V4Add(m1.r0, m2.r0);
    result.r1 = V4Add(m1.r1, m2.r1);
    result.r2 = V4Add(m1.r2, m2.r2);
    result.r3 = V4Add(m1.r3, m2.r3);
    return result;
}

static Mat4 M4Sub(Mat4 m1, Mat4 m2) {
    Mat4 result;
    result.r0 = V4Sub(m1.r0, m2.r0);
    result.r1 = V4Sub(m1.r1, m2.r1);
    result.r2 = V4Sub(m1.r2, m2.r2);
    result.r3 = V4Sub(m1.r3, m2.r3);
    return result;
}

static Mat4 M4MulS(Mat4 m, float s) {
    Mat4 result;
    result.r0 = V4MulS(m.r0, s);
    result.r1 = V4MulS(m.r1, s);
    result.r2 = V4MulS(m.r2, s);
    result.r3 = V4MulS(m.r3, s);
    return result;
}

static Mat4 M4DivS(Mat4 m, float s) {
    Mat4 result;
    result.r0 = V4DivS(m.r0, s);
    result.r1 = V4DivS(m.r1, s);
    result.r2 = V4DivS(m.r2, s);
    result.r3 = V4DivS(m.r3, s);
    return result;
}

static Vec4 M4MulV4(Mat4 m, Vec4 v) {
    Vec4 result = {};
    // TODO(student)
    return result;
}

static Vec3 M4MulV3(Mat4 m, Vec3 v) {
    Vec4 u = M4MulV4(m, (Vec4){v.x, v.y, v.z, 1.f});
    if(ABS(u.w) < EPSILON) return v;
    
    return (Vec3) {u.x/u.w, u.y/u.w, u.z/u.w};
}

static Mat4 M4MulM4(Mat4 m1, Mat4 m2) {
    Mat4 result = {};
    // TODO(student)
    return result;
}

static bool M4Eq(Mat4 m1, Mat4 m2) {
    return V4Eq(m1.r0, m2.r0) && V4Eq(m1.r1, m2.r1) && V4Eq(m1.r2, m2.r2) && V4Eq(m1.r3, m2.r3);
}

static Mat4 M4Transpose(Mat4 m) {
    Mat4 result = {};
    // TODO(student)
    return result;
}

static Mat4 M4Diag(float s00, float s11, float s22, float s33)  {
    Mat4 result;
    
    result.s00 = s00;
    result.s01 = 0.f;
    result.s02 = 0.f;
    result.s03 = 0.f;
    
    result.s10 = 0.f;
    result.s11 = s11;
    result.s12 = 0.f;
    result.s13 = 0.f;
    
    result.s20 = 0.f;
    result.s21 = 0.f;
    result.s22 = s22;
    result.s23 = 0.f;
    
    result.s30 = 0.f;
    result.s31 = 0.f;
    result.s32 = 0.f;
    result.s33 = s33;
    
    return result;
}

static Mat4 M4DiagS(float s) {
    return M4Diag(s, s, s, s);
}

static float M4Det(Mat4 m) {
    float result = 0.f;
    // TODO(student)
    return result;
}

static Mat4 M4Rotate(float theta, Vec3 axis) {
    Mat4 result = {};
    // TODO(student)
    return result;
}

static Mat4 M4RotateV(Vec3 source, Vec3 dest) {
    Mat4 result = {};
    // TODO(student)
    return result;
}

static Mat4 M4Scale(float x, float y, float z) {
    return M4Diag(x, y, z, 1.f);
}

static Mat4 M4ScaleV3(Vec3 v) {
    return M4Scale(v.x, v.y, v.z);
}

static Mat4 M4ScaleS(float s) {
    return M4Scale(s, s, s);
}

static Mat4 M4Translate(Vec3 v) {
    Mat4 result = M4DiagS(1.f);
    // TODO(student)
    return result;
}

static Mat4 M4Identity() {
    return M4ScaleS(1.f);
}

static Mat4 M4Orthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
    Mat4 result = {};
    // TODO(student)
    return result;
}

static Mat4 M4Perspective(float fov, float aspect, float zNear, float zFar) {
    Mat4 result = {};
    // TODO(student)
    return result;
}

static Mat4 M4LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Mat4 result = {};
    // TODO(student)
    return result;
}

#endif // MATH_HEADER
