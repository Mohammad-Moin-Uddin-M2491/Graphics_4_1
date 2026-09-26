#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "scene.h"
#include "hud.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <stdexcept>

using namespace cg;
// One set of bounds is shared by drawing and click handling, in HUD units.
const UiRect playButton{28,110,126,36},resetButton{164,110,98,36},lightButton{276,110,244,36};
const UiRect dimButton{532,110,38,36},brightButton{700,110,38,36},shadowButton{752,110,170,36},shadeButton{934,110,318,36};
struct App {
    Simulation simulation;
    float yaw=.24f,pitch=.43f,distance=25.5f;
    Vec3 target{.1f,1.25f,0};
    bool dragging=false,labels=true;
    double lastX=0,lastY=0;
    int shading=0,camera=1,lighting=0;
    float brightness=1,uiX=-1,uiY=-1;
    bool shadows=true;
    void togglePlay(){if(simulation.time>=DEMO_END)simulation.reset();simulation.running=!simulation.running;}
    bool clickUi(float x,float y){
        if(playButton.contains(x,y))togglePlay();
        else if(resetButton.contains(x,y))simulation.reset();
        else if(lightButton.contains(x,y))lighting=(lighting+1)%4;
        else if(dimButton.contains(x,y))brightness=std::max(.4f,brightness-.2f);
        else if(brightButton.contains(x,y))brightness=std::min(2.0f,brightness+.2f);
        else if(shadowButton.contains(x,y))shadows=!shadows;
        else if(shadeButton.contains(x,y))shading=(shading+1)%3;
        else return false;
        dragging=false;return true;
    }
    void preset(int n){camera=n;
        if(n==1){yaw=.24f;pitch=.43f;distance=25.5f;target={.1f,1.25f,0};}
        if(n==2){yaw=0;pitch=.10f;distance=25.0f;target={.1f,1.9f,0};}
        if(n==3){yaw=-.22f;pitch=.30f;distance=12;target={-6.25f,2.0f,0};}
        if(n==4){yaw=.30f;pitch=.36f;distance=12.5f;target={3.6f,1.1f,0};}
    }
};
static App& app(GLFWwindow* w){return *static_cast<App*>(glfwGetWindowUserPointer(w));}
static void keyCallback(GLFWwindow* w,int key,int,int action,int){
    if(action!=GLFW_PRESS)return;
    auto& a=app(w);
    switch(key){case GLFW_KEY_ESCAPE:glfwSetWindowShouldClose(w,GL_TRUE);break;
    case GLFW_KEY_SPACE:a.togglePlay();break;
    case GLFW_KEY_R:a.simulation.reset();break;
    case GLFW_KEY_1:case GLFW_KEY_2:case GLFW_KEY_3:case GLFW_KEY_4:a.preset(key-GLFW_KEY_0);break;
    case GLFW_KEY_L:a.shading=(a.shading+1)%3;break;
    case GLFW_KEY_B:a.lighting=(a.lighting+1)%4;break;
    case GLFW_KEY_H:a.labels=!a.labels;break;
    case GLFW_KEY_LEFT:a.simulation.running=false;a.simulation.time=std::max(0.0f,a.simulation.time-.25f);break;
    case GLFW_KEY_RIGHT:a.simulation.running=false;a.simulation.time=std::min(DEMO_END,a.simulation.time+.25f);break;
    case GLFW_KEY_MINUS:a.simulation.speed=std::max(.25f,a.simulation.speed-.25f);break;
    case GLFW_KEY_EQUAL:a.simulation.speed=std::min(2.0f,a.simulation.speed+.25f);break;
    default:break;}
}
static void mouseButton(GLFWwindow* w,int button,int action,int){
    if(button!=GLFW_MOUSE_BUTTON_LEFT)return;
    auto& a=app(w);double x,y;glfwGetCursorPos(w,&x,&y);
    int width,height;glfwGetWindowSize(w,&width,&height);if(width<=0||height<=0)return;
    float ux=float(x)*1280/width,uy=float(y)*1280/width,hh=float(height)*1280/width;
    if(action==GLFW_RELEASE){a.dragging=false;return;}
    if(a.clickUi(ux,uy))return;
    if(action==GLFW_PRESS&&uy>hh-101&&uy<hh-61&&ux>=28&&ux<=1252){
        a.simulation.running=false;a.simulation.time=clamp01((ux-28)/1224)*DEMO_END;a.dragging=false;return;}
    a.dragging=uy>158&&uy<hh-145;a.lastX=x;a.lastY=y;
}
static void cursor(GLFWwindow* w,double x,double y){auto& a=app(w);if(a.dragging){a.yaw-=float(x-a.lastX)*.006f;a.pitch=std::clamp(a.pitch+float(y-a.lastY)*.005f,.06f,1.35f);}a.lastX=x;a.lastY=y;}
static void scroll(GLFWwindow* w,double,double y){auto& a=app(w);a.distance=std::clamp(a.distance-float(y)*1.2f,5.0f,44.0f);}
static std::string fixed(float v,int precision=1){std::ostringstream s;s<<std::fixed<<std::setprecision(precision)<<v;return s.str();}

