#pragma once
#include "renderer.h"
#include <array>
#include <cctype>

namespace cg {
// Tiny built-in bitmap lettering, rendered as triangles. No font/image assets.
inline std::array<unsigned char,5> glyph(char c) {
    static const unsigned char letters[][5]={
        {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},
        {127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},
        {0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},
        {127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},
        {62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},
        {63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},
        {3,4,120,4,3},{97,81,73,69,67}};
    static const unsigned char digits[][5]={
        {62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},
        {24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},
        {54,73,73,73,54},{6,73,73,41,30}};
    c=char(std::toupper(static_cast<unsigned char>(c)));
    if(c>='A'&&c<='Z'){auto p=letters[c-'A'];return {p[0],p[1],p[2],p[3],p[4]};}
    if(c>='0'&&c<='9'){auto p=digits[c-'0'];return {p[0],p[1],p[2],p[3],p[4]};}
    switch(c){case '/':return {64,32,16,8,4};case '-':return {8,8,8,8,8};case '.':return {0,96,96,0,0};
        case ':':return {0,54,54,0,0};case '+':return {8,8,62,8,8};case '%':return {99,19,8,100,99};
        case '[':return {0,127,65,65,0};case ']':return {0,65,65,127,0};case '|':return {0,0,127,0,0};default:return {};}
}
struct HudVertex {float x,y;Vec3 color;};
struct UiRect {
    float x,y,w,h;
    bool contains(float px,float py) const {return px>=x&&px<x+w&&py>=y&&py<y+h;}
};
struct UiLayout {
    float factor,canvasWidth,height,left;
    UiLayout(int w,int h){factor=std::min(w/1280.f,h/720.f);canvasWidth=w/factor;height=h/factor;left=(canvasWidth-1280)*.5f;}
};
class Hud {
    GLuint program=0,vao=0,vbo=0;
    std::vector<HudVertex> vertices;
public:
    float width=1280,height=720,canvasWidth=1280,left=0;
    Hud(){program=makeProgram(R"GLSL(#version 330 core
layout(location=0)in vec2 p;layout(location=1)in vec3 color;uniform vec2 viewport;uniform float left;
out vec3 c;void main(){c=color;gl_Position=vec4((p.x+left)/viewport.x*2.0-1.0,1.0-p.y/viewport.y*2.0,0,1);})GLSL",
        R"GLSL(#version 330 core
in vec3 c;out vec4 frag;void main(){frag=vec4(c,1);})GLSL");
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(HudVertex),nullptr);
        glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(HudVertex),reinterpret_cast<void*>(sizeof(float)*2));}
    void begin(int w,int h){UiLayout l(w,h);width=1280;height=l.height;canvasWidth=l.canvasWidth;left=l.left;vertices.clear();}
    void rect(float x,float y,float w,float h,Vec3 c){vertices.insert(vertices.end(),{{x,y,c},{x+w,y,c},{x+w,y+h,c},{x,y,c},{x+w,y+h,c},{x,y+h,c}});}
    void text(float x,float y,const std::string& value,float size,Vec3 c){for(char ch:value){auto g=glyph(ch);for(int col=0;col<5;++col)for(int row=0;row<7;++row)if(g[col]&(1<<row))rect(x+col*size,y+row*size,size,size,c);x+=6*size;}}
    void button(UiRect r,const std::string& label,bool hover,bool primary=false){
        Vec3 edge=primary?Vec3{.95f,.65f,.29f}:Vec3{.3f,.48f,.55f};
        rect(r.x,r.y,r.w,r.h,edge);
        rect(r.x+1,r.y+1,r.w-2,r.h-2,hover?Vec3{.17f,.29f,.33f}:Vec3{.065f,.12f,.15f});
        text(r.x+10,r.y+10,label,1.4f,primary?edge:Vec3{.88f,.94f,.95f});
    }
    void badge(const Mat4& vp,Vec3 world,int number,bool active){
        float cx=vp.m[0]*world.x+vp.m[4]*world.y+vp.m[8]*world.z+vp.m[12];
        float cy=vp.m[1]*world.x+vp.m[5]*world.y+vp.m[9]*world.z+vp.m[13];
        float cw=vp.m[3]*world.x+vp.m[7]*world.y+vp.m[11]*world.z+vp.m[15];if(cw<=0)return;
        float x=(cx/cw*.5f+.5f)*canvasWidth-left,y=(.5f-cy/cw*.5f)*height;
        if(x<16||x>width-16||y<90||y>height-90)return;
        Vec3 c=active?Vec3{.95f,.65f,.26f}:Vec3{.44f,.58f,.63f};
        rect(x-13,y-10,27,22,{.06f,.10f,.13f});rect(x-13,y+12,27,2,c);
        std::string s=(number<10?"0":"")+std::to_string(number);text(x-9,y-6,s,1.6f,c);
    }
    void flush(){glDisable(GL_DEPTH_TEST);glUseProgram(program);glUniform2f(glGetUniformLocation(program,"viewport"),canvasWidth,height);glUniform1f(glGetUniformLocation(program,"left"),left);
        glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(HudVertex),vertices.data(),GL_DYNAMIC_DRAW);glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()));}
    ~Hud(){glDeleteBuffers(1,&vbo);glDeleteVertexArrays(1,&vao);glDeleteProgram(program);}
};
}
