#pragma once
#include "renderer.h"
#include <limits>
#include <numeric>
#include <chrono>

namespace cg {
// Optional CPU snapshot renderer. It intersects the SAME world-space triangles
// as OpenGL. A binary bounding-volume tree only accelerates triangle testing.
// One primary ray/pixel + light visibility rays + at most one reflection bounce.
class RaySnapshot {
    struct Ray {Vec3 origin,direction;};
    struct Triangle {Vec3 a,b,c,na,nb,nc;int material;};
    struct Bounds {
        Vec3 lo{1e30f,1e30f,1e30f},hi{-1e30f,-1e30f,-1e30f};
        void include(Vec3 p){lo={std::min(lo.x,p.x),std::min(lo.y,p.y),std::min(lo.z,p.z)};
            hi={std::max(hi.x,p.x),std::max(hi.y,p.y),std::max(hi.z,p.z)};}
    };
    struct Node {Bounds bounds;int left=-1,right=-1,begin=0,end=0;};
    struct Hit {float distance=1e30f,u=0,v=0;int triangle=-1;};
    std::vector<Triangle> triangles;std::vector<int> order;std::vector<Node> nodes;std::vector<Material> materials;
    Lighting lighting=lightingPreset(0);float exposure=1;bool castShadows=true;
    GLuint texture=0,program=0,vao=0;
    static float component(Vec3 v,int axis){return axis==0?v.x:axis==1?v.y:v.z;}
    static Vec3 product(Vec3 a,Vec3 b){return {a.x*b.x,a.y*b.y,a.z*b.z};}
    static Vec3 normalTransform(const Mat4& m,Vec3 n){
        Vec3 a{m.m[0],m.m[1],m.m[2]},b{m.m[4],m.m[5],m.m[6]},c{m.m[8],m.m[9],m.m[10]};
        return normalize((cross(b,c)*n.x+cross(c,a)*n.y+cross(a,b)*n.z)*(1/dot(a,cross(b,c))));
    }
    int buildNode(int begin,int end){
        int index=int(nodes.size());nodes.push_back({});Bounds bounds,centers;
        for(int i=begin;i<end;++i){auto& t=triangles[order[i]];bounds.include(t.a);bounds.include(t.b);bounds.include(t.c);centers.include((t.a+t.b+t.c)*(1.f/3));}
        nodes[index].bounds=bounds;
        if(end-begin<=6){nodes[index].begin=begin;nodes[index].end=end;return index;}
        Vec3 span=centers.hi-centers.lo;int axis=span.x>span.y?0:1;if(span.z>component(span,axis))axis=2;
        int mid=(begin+end)/2;
        std::nth_element(order.begin()+begin,order.begin()+mid,order.begin()+end,[&](int a,int b){
            auto& x=triangles[a];auto& y=triangles[b];return component(x.a+x.b+x.c,axis)<component(y.a+y.b+y.c,axis);});
        int left=buildNode(begin,mid),right=buildNode(mid,end);
        nodes[index].left=left;nodes[index].right=right;return index;
    }
    static bool boxHit(const Bounds& b,const Ray& ray,float limit){
        float near=0,far=limit;
        for(int axis=0;axis<3;++axis){float d=component(ray.direction,axis),o=component(ray.origin,axis);
            if(std::abs(d)<1e-8f){if(o<component(b.lo,axis)||o>component(b.hi,axis))return false;continue;}
            float a=(component(b.lo,axis)-o)/d,c=(component(b.hi,axis)-o)/d;if(a>c)std::swap(a,c);
            near=std::max(near,a);far=std::min(far,c);if(far<near)return false;
        }return true;
    }
    bool intersect(const Ray& ray,Hit& hit,bool any=false)const{
        if(nodes.empty())return false;
        int stack[128],size=1;stack[0]=0;bool found=false;
        while(size){const Node& node=nodes[stack[--size]];if(!boxHit(node.bounds,ray,hit.distance))continue;
            if(node.left>=0){stack[size++]=node.left;stack[size++]=node.right;continue;}
            for(int i=node.begin;i<node.end;++i){int index=order[i];const auto& t=triangles[index];
                // Moller-Trumbore barycentric ray/triangle intersection.
                Vec3 e1=t.b-t.a,e2=t.c-t.a,p=cross(ray.direction,e2);float determinant=dot(e1,p);
                if(std::abs(determinant)<1e-8f)continue;
                float inverse=1/determinant;Vec3 s=ray.origin-t.a;float u=dot(s,p)*inverse;
                if(u<0||u>1)continue;
                Vec3 q=cross(s,e1);float v=dot(ray.direction,q)*inverse;if(v<0||u+v>1)continue;
                float distance=dot(e2,q)*inverse;if(distance<=.0003f||distance>=hit.distance)continue;
                hit={distance,u,v,index};found=true;if(any)return true;
            }
        }return found;
    }
    Vec3 shade(const Ray& ray,int depth)const{
        Hit hit;if(!intersect(ray,hit))return lighting.background;
        const auto& t=triangles[hit.triangle];const auto& m=materials[t.material];
        Vec3 p=ray.origin+ray.direction*hit.distance;
        Vec3 n=normalize(t.na*(1-hit.u-hit.v)+t.nb*hit.u+t.nc*hit.v);
        if(dot(n,ray.direction)>0)n=n*-1;
        Vec3 view=ray.direction*-1;
        Vec3 ambient=lighting.ground+(lighting.sky-lighting.ground)*(n.y*.5f+.5f);
        Vec3 color=product(m.color,ambient)*(1-.55f*m.metallic);
        auto light=[&](Vec3 l,Vec3 energy,float maxDistance){
            float ndl=std::max(0.f,dot(n,l));if(ndl==0)return Vec3{};
            Hit shadow;shadow.distance=maxDistance;
            if(castShadows&&intersect({p+n*.002f,l},shadow,true))return Vec3{};
            Vec3 reflection=n*(2*dot(n,l))-l,tint=Vec3{1,1,1}*(1-m.metallic)+m.color*m.metallic;
            float highlight=std::pow(std::max(0.f,dot(view,reflection)),m.shine)*m.specular*(1+m.shine*.045f);
            return product(m.color*((1-.65f*m.metallic)*ndl)+tint*(highlight*ndl),energy);
        };
        color=color+light(normalize(lighting.direction),lighting.keyColor,1e25f);
        Vec3 fill=lighting.fillPosition-p;float distance=length(fill);
        color=color+light(normalize(fill),lighting.fillColor*(1/(1+.06f*distance+.025f*distance*distance)),distance-.003f);
        color=color+light(normalize({.25f,.5f,-1}),lighting.rimColor,1e25f);
        float reflectivity=.08f*m.specular+.32f*m.metallic;
        if(depth==0&&reflectivity>.015f){Vec3 reflected=ray.direction-n*(2*dot(ray.direction,n));
            color=color*(1-reflectivity)+shade({p+n*.002f,normalize(reflected)},1)*reflectivity;}
        return color;
    }
public:
    int width=800,height=500;double milliseconds=0;size_t triangleCount=0;
    // A tiny isolated test also runs in --self-test without a graphics context.
    static bool selfTest(){RaySnapshot r;
        r.triangles.push_back({{-1,-1,0},{1,-1,0},{0,1,0},{0,0,1},{0,0,1},{0,0,1},0});
        r.order={0};r.buildNode(0,1);Hit h;
        if(!r.intersect({{0,0,2},{0,0,-1}},h)||std::abs(h.distance-2)>.0001f)return false;
        Hit miss;if(r.intersect({{2,2,2},{0,0,-1}},miss))return false;
        Hit shortRay;shortRay.distance=1;if(r.intersect({{0,0,2},{0,0,-1}},shortRay,true))return false;
        Vec3 n=normalTransform(rotateY(.4f)*scale({2,3,.5f}),{0,0,1});
        return std::abs(length(n)-1)<.0001f&&std::abs(n.x-std::sin(.4f))<.0001f;
    }
    void render(const Renderer& renderer,const std::vector<Draw>& items,Vec3 eye,Vec3 forward,float aspect,int preset,float brightness,bool shadows){
        auto start=std::chrono::steady_clock::now();triangles.clear();nodes.clear();materials.clear();
        lighting=lightingPreset(preset);exposure=lighting.exposure*brightness;castShadows=shadows;
        for(const auto& d:items){int material=int(materials.size());materials.push_back(d.material);auto& v=renderer.meshes[d.mesh].vertices;
            for(size_t i=0;i+2<v.size();i+=3){
                Vec3 a=transform(d.model,v[i].position),b=transform(d.model,v[i+1].position),c=transform(d.model,v[i+2].position);
                if(length(cross(b-a,c-a))<1e-9f)continue;
                triangles.push_back({a,b,c,normalTransform(d.model,v[i].normal),normalTransform(d.model,v[i+1].normal),normalTransform(d.model,v[i+2].normal),material});
            }
        }
        triangleCount=triangles.size();order.resize(triangleCount);std::iota(order.begin(),order.end(),0);buildNode(0,int(order.size()));
        height=std::max(1,int(std::round(width/aspect)));std::vector<unsigned char> pixels(width*height*3);
        Vec3 right=normalize(cross(forward,{0,1,0})),up=cross(right,forward);float spread=std::tan(radians(43)*.5f);
        for(int y=0;y<height;++y)for(int x=0;x<width;++x){
            Vec3 direction=normalize(forward+right*((2*(x+.5f)/width-1)*aspect*spread)+up*((2*(y+.5f)/height-1)*spread));
            Vec3 color=shade({eye,direction},0);int offset=(y*width+x)*3;
            for(int c=0;c<3;++c){float value=std::max(0.f,component(color,c)*exposure);
                pixels[offset+c]=static_cast<unsigned char>(255*std::pow(value/(1+value),1/2.2f));}
        }
        if(!texture){glGenTextures(1,&texture);glGenVertexArrays(1,&vao);
            program=makeProgram(R"GLSL(#version 330 core
out vec2 uv;void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);uv=p;gl_Position=vec4(p*2-1,0,1);})GLSL",
                R"GLSL(#version 330 core
in vec2 uv;out vec4 color;uniform sampler2D picture;void main(){color=texture(picture,uv);})GLSL");
        }
        glBindTexture(GL_TEXTURE_2D,texture);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        milliseconds=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    }
    void draw(int w,int h){glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,w,h);glDisable(GL_DEPTH_TEST);glDisable(GL_BLEND);
        glUseProgram(program);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture);glUniform1i(glGetUniformLocation(program,"picture"),0);
        glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,3);}
    ~RaySnapshot(){if(texture){glDeleteTextures(1,&texture);glDeleteVertexArrays(1,&vao);glDeleteProgram(program);}}
};
}