static void overlay(Hud& h,const App& a,int width,int height,const Mat4& vp,const Pose& pose){
    h.begin(width,height);const Vec3 white{.9f,.94f,.94f},muted{.49f,.61f,.66f},accent{.94f,.65f,.29f},green{.27f,.78f,.68f};
    h.rect(0,0,1280,96,{.04f,.064f,.084f});h.rect(28,24,4,48,accent);
    h.text(46,22,"COG / AUTOMOTIVE CHAIN REACTION",2.8f,white);
    h.text(47,56,"MOHAMMAD MOIN UDDIN MOIN  /  2107111  /  COMPUTER GRAPHICS",1.55f,muted);
    h.text(1040,24,"PROPOSAL BUILD",1.8f,accent);
    h.text(1040,50,"10 MAIN OBJECTS",1.6f,white);
    auto button=[&](UiRect r,const std::string& label,bool primary=false){h.button(r,label,r.contains(a.uiX,a.uiY),primary);};
    button(playButton,a.simulation.running?"PAUSE":"PLAY",true);button(resetButton,"RESET");
    button(lightButton,std::string("LIGHT: ")+lightingPreset(a.lighting).name,true);
    button(dimButton,"-");button(brightButton,"+");
    h.text(582,122,"BRIGHT "+std::to_string(int(std::round(a.brightness*100)))+"%",1.45f,white);
    button(shadowButton,a.shadows?"SHADOWS: ON":"SHADOWS: OFF");
    const char* modes[]={"PHONG","FLAT","GOURAUD"};button(shadeButton,std::string("SHADING: ")+modes[a.shading]);
    int stage=stageAt(a.simulation.time);
    if(a.labels){
        const Vec3 points[]={{-10,4.25f,0},{-8.25f,4.72f,0},{-6.7f,4.3f,0},pose.wheel+Vec3{0,1.05f,0},
            {-.45f,2.95f,0},{1.6f,2.5f,0},{3.05f,2.2f,0},{4.1f,2.2f,0},{5.15f,2.2f,0},{8.65f,2.2f,0}};
        const int stages[]={0,0,1,2,3,4,5,5,5,6};
        for(int i=0;i<10;++i)h.badge(vp,points[i],i+1,stage==stages[i]);
    }
    float y=h.height-145;
    h.rect(0,y,1280,145,{.04f,.064f,.084f});h.rect(28,y,1224,1,{.16f,.24f,.28f});
    h.text(28,y+17,stageName(stage),2.1f,white);
    std::string status=a.simulation.running?"RUNNING":a.simulation.time==0?"READY - PRESS SPACE":a.simulation.time>=DEMO_END?"MILESTONE COMPLETE":"PAUSED";
    h.text(615,y+19,status,1.65f,a.simulation.running?green:accent);
    h.text(1020,y+19,fixed(a.simulation.time)+" / 17.5 SEC",1.6f,muted);
    const char* names[]={"GEAR A","GEAR B","LEVER","WHEEL","WIPER","BAR","PANEL 1","PANEL 2","PANEL 3","CAR MODEL"};
    const float starts[]={0,0,3.05f,4.5f,8.05f,10.15f,DOMINO_START,DOMINO_START+DOMINO_DELAY,DOMINO_START+2*DOMINO_DELAY,99};
    for(int i=0;i<10;++i){float x=28+i*122.4f;bool done=a.simulation.time>starts[i];h.rect(x,y+48,118,28,done?Vec3{.12f,.27f,.27f}:Vec3{.085f,.13f,.16f});h.text(x+7,y+58,names[i],1.4f,done?green:muted);}
    h.rect(28,y+83,1224,4,{.15f,.22f,.25f});h.rect(28,y+83,1224*a.simulation.time/DEMO_END,4,accent);
    h.text(28,y+100,"SPACE START/PAUSE   R RESET   DRAG ORBIT   SCROLL ZOOM   1-4 VIEWS   ARROWS STEP",1.45f,white);
    h.text(28,y+124,"B LIGHTS   L SHADING   H LABELS   -/+ SPEED: "+fixed(a.simulation.speed,2)+"X",1.35f,muted);
    h.text(745,y+124,"NEXT: CAR RELEASE + ENDING SHOT",1.35f,accent);
    h.flush();
}

