#pragma once
#include <cmath>
#include <algorithm>

// Column-major matrices; column vectors: world = T * R * S * local.
namespace cg {
constexpr float PI = 3.14159265358979323846f;
inline float radians(float degrees) { return degrees * PI / 180.0f; }
struct Vec3 {
    float x=0, y=0, z=0;
    Vec3 operator+(Vec3 b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec3 operator-(Vec3 b) const { return {x-b.x,y-b.y,z-b.z}; }
    Vec3 operator*(float s) const { return {x*s,y*s,z*s}; }
};
inline float dot(Vec3 a,Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
inline Vec3 cross(Vec3 a,Vec3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
inline float length(Vec3 a) { return std::sqrt(dot(a,a)); }
inline Vec3 normalize(Vec3 a) { float n=length(a); return n>0 ? a*(1/n) : Vec3{}; }
struct Mat4 {
    float m[16]{};
    static Mat4 identity() { Mat4 a; a.m[0]=a.m[5]=a.m[10]=a.m[15]=1; return a; }
};
inline Mat4 operator*(const Mat4& a,const Mat4& b) {
    Mat4 c;
    for(int col=0;col<4;++col) for(int row=0;row<4;++row)
        for(int k=0;k<4;++k) c.m[col*4+row]+=a.m[k*4+row]*b.m[col*4+k];
    return c;
}
inline Vec3 transform(const Mat4& a,Vec3 p) {
    return {a.m[0]*p.x+a.m[4]*p.y+a.m[8]*p.z+a.m[12],
            a.m[1]*p.x+a.m[5]*p.y+a.m[9]*p.z+a.m[13],
            a.m[2]*p.x+a.m[6]*p.y+a.m[10]*p.z+a.m[14]};
}
inline Mat4 translate(Vec3 p) { auto a=Mat4::identity();a.m[12]=p.x;a.m[13]=p.y;a.m[14]=p.z;return a; }
inline Mat4 scale(Vec3 s) { auto a=Mat4::identity();a.m[0]=s.x;a.m[5]=s.y;a.m[10]=s.z;return a; }
inline Mat4 rotateZ(float t) { auto a=Mat4::identity();float c=std::cos(t),s=std::sin(t);a.m[0]=c;a.m[1]=s;a.m[4]=-s;a.m[5]=c;return a; }
inline Mat4 rotateY(float t) { auto a=Mat4::identity();float c=std::cos(t),s=std::sin(t);a.m[0]=c;a.m[2]=-s;a.m[8]=s;a.m[10]=c;return a; }
inline Mat4 rotateX(float t) { auto a=Mat4::identity();float c=std::cos(t),s=std::sin(t);a.m[5]=c;a.m[6]=s;a.m[9]=-s;a.m[10]=c;return a; }
inline Mat4 perspective(float fov,float aspect,float nearZ,float farZ) {
    Mat4 a;float f=1/std::tan(fov/2);a.m[0]=f/aspect;a.m[5]=f;
    a.m[10]=(farZ+nearZ)/(nearZ-farZ);a.m[11]=-1;a.m[14]=2*farZ*nearZ/(nearZ-farZ);return a;
}
inline Mat4 ortho(float l,float r,float b,float t,float n,float f) {
    auto a=Mat4::identity();a.m[0]=2/(r-l);a.m[5]=2/(t-b);a.m[10]=-2/(f-n);
    a.m[12]=-(r+l)/(r-l);a.m[13]=-(t+b)/(t-b);a.m[14]=-(f+n)/(f-n);return a;
}
inline constexpr float CAMERA_FOV=43, CAMERA_NEAR=.05f, CAMERA_FAR=220;
inline Mat4 cameraProjection(float aspect){return perspective(radians(CAMERA_FOV),aspect,CAMERA_NEAR,CAMERA_FAR);}
// Normalized screen coordinates: (-1,-1) bottom left, (+1,+1) top right.
inline Vec3 cameraRay(Vec3 forward,float x,float y,float aspect){
    Vec3 right=normalize(cross(forward,{0,1,0})),up=cross(right,forward);
    float spread=std::tan(radians(CAMERA_FOV)*.5f);
    return normalize(forward+right*(x*aspect*spread)+up*(y*spread));
}
inline Mat4 lookAt(Vec3 eye,Vec3 target,Vec3 up) {
    Vec3 f=normalize(target-eye),s=normalize(cross(f,up)),u=cross(s,f);
    auto a=Mat4::identity();
    a.m[0]=s.x;a.m[4]=s.y;a.m[8]=s.z;
    a.m[1]=u.x;a.m[5]=u.y;a.m[9]=u.z;
    a.m[2]=-f.x;a.m[6]=-f.y;a.m[10]=-f.z;
    a.m[12]=-dot(s,eye);a.m[13]=-dot(u,eye);a.m[14]=dot(f,eye);return a;
}
inline float clamp01(float x) { return std::clamp(x,0.0f,1.0f); }
inline float smooth(float x) { x=clamp01(x);return x*x*(3-2*x); }
inline float lerp(float a,float b,float p) { return a+(b-a)*p; }
}
