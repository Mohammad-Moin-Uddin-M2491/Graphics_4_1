#pragma once
#include <glad/gl.h>
#include "math3d.h"
#include "lighting.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cstdint>

namespace cg {
inline GLuint makeProgram(const char* vs,const char* fs) {
    auto compile=[](GLenum type,const char* source) {
        GLuint s=glCreateShader(type);glShaderSource(s,1,&source,nullptr);glCompileShader(s);
        GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
        if(!ok) {char log[4096];glGetShaderInfoLog(s,4096,nullptr,log);glDeleteShader(s);throw std::runtime_error(log);}return s;
    };
    GLuint v=compile(GL_VERTEX_SHADER,vs),f=compile(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();
    glAttachShader(p,v);glAttachShader(p,f);glLinkProgram(p);glDeleteShader(v);glDeleteShader(f);
    GLint ok;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){char log[4096];glGetProgramInfoLog(p,4096,nullptr,log);glDeleteProgram(p);throw std::runtime_error(log);}return p;
}
inline void matrix(GLuint p,const char* name,const Mat4& m) {glUniformMatrix4fv(glGetUniformLocation(p,name),1,GL_FALSE,m.m);}
inline void vector(GLuint p,const char* name,Vec3 v) {glUniform3f(glGetUniformLocation(p,name),v.x,v.y,v.z);}
struct Vertex {Vec3 position,normal;};
struct Mesh {GLuint vao=0,vbo=0;GLsizei count=0;std::vector<Vertex> vertices;};
inline Mesh upload(const std::vector<Vertex>& v) {
    Mesh m;m.vertices=v;m.count=static_cast<GLsizei>(v.size());glGenVertexArrays(1,&m.vao);glGenBuffers(1,&m.vbo);
    glBindVertexArray(m.vao);glBindBuffer(GL_ARRAY_BUFFER,m.vbo);glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(Vertex),v.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),nullptr);
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(sizeof(Vec3)));return m;
}
inline void triangle(std::vector<Vertex>& v,Vec3 a,Vec3 b,Vec3 c) {Vec3 n=normalize(cross(b-a,c-a));v.push_back({a,n});v.push_back({b,n});v.push_back({c,n});}
inline Mesh cubeMesh() {
    std::vector<Vertex> v;
    const Vec3 p[]={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},
                    {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    const int f[][4]={{4,5,6,7},{1,0,3,2},{0,4,7,3},{5,1,2,6},{3,7,6,2},{0,1,5,4}};
    for(auto& q:f){triangle(v,p[q[0]],p[q[1]],p[q[2]]);triangle(v,p[q[0]],p[q[2]],p[q[3]]);}return upload(v);
}
// A circular solid along local Z, made only from triangles: two cap fans
// plus two triangles per side segment. Side normals interpolate smoothly.
inline Mesh cylinderMesh() {
    std::vector<Vertex> v;constexpr int N=48;
    for(int i=0;i<N;++i){float a=2*PI*i/N,b=2*PI*(i+1)/N;
        Vec3 n{std::cos(a),std::sin(a),0},m{std::cos(b),std::sin(b),0};
        Vec3 p{n.x,n.y,-.5f},q{m.x,m.y,-.5f},r{m.x,m.y,.5f},s{n.x,n.y,.5f};
        v.insert(v.end(),{{p,n},{q,m},{r,m},{p,n},{r,m},{s,n}});
        triangle(v,{0,0,.5f},s,r);triangle(v,{0,0,-.5f},q,p);
    }return upload(v);
}
inline Mesh sphereMesh() {
    std::vector<Vertex> v;
    auto p=[](int y,int x){float a=PI*y/16,b=2*PI*x/24;return Vec3{std::sin(a)*std::cos(b),std::cos(a),std::sin(a)*std::sin(b)};};
    for(int y=0;y<16;++y)for(int x=0;x<24;++x){Vec3 a=p(y,x),b=p(y+1,x),c=p(y+1,x+1),d=p(y,x+1);
        v.insert(v.end(),{{a,a},{b,b},{c,c},{a,a},{c,c},{d,d}});}return upload(v);
}
inline Mesh cabinMesh() {
    std::vector<Vertex> v;
    Vec3 p[]={{-1,-.5f,-.5f},{1,-.5f,-.5f},{.55f,.5f,-.5f},{-.65f,.5f,-.5f},
              {-1,-.5f,.5f},{1,-.5f,.5f},{.55f,.5f,.5f},{-.65f,.5f,.5f}};
    const int f[][4]={{4,5,6,7},{1,0,3,2},{0,4,7,3},{5,1,2,6},{3,7,6,2},{0,1,5,4}};
    for(auto& q:f){triangle(v,p[q[0]],p[q[1]],p[q[2]]);triangle(v,p[q[0]],p[q[2]],p[q[3]]);}return upload(v);
}
struct Material {Vec3 color;float shine=32,specular=.3f,metallic=0;};
// Hollow circular geometry from triangles, also used as half-circle fenders.
inline Mesh ringMesh(float arc=2*PI) {
    std::vector<Vertex> v;constexpr int N=64;
    for(int i=0;i<N;++i){float a=arc*i/N,b=arc*(i+1)/N;
        Vec3 na{std::cos(a),std::sin(a),0},nb{std::cos(b),std::sin(b),0};
        for(float z:{-.5f,.5f}){
            Vec3 oa{na.x,na.y,z},ob{nb.x,nb.y,z},ia{na.x*.65f,na.y*.65f,z},ib{nb.x*.65f,nb.y*.65f,z};
            if(z>0){triangle(v,oa,ob,ib);triangle(v,oa,ib,ia);}else{triangle(v,oa,ib,ob);triangle(v,oa,ia,ib);}
        }
        for(float r:{.65f,1.0f}){Vec3 n=na*(r<1?-1.f:1.f),m=nb*(r<1?-1.f:1.f);
            Vec3 p{na.x*r,na.y*r,-.5f},q{nb.x*r,nb.y*r,-.5f},s{na.x*r,na.y*r,.5f},t{nb.x*r,nb.y*r,.5f};
            v.insert(v.end(),{{p,n},{q,m},{t,m},{p,n},{t,m},{s,n}});}
    }
    if(arc<2*PI){for(float a:{0.f,arc}){Vec3 n{std::cos(a),std::sin(a),0};
        triangle(v,{n.x,n.y,-.5f},{n.x,n.y,.5f},{n.x*.65f,n.y*.65f,.5f});
        triangle(v,{n.x,n.y,-.5f},{n.x*.65f,n.y*.65f,.5f},{n.x*.65f,n.y*.65f,-.5f});}}
    return upload(v);
}
struct Draw {int mesh;Mat4 model;Material material;};
class Renderer {
public:
    GLuint program=0,depthProgram=0,shadowFbo=0,shadowTexture=0;
    Mesh meshes[6];
    Mat4 lightVP;
    Renderer() {
        // Share the lighting formula between vertex (Gouraud) and fragment
        // evaluation, so changing light presets works in every shading mode.
        const std::string lightCode=R"GLSL(
uniform vec3 baseColor,eye,keyDirection,keyColor,fillPosition,fillColor,rimColor,skyColor,groundColor;
uniform float shine,specular,metallic;
vec3 ambient(vec3 n) {
    return baseColor*mix(groundColor,skyColor,n.y*.5+.5)*(1.0-.55*metallic);
}
vec3 lightTerm(vec3 n,vec3 v,vec3 l,vec3 energy) {
    float ndl=max(dot(n,l),0.0);vec3 reflected=reflect(-l,n);
    vec3 tint=mix(vec3(1),baseColor,metallic);
    vec3 diffuse=baseColor*(1.0-.65*metallic)*ndl;
    vec3 highlight=tint*pow(max(dot(v,reflected),0.0),shine)*specular*(1.0+shine*.045);
    return (diffuse+highlight*ndl)*energy;
}
void evaluateLights(vec3 n,vec3 p,out vec3 key,out vec3 other) {
    vec3 v=normalize(eye-p),delta=fillPosition-p;float distance=length(delta);
    key=lightTerm(n,v,normalize(keyDirection),keyColor);
    other=ambient(n)+lightTerm(n,v,normalize(delta),fillColor/(1.0+.06*distance+.025*distance*distance));
    other+=lightTerm(n,v,normalize(vec3(.25,.5,-1)),rimColor);
}
)GLSL";
        const std::string vs=std::string(R"GLSL(#version 330 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
uniform mat4 model,view,projection,lightVP;
out vec3 worldPos,N,vertexKey,vertexOther;
out vec4 shadowPos;
)GLSL")+lightCode+R"GLSL(
void main(){vec4 p=model*vec4(position,1);worldPos=p.xyz;
    N=normalize(transpose(inverse(mat3(model)))*normal);
    evaluateLights(N,worldPos,vertexKey,vertexOther);shadowPos=lightVP*p;gl_Position=projection*view*p;}
)GLSL";
        const std::string fs=std::string(R"GLSL(#version 330 core
in vec3 worldPos,N,vertexKey,vertexOther;
in vec4 shadowPos;
uniform int shading,shadows;
uniform float exposure,softness;
uniform vec3 background;
uniform sampler2D shadowMap;
out vec4 frag;
)GLSL")+lightCode+R"GLSL(
float shadow(vec3 n,vec3 l){vec3 p=shadowPos.xyz/shadowPos.w*.5+.5;
    // Account for the receiver's slope across each filter tap. Without this,
    // a wide shadow filter makes a flat floor incorrectly shadow itself.
    vec3 dx=dFdx(p),dy=dFdy(p);float det=dx.x*dy.y-dx.y*dy.x;
    vec2 gradient=abs(det)>.00000001?vec2(dy.y*dx.z-dx.y*dy.z,dx.x*dy.z-dy.x*dx.z)/det:vec2(0);
    if(shadows==0)return 0.0;
    if(p.z>1.0||p.z<0.0||p.x<0.0||p.x>1.0||p.y<0.0||p.y>1.0)return 0.0;
    float bias=max(.0012*(1.0-dot(n,l)),.0004),s=0.0;vec2 texel=softness/vec2(textureSize(shadowMap,0));
    for(int x=-2;x<=2;++x)for(int y=-2;y<=2;++y){vec2 offset=vec2(x,y)*texel;
        s+=p.z+dot(gradient,offset)-bias>texture(shadowMap,p.xy+offset).r?1.0:0.0;}
    return s/25.0;}
void main(){vec3 n=normalize(N);
    if(shading==1){n=normalize(cross(dFdx(worldPos),dFdy(worldPos)));if(dot(n,N)<0.0)n=-n;}
    vec3 key,other;
    if(shading==2){key=vertexKey;other=vertexOther;}
    else evaluateLights(n,worldPos,key,other);
    vec3 c=other+key*(1.0-shadow(n,normalize(keyDirection)));
    float haze=1.0-exp(-.025*max(length(eye-worldPos)-24.0,0.0));
    c=mix(c,background,haze);c=max(c*exposure,vec3(0));
    // Reinhard tone mapping preserves highlights before display gamma.
    c=c/(vec3(1)+c);frag=vec4(pow(c,vec3(1.0/2.2)),1);}
)GLSL";
        program=makeProgram(vs.c_str(),fs.c_str());
        depthProgram=makeProgram(R"GLSL(#version 330 core
layout(location=0)in vec3 position;uniform mat4 model,lightVP;
void main(){gl_Position=lightVP*model*vec4(position,1);})GLSL",R"GLSL(#version 330 core
void main(){})GLSL");
        meshes[0]=cubeMesh();meshes[1]=cylinderMesh();meshes[2]=sphereMesh();meshes[3]=cabinMesh();
        meshes[4]=ringMesh();meshes[5]=ringMesh(PI);
        lightVP=ortho(-19,19,-13,13,1,65)*lookAt({-12,27,18},{0,0,0},{0,1,0});
        glGenFramebuffers(1,&shadowFbo);glGenTextures(1,&shadowTexture);glBindTexture(GL_TEXTURE_2D,shadowTexture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,2048,2048,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
        float border[]={1,1,1,1};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
        glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,shadowTexture,0);
        glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Shadow framebuffer incomplete");
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
    void draw(const std::vector<Draw>& items,const Mat4& view,const Mat4& projection,Vec3 eye,int w,int h,int shading,int lighting,float brightness,bool shadows) {
        const Lighting light=lightingPreset(lighting);
        lightVP=ortho(-19,19,-15,15,1,75)*lookAt(normalize(light.direction)*35,{0,0,0},{0,1,0});
        glEnable(GL_DEPTH_TEST);glDisable(GL_BLEND);
        if(shadows){
        glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo);glViewport(0,0,2048,2048);glClear(GL_DEPTH_BUFFER_BIT);glUseProgram(depthProgram);matrix(depthProgram,"lightVP",lightVP);
        glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(2,4);
        for(auto& d:items){matrix(depthProgram,"model",d.model);glBindVertexArray(meshes[d.mesh].vao);glDrawArrays(GL_TRIANGLES,0,meshes[d.mesh].count);}
        glDisable(GL_POLYGON_OFFSET_FILL);}
        glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,w,h);
        auto display=[&](float c){c*=light.exposure*brightness;return std::pow(c/(1+c),1/2.2f);};
        glClearColor(display(light.background.x),display(light.background.y),display(light.background.z),1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);matrix(program,"view",view);matrix(program,"projection",projection);matrix(program,"lightVP",lightVP);vector(program,"eye",eye);
        glUniform1i(glGetUniformLocation(program,"shading"),shading);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,shadowTexture);glUniform1i(glGetUniformLocation(program,"shadowMap"),0);
        vector(program,"keyDirection",light.direction);vector(program,"keyColor",light.keyColor);
        vector(program,"fillPosition",light.fillPosition);vector(program,"fillColor",light.fillColor);vector(program,"rimColor",light.rimColor);
        vector(program,"skyColor",light.sky);vector(program,"groundColor",light.ground);vector(program,"background",light.background);
        glUniform1f(glGetUniformLocation(program,"exposure"),light.exposure*brightness);
        glUniform1f(glGetUniformLocation(program,"softness"),light.softness);glUniform1i(glGetUniformLocation(program,"shadows"),shadows?1:0);
        for(auto& d:items){matrix(program,"model",d.model);vector(program,"baseColor",d.material.color);
            glUniform1f(glGetUniformLocation(program,"shine"),d.material.shine);glUniform1f(glGetUniformLocation(program,"specular"),d.material.specular);
            glUniform1f(glGetUniformLocation(program,"metallic"),d.material.metallic);
            glBindVertexArray(meshes[d.mesh].vao);glDrawArrays(GL_TRIANGLES,0,meshes[d.mesh].count);}
    }
    ~Renderer(){for(auto& m:meshes){glDeleteBuffers(1,&m.vbo);glDeleteVertexArrays(1,&m.vao);}glDeleteProgram(program);glDeleteProgram(depthProgram);glDeleteTextures(1,&shadowTexture);glDeleteFramebuffers(1,&shadowFbo);}
};
// Save the actual rendered framebuffer, including the HUD, for visual QA.
inline void captureBMP(const std::string& path,int w,int h) {
    int stride=(w*3+3)&~3;std::vector<unsigned char> data(stride*h);
    glPixelStorei(GL_PACK_ALIGNMENT,4);glReadPixels(0,0,w,h,GL_BGR,GL_UNSIGNED_BYTE,data.data());
    std::ofstream out(path,std::ios::binary);if(!out)throw std::runtime_error("Cannot write screenshot: "+path);
    auto u16=[&](uint16_t x){out.put(char(x));out.put(char(x>>8));};
    auto u32=[&](uint32_t x){for(int i=0;i<4;++i)out.put(char(x>>(8*i)));};
    out.put('B');out.put('M');u32(54+stride*h);u32(0);u32(54);u32(40);u32(w);u32(h);u16(1);u16(24);u32(0);u32(stride*h);u32(2835);u32(2835);u32(0);u32(0);
    out.write(reinterpret_cast<const char*>(data.data()),data.size());
}
}