// CPU-only checks run without opening a window or requiring a GPU.
static int selfTest(){int checks=0;auto check=[&](bool b,const char* name){++checks;if(!b)throw std::runtime_error(std::string("Check failed: ")+name);};
    Vec3 q=transform(translate({4,2,0})*rotateZ(PI/2)*scale({2,1,1}),{1,0,0});
    check(length(q-Vec3{4,4,0})<.0001f,"T R S application order");
    Vec3 pivot{2,3,0};Mat4 m=translate(pivot)*rotateZ(.7f)*translate(pivot*-1);
    check(length(transform(m,pivot)-pivot)<.0001f,"local pivot remains fixed");
    Simulation s;s.update(1);check(s.time==0,"initially paused");s.running=true;s.update(2);check(s.time==2,"clock advances");s.running=false;s.update(2);check(s.time==2,"pause freezes time");
    s.reset();check(s.time==0&&!s.running,"reset clears clock and play state");s.running=true;s.update(100);check(s.time==DEMO_END&&!s.running,"completion clamps and pauses");
    float oldTravel=-1;Pose last=poseAt(0);
    for(int i=0;i<=17500;++i){float t=i*.001f;Pose p=poseAt(t);
        check(std::abs(p.gearSmall+1.5f*p.gearLarge)<.00001f,"gear ratio");
        check(std::abs(p.wheelSpin*WHEEL_RADIUS+p.wheelTravel)<.00001f,"wheel rolling without slip");
        check(p.wheelTravel>=oldTravel,"wheel never reverses");
        check(length(p.wheel-last.wheel)<.003f,"wheel has no teleport");
        check(std::abs(p.lever-last.lever)<.002f&&std::abs(p.wiper-last.wiper)<.002f&&std::abs(p.bar-last.bar)<.002f,"pivot continuity");
        for(int j=0;j<3;++j)check(std::abs(p.domino[j]-last.domino[j])<.002f,"panel continuity");
        oldTravel=p.wheelTravel;last=p;
    }
    auto p=poseAt(0);check(p.gearSmall==0&&p.wiper==0&&p.domino[2]==0,"reset pose");
    check(poseAt(4.49f).wheelTravel==0,"wheel waits for release");
    check(poseAt(8.04f).wiper==0,"wiper waits for wheel");
    check(poseAt(DOMINO_START-.01f).domino[0]==0,"panels wait for bar");
    check(poseAt(DEMO_END).domino[2]<-1.3f,"last panel completes");
    for(int i=0;i<=240;++i){float t=8.05f+i*.01f;auto contact=poseAt(t);
        float paddleX=-1.05f+.3f*smooth(phase(t,8.05f,2.4f));
        check(std::abs(contact.wheel.x+WHEEL_RADIUS-(paddleX-.05f))<.0001f,"wheel follows paddle contact");}
    check(std::abs(1.05f*std::cos(poseAt(DEMO_END).domino[0])-.13f)<.0001f,"resting panels touch");
    App ui;ui.simulation.time=6.5f;ui.simulation.running=true;
    for(int mode=1;mode<=4;++mode){check(ui.clickUi(300,125),"lighting button consumes click");check(ui.lighting==mode%4,"all lighting presets selectable");}
    check(ui.simulation.time==6.5f&&ui.simulation.running,"lighting does not interrupt animation");
    ui.clickUi(770,125);check(!ui.shadows,"shadow button toggles");
    for(int i=0;i<20;++i)ui.clickUi(715,125);
    check(ui.brightness==2,"brightness upper bound");
    for(int i=0;i<20;++i)ui.clickUi(545,125);
    check(ui.brightness==.4f,"brightness lower bound");
    ui.clickUi(50,125);check(!ui.simulation.running,"on-screen pause");
    ui.clickUi(50,125);check(ui.simulation.running,"on-screen play");
    ui.clickUi(180,125);check(ui.simulation.time==0&&!ui.simulation.running,"on-screen reset");
    check(!ui.clickUi(400,250),"scene clicks do not change controls");
    std::cout<<"PASS: "<<checks<<" animation and transformation checks\n";return 0;
}

