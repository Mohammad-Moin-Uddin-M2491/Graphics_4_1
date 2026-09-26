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
const UiRect playButton{820,8,88,30},resetButton{728,8,80,30},lightButton{920,8,225,30},toolsButton{1157,8,100,30};
const UiRect dimButton{728,54,38,30},brightButton{896,54,38,30},shadowButton{18,54,170,30},shadeButton{200,54,305,30};
struct App {
    Simulation simulation;
    float yaw=.28f,pitch=.57f,distance=22.5f;
    Vec3 target{.6f,.8f,1.8f};
    bool dragging=false,labels=false,controls=false;
    double lastX=0,lastY=0;
    int shading=0,camera=1,lighting=0;
    float brightness=1,uiX=-1,uiY=-1;
    bool shadows=true;
    void togglePlay(){if(simulation.time>=DEMO_END)simulation.reset();simulation.running=!simulation.running;}
    bool clickUi(float x,float y){
        if(playButton.contains(x,y))togglePlay();
        else if(resetButton.contains(x,y))simulation.reset();
        else if(lightButton.contains(x,y))lighting=(lighting+1)%4;
        else if(toolsButton.contains(x,y))controls=!controls;
        else if(controls&&dimButton.contains(x,y))brightness=std::max(.4f,brightness-.2f);
        else if(controls&&brightButton.contains(x,y))brightness=std::min(2.0f,brightness+.2f);
        else if(controls&&shadowButton.contains(x,y))shadows=!shadows;
        else if(controls&&shadeButton.contains(x,y))shading=(shading+1)%3;
        else return false;
        dragging=false;return true;
    }
    void preset(int n){camera=n;
        if(n==1){yaw=.28f;pitch=.57f;distance=22.5f;target={.6f,.8f,1.8f};}
        if(n==2){yaw=.48f;pitch=.45f;distance=15.5f;target={4.8f,1.45f,1.1f};}
        if(n==3){yaw=-.22f;pitch=.30f;distance=12;target={-6.25f,2.0f,0};}
        if(n==4){yaw=.62f;pitch=.43f;distance=13.0f;target={7.5f,1.3f,2.2f};}
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
    case GLFW_KEY_C:a.controls=!a.controls;break;
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
    if(action==GLFW_PRESS&&uy>hh-54&&uy<hh-40&&ux>=28&&ux<=1252){
        a.simulation.running=false;a.simulation.time=clamp01((ux-28)/1224)*DEMO_END;a.dragging=false;return;}
    a.dragging=uy>(a.controls?90:44)&&uy<hh-54;a.lastX=x;a.lastY=y;
}
static void cursor(GLFWwindow* w,double x,double y){auto& a=app(w);if(a.dragging){a.yaw-=float(x-a.lastX)*.006f;a.pitch=std::clamp(a.pitch+float(y-a.lastY)*.005f,.06f,1.35f);}a.lastX=x;a.lastY=y;}
static void scroll(GLFWwindow* w,double,double y){auto& a=app(w);a.distance=std::clamp(a.distance-float(y)*1.2f,5.0f,44.0f);}
static std::string fixed(float v,int precision=1){std::ostringstream s;s<<std::fixed<<std::setprecision(precision)<<v;return s.str();}

static void overlay(Hud& h,const App& a,int width,int height,const Mat4& vp,const Pose& pose){
    h.begin(width,height);
    const Vec3 white{.83f,.86f,.86f},muted{.48f,.55f,.57f},accent{.72f,.58f,.36f};
    h.rect(0,0,1280,44,{.035f,.045f,.05f});
    h.text(20,15,"AUTOMOTIVE CHAIN REACTION",1.8f,white);
    h.text(500,17,"MOIN / 2107111",1.3f,muted);
    auto button=[&](UiRect r,const std::string& label,bool active=false){h.button(r,label,r.contains(a.uiX,a.uiY),active);};
    button(playButton,a.simulation.time>=DEMO_END?"REPLAY":a.simulation.running?"PAUSE":"PLAY");
    button(resetButton,"RESET");button(lightButton,std::string("LIGHT: ")+lightingPreset(a.lighting).name);
    button(toolsButton,"TOOLS",a.controls);
    if(a.controls){
        h.rect(0,44,1280,46,{.045f,.06f,.07f});
        button(shadowButton,a.shadows?"SHADOWS: ON":"SHADOWS: OFF");
        const char* modes[]={"BLINN-PHONG","FLAT","GOURAUD"};
        button(shadeButton,std::string("SHADING: ")+modes[a.shading]);
        button(dimButton,"-");button(brightButton,"+");
        h.text(780,64,"LIGHT "+std::to_string(int(std::round(a.brightness*100)))+"%",1.3f,white);
        h.text(980,64,"C CLOSE TOOLS",1.3f,muted);
    }
    int stage=stageAt(a.simulation.time);
    if(a.labels){
        const Vec3 points[]={{-10,4.25f,0},{-8.25f,4.72f,0},{-6.7f,4.3f,0},pose.wheel+Vec3{0,1.05f,0},
            {-.45f,2.95f,0},{1.6f,2.5f,0},{3.55f,2.65f,0},{6.0f,2.25f,0},{6.9f,2.6f,0},{7.65f,3.95f,0},{9,2.1f,2.2f},{9,2.0f,.4f+pose.carTravel}};
        const int stages[]={0,0,1,2,3,4,5,5,6,7,7,8};
        for(int i=0;i<12;++i)h.badge(vp,points[i],i+1,stage==stages[i]);
        h.rect(18,h.height-101,1240,42,{.04f,.055f,.065f});
        h.text(28,h.height-90,"GEARS > LEVER > WHEEL > WIPER > BAR > CRANK/PISTON > CATCH > PULLEY > CAR",1.6f,muted);
        h.text(28,h.height-73,"X: CHAIN DIRECTION   Y: HEIGHT   Z: CAR EXIT   SPEED: "+fixed(a.simulation.speed,2)+"X",1.3f,muted);
    }
    float y=h.height-48;
    h.rect(0,y,1280,48,{.035f,.045f,.05f});
    h.rect(28,y-3,1224,2,{.16f,.19f,.2f});h.rect(28,y-3,1224*a.simulation.time/DEMO_END,2,accent);
    h.text(22,y+10,a.simulation.time>=DEMO_END?"FINISHED / SPACE TO REPLAY":stageName(stage),1.55f,white);
    h.text(850,y+10,(a.simulation.running?"PLAYING  ":"PAUSED  ")+fixed(a.simulation.time)+" / 25 SEC",1.35f,muted);
    h.text(22,y+31,"SPACE PLAY/PAUSE   R RESET   1 OVERVIEW   2 PRESENTATION   3-4 CLOSE VIEWS   H DETAILS   C TOOLS   B LIGHT",1.18f,muted);
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
    Pose last=poseAt(0);
    for(int i=0;i<=3000;++i){float t=i/120.0f;Pose p=poseAt(t);
        check(std::abs(p.gearSmall+1.5f*p.gearLarge)<.00001f,"gear ratio");
        check(std::abs(p.wheelSpin*WHEEL_RADIUS+p.wheelTravel)<.00001f,"wheel rolling");
        check(std::abs(length(sliderPin(p.crank)-crankPin(p.crank))-ROD_LENGTH)<.00001f,"connecting rod remains rigid");
        check(p.weightDrop==p.gateLift,"cable length preserved");
        check(std::abs(p.carSpin*CAR_WHEEL_RADIUS+p.carTravel)<.00001f,"car wheels match distance");
        check(p.carTravel>=last.carTravel&&p.carTravel-last.carTravel<.025f,"car continuous forward movement");
        check(length(p.wheel-last.wheel)<.025f,"wheel has no teleport");
        check(std::abs(p.crank-last.crank)<.025f&&std::abs(p.catchAngle-last.catchAngle)<.025f,"new pivot continuity");
        if(p.carTravel>0)check(p.gateLift>=1.39f,"barrier clears roof before car starts");
        if(t<WEIGHT_START)check(p.weightDrop==0,"catch holds counterweight");
        last=p;
    }
    auto p=poseAt(0);check(p.crank==CRANK_INITIAL&&p.carTravel==0&&p.weightDrop==0&&p.catchAngle==0,"new mechanisms reset");
    check(poseAt(4.49f).wheelTravel==0&&poseAt(8.04f).wiper==0,"upstream trigger waits");
    check(poseAt(CRANK_START-.01f).crank==CRANK_INITIAL,"crank waits for bar");
    check(poseAt(DEMO_END).carTravel==4,"car completes four-unit exit");
    check(.36f+poseAt(CAR_START).gateLift-.08f > .03f+1.51f+.065f/2+.05f,"physical roof clearance margin");
    float latchEnd=poseAt(WEIGHT_START).catchAngle;
    check(6.7f+.85f*std::cos(latchEnd)-.05f*std::sin(latchEnd)<7.21f-.39f,"catch fully withdraws from weight path");
    for(int i=0;i<=240;++i){float t=8.05f+i*.01f;auto contact=poseAt(t);
        float paddleX=-1.05f+.3f*smooth(phase(t,8.05f,2.4f));
        check(std::abs(contact.wheel.x+WHEEL_RADIUS-(paddleX-.05f))<.0001f,"wheel follows paddle contact");}
    App ui;ui.simulation.time=6.5f;ui.simulation.running=true;
    auto click=[&](UiRect r){return ui.clickUi(r.x+r.w*.5f,r.y+r.h*.5f);};
    for(int mode=1;mode<=4;++mode){check(click(lightButton),"lighting click consumed");check(ui.lighting==mode%4,"all lighting presets selectable");}
    check(ui.simulation.time==6.5f&&ui.simulation.running,"lighting independent of animation");
    check(!click(shadowButton),"hidden tools do not intercept scene");click(toolsButton);
    click(shadowButton);check(!ui.shadows,"shadow toggle");
    for(int i=0;i<20;++i)click(brightButton);
    check(ui.brightness==2,"brightness limit");
    click(playButton);check(!ui.simulation.running,"pause button");click(playButton);check(ui.simulation.running,"play button");
    click(resetButton);check(ui.simulation.time==0&&!ui.simulation.running,"reset button");
    ui.simulation.time=DEMO_END;click(playButton);check(ui.simulation.time==0&&ui.simulation.running,"replay after final hold");
    for(int view=1;view<=4;++view){ui.preset(view);check(ui.distance>0&&ui.camera==view,"camera presets");}
    std::cout<<"PASS: "<<checks<<" animation and transformation checks\n";return 0;
}

int main(int argc,char** argv){
    bool capture=false,verify=false;std::string captureDir="docs/screenshots";
    try{if(argc>1&&std::string(argv[1])=="--self-test")return selfTest();
        if(argc>1&&std::string(argv[1])=="--capture"){capture=true;if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
        if(argc>1&&std::string(argv[1])=="--verify-playback")verify=true;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
    glfwSetErrorCallback([](int code,const char* text){std::cerr<<"GLFW "<<code<<": "<<text<<'\n';});
    if(!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,4);if(capture||verify)glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    GLFWwindow* window=glfwCreateWindow(1440,900,"COG | 3D Automotive Chain Reaction | 2107111",nullptr,nullptr);
    if(!window){glfwTerminate();return 1;}glfwMakeContextCurrent(window);glfwSwapInterval(capture||verify?0:1);
    int result=0;
    try{
        if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))throw std::runtime_error("GLAD could not load OpenGL");
        std::cout<<"OpenGL: "<<glGetString(GL_VERSION)<<"\nRenderer: "<<glGetString(GL_RENDERER)<<'\n';
        glEnable(GL_MULTISAMPLE);
        App state;
        // Start automatically, then hold the completed scene for replay or reset.
        state.simulation.running=!capture;
        glfwSetWindowUserPointer(window,&state);glfwSetKeyCallback(window,keyCallback);glfwSetMouseButtonCallback(window,mouseButton);glfwSetCursorPosCallback(window,cursor);glfwSetScrollCallback(window,scroll);
        Renderer renderer;Hud hud;Scene scene;
        if(verify){
            // Exercise the real registered handlers with a valid GLFW context.
            keyCallback(window,GLFW_KEY_SPACE,0,GLFW_PRESS,0);state.simulation.update(1);
            if(state.simulation.time!=0||state.simulation.running)throw std::runtime_error("Pause handler failed");
            for(int k=GLFW_KEY_1;k<=GLFW_KEY_4;++k)keyCallback(window,k,0,GLFW_PRESS,0);
            state.dragging=true;cursor(window,20,15);state.dragging=false;scroll(window,0,1);
            if(state.camera!=4||state.distance>=13||state.yaw==.62f)throw std::runtime_error("Camera handlers failed");
            keyCallback(window,GLFW_KEY_B,0,GLFW_PRESS,0);keyCallback(window,GLFW_KEY_L,0,GLFW_PRESS,0);
            if(state.lighting!=1||state.shading!=1)throw std::runtime_error("Lighting/shading handlers failed");
            keyCallback(window,GLFW_KEY_R,0,GLFW_PRESS,0);keyCallback(window,GLFW_KEY_SPACE,0,GLFW_PRESS,0);
            state.preset(1);state.lighting=state.shading=0;
        }
        double previous=glfwGetTime();int frame=0;
        const float sampleTimes[]={0,3.8f,6.5f,9.8f,11.15f,11.9f,12.8f,13.5f,14.5f,15.5f,16.7f,18.8f,20.8f,25,11.15f,12.8f,14,15.7f,16.7f,20,25,3.8f,6.5f,6.5f};
        const int sampleCameras[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,4,4,4,4,3,3,3};
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();double now=glfwGetTime();float dt=static_cast<float>(now-previous);previous=now;
            int width,height;glfwGetFramebufferSize(window,&width,&height);
            if(width==0||height==0){glfwWaitEventsTimeout(.1);continue;}
            if(capture){
                if(frame<24){state.simulation.time=sampleTimes[frame];state.preset(sampleCameras[frame]);state.shading=frame==22?1:frame==23?2:0;}
                else {state.shading=0;
                    state.simulation.time=15.5f;state.preset(2);state.lighting=(frame-24)%4;
                    state.shadows=frame!=28;state.brightness=frame==29?.4f:1.0f;
                    state.controls=state.labels=frame>=30;}
            }
            else {
                state.simulation.update(verify?1.0f/30:std::min(dt,.1f));
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
            if(capture&&++frame==32)glfwSetWindowShouldClose(window,GL_TRUE);
            if(verify){++frame;if(state.simulation.time>=DEMO_END){
                if(pose.carTravel!=4||state.simulation.running)throw std::runtime_error("Playback did not finish correctly");
                std::cout<<"PASS: complete 25-second GPU playback, "<<frame<<" frames; pause/reset, camera, lighting and shading handlers\n";
                glfwSetWindowShouldClose(window,GL_TRUE);}}
        }
    }catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';result=1;}
    glfwDestroyWindow(window);glfwTerminate();return result;
}