int main(int argc,char** argv){
    bool capture=false;std::string captureDir="docs/screenshots";
    try{if(argc>1&&std::string(argv[1])=="--self-test")return selfTest();
        if(argc>1&&std::string(argv[1])=="--capture"){capture=true;if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
    glfwSetErrorCallback([](int code,const char* text){std::cerr<<"GLFW "<<code<<": "<<text<<'\n';});
    if(!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,4);if(capture)glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    GLFWwindow* window=glfwCreateWindow(1440,900,"COG | 3D Automotive Chain Reaction | 2107111",nullptr,nullptr);
    if(!window){glfwTerminate();return 1;}glfwMakeContextCurrent(window);glfwSwapInterval(capture?0:1);
    int result=0;
    try{
        if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))throw std::runtime_error("GLAD could not load OpenGL");
        std::cout<<"OpenGL: "<<glGetString(GL_VERSION)<<"\nRenderer: "<<glGetString(GL_RENDERER)<<'\n';
        glEnable(GL_MULTISAMPLE);
        App state;
        // Start as soon as the window is ready. The exhibit loops for a live
        // presentation; Space still pauses it and R resets it to the first pose.
        state.simulation.running=!capture;
        glfwSetWindowUserPointer(window,&state);glfwSetKeyCallback(window,keyCallback);glfwSetMouseButtonCallback(window,mouseButton);glfwSetCursorPosCallback(window,cursor);glfwSetScrollCallback(window,scroll);
        Renderer renderer;Hud hud;Scene scene;
        double previous=glfwGetTime();int frame=0;
        const float sampleTimes[]={0,3.8f,6.5f,9.8f,11.4f,12.7f,14.7f,17.5f,3.8f,9.8f,12.7f,17.5f,6.5f,6.5f};
        const int sampleCameras[]={1,1,1,1,1,1,1,1,3,4,4,2,3,3};
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();double now=glfwGetTime();float dt=static_cast<float>(now-previous);previous=now;
            int width,height;glfwGetFramebufferSize(window,&width,&height);
            if(width==0||height==0){glfwWaitEventsTimeout(.1);continue;}
            if(capture){
                if(frame<14){state.simulation.time=sampleTimes[frame];state.preset(sampleCameras[frame]);state.shading=frame==12?1:frame==13?2:0;}
                else {state.simulation.time=3.8f;state.preset(frame<18?1:3);state.shading=0;
                    state.lighting=3;for(int i=0;i<=(frame-14)%4;++i)state.clickUi(300,125);
                    state.shadows=frame!=22;state.brightness=frame==23?.4f:1.0f;}
            }
            else {
                bool wasRunning=state.simulation.running;
                state.simulation.update(std::min(dt,.1f));
                if(wasRunning&&state.simulation.time>=DEMO_END){state.simulation.time=0;state.simulation.running=true;}
            }
            double mouseX,mouseY;int windowWidth,windowHeight;glfwGetCursorPos(window,&mouseX,&mouseY);glfwGetWindowSize(window,&windowWidth,&windowHeight);
            if(windowWidth>0){state.uiX=float(mouseX)*1280/windowWidth;state.uiY=float(mouseY)*1280/windowWidth;}
            Pose pose=poseAt(state.simulation.time);scene.build(pose,state.simulation.time);
            Vec3 eye=state.target+Vec3{std::sin(state.yaw)*std::cos(state.pitch),std::sin(state.pitch),std::cos(state.yaw)*std::cos(state.pitch)}*state.distance;
            Mat4 view=lookAt(eye,state.target,{0,1,0}),projection=perspective(radians(43),float(width)/height,.1f,180);
            renderer.draw(scene.items,view,projection,eye,width,height,state.shading,state.lighting,state.brightness,state.shadows);overlay(hud,state,width,height,projection*view,pose);
            GLenum error=glGetError();if(error!=GL_NO_ERROR)throw std::runtime_error("OpenGL error: "+std::to_string(error));
            if(capture){std::string name=captureDir+"/frame_"+(frame<10?"0":"")+std::to_string(frame)+".bmp";captureBMP(name,width,height);std::cout<<"Captured "<<name<<" at t="<<state.simulation.time<<'\n';}
            glfwSwapBuffers(window);
            if(capture&&++frame==24)glfwSetWindowShouldClose(window,GL_TRUE);
        }
    }catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';result=1;}
    glfwDestroyWindow(window);glfwTerminate();return result;
}
