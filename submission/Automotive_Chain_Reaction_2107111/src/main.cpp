#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "scene.h"
#include "hud.h"
#include "raytrace.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <stdexcept>

using namespace cg;
// Drawing and hit testing share exactly the same button bounds (HUD units).
const UiRect modeButton{390,8,244,30},resetButton{645,8,85,30},playButton{741,8,90,30};
const UiRect toolsButton{1113,8,145,30};
const UiRect shadowButton{42,245,265,42},shadeButton{325,245,450,42},dimButton{42,340,52,42},brightButton{290,340,52,42};
const UiRect rayButton{42,430,575,42},closeGuideButton{1078,68,150,36};
enum GuidePage {CameraPage,OperatePage,DrivePage,LightingPage,ChainFirstPage,ChainLastPage,GuidePageCount};
static UiRect guideTab(int page){return {42.f+198*page,145,186,36};}
struct App {
    const Renderer* renderer=nullptr;const Scene* scene=nullptr;
    Simulation simulation;Camera camera;CarState car;
    bool looking=false,labels=false,controls=false;
    int guidePage=0;std::array<bool,GLFW_KEY_LAST+1> held{};
    double lastX=0,lastY=0;int shading=0,lighting=0;
    float brightness=1,uiX=-1,uiY=-1;bool shadows=true;
    bool rayRequested=false,rayActive=false,resumeAfterRay=false;
    void closeRay(){rayActive=false;simulation.running=resumeAfterRay;}
    void cycleLighting(){lighting=(lighting+1)%4;}
    bool clickLamp(float x,float y,int width,int height){
        if(controls||rayActive||looking||!renderer||!scene||width<=0||height<=0)return false;
        UiLayout layout(width,height);float hudY=y/layout.factor;
        if(x<0||x>=width||hudY<44||hudY>=layout.height-65)return false;
        Vec3 direction=cameraRay(camera.forward(),2*x/width-1,1-2*y/height,float(width)/height);
        if(!scene->isLamp(renderer->pick(scene->items,camera.eye,direction,dot(direction,camera.forward()))))return false;
        cycleLighting();return true;
    }
    void reset(){simulation.reset();car.reset();held.fill(false);}
    int mechanismHeld()const{int s=stageAt(simulation.time);if(s>=ACTION_COUNT)return 0;
        int key=stageControl(s).key;return held[key]?key:0;}
    void toggleMode(){simulation.interactive=!simulation.interactive;reset();if(!simulation.interactive)simulation.running=true;}
    bool needsKeyGuide()const{int s=stageAt(simulation.time);
        if(!simulation.interactive||s>=ACTION_COUNT)return false;
        auto control=stageControl(s);
        return control.operation!=Operation::Release&&(simulation.waiting||(simulation.running&&!mechanismHeld()));
    }
    std::string playLabel()const{
        if(simulation.blockedStage>=0)return "CHECK";
        if(needsKeyGuide())return std::string(1,char(stageControl(stageAt(simulation.time)).key))+" GUIDE";
        return simulation.waiting?"F GO":simulation.running?"PAUSE":"PLAY";
    }
    void togglePlay(bool button=false){
        if(simulation.blockedStage>=0||(needsKeyGuide()&&(button||simulation.waiting))){controls=true;guidePage=OperatePage;return;}
        if(simulation.waiting){simulation.activate();return;}
        simulation.running=!simulation.running;
    }
    bool clickUi(float x,float y){
        if(rayActive){closeRay();return true;}
        if(playButton.contains(x,y))togglePlay(true);
        else if(resetButton.contains(x,y))reset();
        else if(modeButton.contains(x,y))toggleMode();
        else if(toolsButton.contains(x,y))controls=!controls;
        else if(controls&&closeGuideButton.contains(x,y))controls=false;
        else if(controls&&guidePage==LightingPage&&dimButton.contains(x,y))brightness=std::max(.4f,brightness-.2f);
        else if(controls&&guidePage==LightingPage&&brightButton.contains(x,y))brightness=std::min(2.0f,brightness+.2f);
        else if(controls&&guidePage==LightingPage&&shadowButton.contains(x,y))shadows=!shadows;
        else if(controls&&guidePage==LightingPage&&shadeButton.contains(x,y))shading=(shading+1)%3;
        else if(controls&&guidePage==LightingPage&&rayButton.contains(x,y))rayRequested=true;
        else {if(controls)for(int i=0;i<GuidePageCount;++i)if(guideTab(i).contains(x,y)){guidePage=i;return true;}return controls;}
        return true;
    }
};
static App& app(GLFWwindow* w){return *static_cast<App*>(glfwGetWindowUserPointer(w));}
static void releaseMouse(GLFWwindow* w){app(w).looking=false;glfwSetInputMode(w,GLFW_CURSOR,GLFW_CURSOR_NORMAL);}
static void keyCallback(GLFWwindow* w,int key,int,int action,int){
    auto& a=app(w);
    if(key>=0&&key<=GLFW_KEY_LAST&&action!=GLFW_REPEAT)a.held[key]=action==GLFW_PRESS;
    if(action!=GLFW_PRESS)return;
    if(a.rayActive){if(key==GLFW_KEY_T||key==GLFW_KEY_ESCAPE)a.closeRay();return;}
    switch(key){
    case GLFW_KEY_ESCAPE:a.controls=false;releaseMouse(w);break;
    case GLFW_KEY_SPACE:a.togglePlay();break;
    case GLFW_KEY_R:a.reset();break;
    case GLFW_KEY_I:a.toggleMode();break;
    case GLFW_KEY_F:a.simulation.activate();break;
    case GLFW_KEY_1:case GLFW_KEY_2:case GLFW_KEY_3:case GLFW_KEY_4:case GLFW_KEY_5:case GLFW_KEY_6:a.camera.preset(key-GLFW_KEY_0);break;
    case GLFW_KEY_L:a.shading=a.shading==2?0:2;break;
    case GLFW_KEY_B:a.cycleLighting();break;
    case GLFW_KEY_H:a.labels=!a.labels;break;
    case GLFW_KEY_C:a.controls=!a.controls;releaseMouse(w);break;
    case GLFW_KEY_T:a.rayRequested=true;releaseMouse(w);break;
    default:a.simulation.operate(key);break;}
}
static void leftClick(GLFWwindow* w,float x,float y){
    auto& a=app(w);if(a.looking)return;
    int width,height;glfwGetWindowSize(w,&width,&height);
    if(width>0&&height>0){UiLayout layout(width,height);
        if(!a.clickUi(x/layout.factor-layout.left,y/layout.factor))a.clickLamp(x,y,width,height);}
}
static void mouseButton(GLFWwindow* w,int button,int action,int){
    auto& a=app(w);double x,y;glfwGetCursorPos(w,&x,&y);
    if(a.rayActive){if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS)a.closeRay();return;}
    if(button==GLFW_MOUSE_BUTTON_RIGHT){
        if(action==GLFW_PRESS&&!a.controls){a.looking=true;a.lastX=x;a.lastY=y;glfwSetInputMode(w,GLFW_CURSOR,GLFW_CURSOR_DISABLED);}
        else releaseMouse(w);
        return;
    }
    if(button!=GLFW_MOUSE_BUTTON_LEFT||action!=GLFW_PRESS||a.looking)return;
    leftClick(w,float(x),float(y));
}
static void cursor(GLFWwindow* w,double x,double y){auto& a=app(w);
    if(a.looking)a.camera.look(float(x-a.lastX),float(y-a.lastY));
    a.lastX=x;a.lastY=y;
}
static void scroll(GLFWwindow* w,double,double y){auto& a=app(w);if(!a.rayActive&&!a.controls)a.camera.eye=a.camera.eye+a.camera.forward()*float(y)*.75f;}
static void focus(GLFWwindow* w,int focused){if(!focused){releaseMouse(w);app(w).held.fill(false);}}
static std::string fixed(double v,int precision=1){std::ostringstream s;s<<std::fixed<<std::setprecision(precision)<<v;return s.str();}
static std::string nextAction(const App& a){
    int stage=stageAt(a.simulation.time);
    if(a.simulation.blockedStage>=0)return "CONTACT CHECK: "+std::string(handoffFailure(a.simulation.blockedStage));
    if(!a.simulation.running&&!a.simulation.waiting)return "SPACE / START OR RESUME THE CURRENT ACTION";
    if(stage==ACTION_COUNT)return "ARROWS / DRIVE THE RELEASED CAR / 4 FOR EXIT VIEW";
    if(!a.simulation.interactive)return "AUTOMATIC / NEXT: "+std::string(stageName(stage+1));
    const auto& control=stageControl(stage);
    if(!a.simulation.waiting&&control.operation==Operation::Release)return "ACTION IN MOTION / WAIT FOR THE NEXT MECHANISM";
    return control.instruction;
}
static void overlay(Hud& h,const App& a,int width,int height,const Mat4& vp,const Pose& p){
    h.begin(width,height);
    const Vec3 white{.89f,.92f,.92f},muted{.64f,.71f,.73f},accent{.86f,.68f,.40f},panel{.045f,.066f,.078f};
    if(a.rayActive){
        h.rect(-h.left,0,h.canvasWidth,44,{.035f,.045f,.05f});h.text(20,16,"CPU RAY-TRACED SNAPSHOT / T, ESC OR LEFT CLICK TO RETURN",1.6f,white);
        h.rect(-h.left,h.height-44,h.canvasWidth,44,{.035f,.045f,.05f});
        h.text(20,h.height-31,"TRIANGLE INTERSECTIONS / SHADOW RAYS / ONE REFLECTION BOUNCE / 800 PX WIDE",1.4f,accent);
        h.text(20,h.height-13,"FROZEN FRAME / RETURN TO OPENGL FOR CAMERA, DRIVING AND ANIMATION",1.2f,muted);h.flush();return;
    }
    int stage=stageAt(a.simulation.time);
    h.rect(-h.left,0,h.canvasWidth,44,{.035f,.045f,.05f});h.text(18,16,"AUTOMOTIVE CHAIN REACTION",1.5f,white);
    auto button=[&](UiRect r,const std::string& text,bool active=false){h.button(r,text,r.contains(a.uiX,a.uiY),active);};
    button(modeButton,a.simulation.interactive?"MODE: INTERACTIVE [I]":"MODE: PRESENTATION [I]");
    button(playButton,a.playLabel());
    button(resetButton,"RESET");h.text(848,18,std::string("LAMP: ")+lightingPreset(a.lighting).name+" [B]",1.4f,accent);
    button(toolsButton,"GUIDE [C]",a.controls);
    if(!a.controls){
        Vec3 point=LAMP_BULB+Vec3{0,1.30f,0};
        float w=vp.m[3]*point.x+vp.m[7]*point.y+vp.m[11]*point.z+vp.m[15];
        if(w>0){Vec3 clip=transform(vp,point);
            float x=(clip.x/w*.5f+.5f)*h.canvasWidth-h.left,y=(.5f-clip.y/w*.5f)*h.height;
            if(x>85&&x<1195&&y>65&&y<h.height-90){
                h.rect(x-83,y-13,166,27,panel);h.text(x-73,y-5,"CLICK LAMP / B",1.8f,accent);
            }
        }
    }
    if(a.labels&&!a.controls){
        const Vec3 points[]={p.hand+Vec3{0,.4f,0},{-9,4.6f,0},{-6.7f,4.2f,0},p.wheel+Vec3{0,1.05f,0},
            {-.45f,2.95f,0},{1.6f,2.5f,0},{4.0f,2.7f,.78f},p.relayBall+Vec3{0,.65f,0},
            {BALL_END.x,.85f,3.7f},PENDULUM_PIVOT+Vec3{0,.35f,0},ROCKER_PIVOT+Vec3{0,.4f,0},
            RELEASE_PIN+Vec3{0,.9f+p.releasePin,0},{6.93f,2.1f,1.1f},{6.7f,1.8f,.1f},
            {7.65f,3.95f,0},{9,1.8f,4},{10.8f,1.6f,1.25f},p.carPosition+Vec3{0,2,0}};
        for(int i=0;i<=ACTION_COUNT;++i)h.badge(vp,points[i],i+1,stage==i);
    }
    if(a.controls){
        h.rect(24,56,1220,580,{.23f,.31f,.34f});h.rect(26,58,1216,576,panel);
        h.text(42,77,"DEMO GUIDE / "+std::to_string(a.guidePage+1)+" OF 6",2.6f,white);
        auto bigButton=[&](UiRect r,const std::string& title,bool selected=false){h.button(r,title,r.contains(a.uiX,a.uiY),selected,2.0f);};
        bigButton(closeGuideButton,"CLOSE [C]");
        h.text(42,113,"CHOOSE A PAGE / C OR ESC CLOSES / CAMERA ALWAYS AVAILABLE",2.0f,muted);
        const char* pages[]={"CAMERA","OPERATE","DRIVE","LIGHT","CHAIN 1","CHAIN 2"};
        for(int i=0;i<GuidePageCount;++i)bigButton(guideTab(i),pages[i],a.guidePage==i);
        auto row=[&](float y,const char* key,const char* description){h.text(42,y,key,2.35f,accent);h.text(325,y,description,2.35f,white);};
        if(a.guidePage==CameraPage){
            row(212,"W / S","FORWARD / BACKWARD");row(253,"A / D","STRAFE LEFT / RIGHT");
            row(294,"Q / E","DOWN / UP");row(335,"RIGHT MOUSE","HOLD TO LOOK / RELEASE FOR BUTTONS");
            row(376,"SHIFT / SCROLL","MOVE FASTER / MOVE CLOSER");
            row(417,"1 / 2 / 3","OVERVIEW / CRANK / GLOVE");
            row(458,"4 / 5 / 6","CAR / FRONT ROW / RELAY CONTACTS");
            h.text(42,510,"CLOSE GUIDE BEFORE MOUSE LOOK. FREE MOVEMENT STAYS MANUAL.",2.15f,muted);
        }else if(a.guidePage==OperatePage){
            h.text(42,212,stageName(stage),2.6f,white);
            h.text(42,254,nextAction(a),2.15f,accent);
            const bool manual=stage<ACTION_COUNT&&stageControl(stage).operation!=Operation::Release;
            h.text(42,296,manual?"KEY GUIDE BUTTON SHOWS THIS PAGE. IT DOES NOT MOVE THE PART.":"F OR THE F GO BUTTON RELEASES THE READY MECHANISM.",2.05f,white);
            h.text(42,338,"G GRIP / Z QUARTER TURN / X CAM / V WIPER / P CRANK",2.25f,white);
            h.text(42,380,"HOLD G, X, V OR P. RELEASE THE KEY TO STOP MID-STROKE.",2.2f,white);
            h.text(42,422,"Z NEEDS FOUR TAPS. THE LATER RELAY USES ONLY F.",2.2f,white);
            h.text(42,464,"SPACE PAUSE / RESUME    I CHANGE MODE    R RESET",2.2f,white);
            h.text(42,506,"ONLY THE READY PART MOVES. EACH HANDOFF CHECKS CONTACT.",2.15f,muted);
        }else if(a.guidePage==DrivePage){
            row(212,"UP / DOWN","ACCELERATE / BRAKE AND REVERSE");
            row(253,"LEFT / RIGHT","STEER THE FRONT WHEELS");row(294,"4","SHOW CAR AND DRIVING APRON");
            row(335,"SPACE","PAUSE / RESUME CAR AND MACHINE");row(376,"R","RESET CAR AND ALL MECHANISMS");
            h.text(42,429,"CAR UNLOCKS AT 70 ACTIVE SECONDS, AFTER THE SHOE CLEARS.",2.2f,white);
            h.text(42,470,"DRIVE STRAIGHT OVER THE BRIDGE BEFORE TURNING ON THE APRON.",2.1f,white);
            h.text(42,511,"WASD + QE STILL MOVE THE CAMERA WHILE YOU DRIVE.",2.2f,muted);
        }else if(a.guidePage==LightingPage){
            h.text(42,208,"L COMPARES PHONG / GOURAUD. CLICK THE SCENE LAMP OR B.",2.2f,white);
            bigButton(shadowButton,a.shadows?"SHADOWS ON":"SHADOWS OFF");
            const char* names[]={"PHONG / FRAGMENT","FLAT / FACE","GOURAUD / VERTEX"};bigButton(shadeButton,names[a.shading]);
            h.text(42,303,"STUDIO / NIGHT: LAMP     DAY / SUNSET: LAMP + SUN",2.2f,white);
            bigButton(dimButton,"-");bigButton(brightButton,"+");h.text(117,351,std::to_string(int(std::round(a.brightness*100)))+"%",2.3f,accent);
            h.text(380,351,"EXPOSURE",2.2f,white);
            h.text(42,399,"T TRACES THE CURRENT CAMERA AND POSE. IT TAKES A FEW SECONDS.",2.1f,white);
            bigButton(rayButton,"T / RAY-TRACED SNAPSHOT");
            h.text(42,498,"SNAPSHOT: T, ESC OR LEFT CLICK RETURNS TO ANIMATION.",2.2f,muted);
            h.text(42,531,"1: FIND LAMP / CLOSE GUIDE TO CLICK / WORKS IN BOTH MODES",2.0f,accent);
        }else{
            int first=a.guidePage==ChainFirstPage?0:9,last=std::min(first+9,ACTION_COUNT);
            h.text(42,210,"ACTION",2.15f,muted);h.text(970,210,"SECONDS",2.15f,muted);h.text(1150,210,"KEY",2.15f,muted);
            for(int i=first;i<last;++i){float y=248+(i-first)*33;
                std::string label=std::to_string(i+1)+" / "+stageName(i);
                h.text(42,y,label,2.2f,i==stage?accent:white);
                h.text(970,y,fixed(STAGE_TIME[i],0)+"-"+fixed(STAGE_TIME[i+1],0),2.2f,muted);
                h.text(1168,y,std::string(1,char(stageControl(i).key)),2.2f,accent);
            }
            if(first==9)h.text(42,532,"CAR RELEASE: 70 SEC / I SELECTS INTERACTIVE MODE",2.0f,accent);
        }
        h.rect(40,558,1188,1,{.25f,.34f,.38f});
        h.text(42,575,"NOW: "+std::string(stageName(stage)),2.0f,white);
        h.text(42,607,nextAction(a),1.9f,accent);
    }
    float y=h.height-65;h.rect(-h.left,y,h.canvasWidth,65,{.035f,.045f,.05f});
    h.rect(22,y-3,1236,2,{.16f,.19f,.2f});h.rect(22,y-3,1236*float(std::min(a.simulation.time,double(CAR_START)))/CAR_START,2,accent);
    h.text(22,y+9,std::to_string(stage+1)+" / "+stageName(stage),1.65f,white);
    h.text(860,y+9,fixed(std::min(a.simulation.time,double(CAR_START)))+" / 70 SEC   "+(stage==ACTION_COUNT?"RELEASED":"CAR LOCKED"),1.3f,accent);
    std::string prompt;
    if(a.simulation.interactive&&stage<ACTION_COUNT)prompt=(a.simulation.waiting?"READY: ":!a.simulation.running?"PAUSED / SPACE: ":"OPERATE: ")+nextAction(a);
    else prompt=!a.simulation.running?"PAUSED: SPACE TO RESUME":nextAction(a);
    h.text(22,y+29,prompt,1.35f,accent);
    const char* shadeNames[]={"PHONG","FLAT","GOURAUD"};
    h.text(1090,y+29,std::string("L: ")+shadeNames[a.shading],1.3f,muted);
    h.text(22,y+49,(stage>=Ball&&stage<=Drum)?"VIEW TIP: PRESS 6 FOR CUP / STRIKER / ROCKER / PIN CONTACTS. CAMERA STAYS MANUAL.":"WASD + QE CAMERA   RIGHT MOUSE LOOK   1 OVERVIEW   SPACE PAUSE   R RESET   C GUIDE",1.4f,muted);
    h.flush();
}
#include "validation.h"

struct CaptureSample {double time;int camera;std::string name;int guide=-1,shading=0,lighting=0;bool interactive=false;};
static std::vector<CaptureSample> reviewSamples(){
    std::vector<CaptureSample> samples{{0,1,"overview"}};
    for(int mode=0;mode<4;++mode)samples.push_back({0,1,"lamp_mode_"+std::to_string(mode),-1,0,mode});
    for(int mode=0;mode<4;++mode)for(int shading=0;shading<3;++shading)
        samples.push_back({60.4,1,"lighting_"+std::to_string(mode)+"_shading_"+std::to_string(shading),-1,shading,mode});
    // Review each boundary from both sides; sample times follow STAGE_TIME.
    const int cameras[]={1,3,3,3,1,1,2,2,6,6,6,6,6,2,2,4,4,4};
    for(int next=1;next<=ACTION_COUNT;++next){
        std::string stem="handoff_"+(next<10?std::string("0"):std::string())+std::to_string(next);
        double boundary=STAGE_TIME[next];
        samples.push_back({boundary-std::min(.5,stageDuration(next-1)*.2),cameras[next],stem+"_before"});
        samples.push_back({boundary,cameras[next],stem+"_at"});
        samples.push_back({boundary+std::min(.65,next<ACTION_COUNT?stageDuration(next)*.2:double(CAR_EXIT_DURATION)*.2),cameras[next],stem+"_after"});
    }
    for(int stage:{Cup,Pendulum,Rocker,Pin,Drum})samples.push_back({STAGE_TIME[stage],1,"overview_"+std::to_string(stage)});
    samples.push_back({DEMO_END,4,"car_departed"});
    for(int mode=0;mode<3;++mode)samples.push_back({STAGE_TIME[Pendulum]+stageDuration(Pendulum)*.5,6,"shading_"+std::to_string(mode),-1,mode});
    for(int page=0;page<GuidePageCount;++page)samples.push_back({STAGE_TIME[Cup],1,"guide_"+std::to_string(page),page,0,0,true});
    samples.push_back({0,1,"guide_hold_key",OperatePage,0,0,true});
    // Closely spaced views catch a visible dwell between cup withdrawal and swing.
    const float releaseOffsets[]={-.10f,0,.10f,.30f,.60f,1.f};
    for(int i=0;i<6;++i)samples.push_back({pendulumReleaseTime()+releaseOffsets[i],6,"striker_release_"+std::to_string(i)});
    return samples;
}

int main(int argc,char** argv){
    bool capture=false,verify=false,timing=false,rayCapture=false,verifyInteractive=false,lightingOnly=false;std::string captureDir="docs/final-preview";
    try{
        if(argc>1&&std::string(argv[1])=="--self-test")return selfTest();
        if(argc>1&&std::string(argv[1])=="--capture"){capture=true;if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
        if(argc>1&&std::string(argv[1])=="--capture-ray"){capture=rayCapture=true;captureDir="docs/ray-preview";if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
        if(argc>1&&std::string(argv[1])=="--verify-playback")verify=true;
        if(argc>1&&std::string(argv[1])=="--verify-lighting")verify=lightingOnly=true;
        if(argc>1&&std::string(argv[1])=="--verify-interactive"){verify=true;verifyInteractive=true;}
        if(argc>1&&std::string(argv[1])=="--verify-timing"){verify=true;timing=true;}
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
    glfwSetErrorCallback([](int code,const char* text){std::cerr<<"GLFW "<<code<<": "<<text<<'\n';});
    if(!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,4);if(capture||verify)glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    GLFWwindow* window=glfwCreateWindow(1440,900,"COG | Interactive Automotive Chain Reaction | 2107111",nullptr,nullptr);
    if(!window){glfwTerminate();return 1;}glfwSetWindowSizeLimits(window,960,600,GLFW_DONT_CARE,GLFW_DONT_CARE);glfwMakeContextCurrent(window);glfwSwapInterval(capture||verify?0:1);
    int result=0;
    try{
        if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))throw std::runtime_error("GLAD could not load OpenGL");
        std::cout<<"OpenGL: "<<glGetString(GL_VERSION)<<"\nRenderer: "<<glGetString(GL_RENDERER)<<std::endl;glEnable(GL_MULTISAMPLE);
        App state;state.simulation.running=!capture;
        glfwSetWindowUserPointer(window,&state);glfwSetKeyCallback(window,keyCallback);glfwSetMouseButtonCallback(window,mouseButton);
        glfwSetCursorPosCallback(window,cursor);glfwSetScrollCallback(window,scroll);glfwSetWindowFocusCallback(window,focus);
        Renderer renderer;Hud hud;Scene scene;RaySnapshot ray;
        state.renderer=&renderer;state.scene=&scene;
        scene.build(poseAt(0),0);
        if(verify){verifyLighting(renderer);if(lightingOnly)glfwSetWindowShouldClose(window,GL_TRUE);else verifyHandlers(window);}
        if(verifyInteractive)keyCallback(window,GLFW_KEY_I,0,GLFW_PRESS,0);
        int inputStage=-1,holdFrames=0;bool testedHold=false;
        const auto samples=reviewSamples();
        if(capture){std::ofstream manifest(captureDir+"/frames.csv");manifest<<"name,active_seconds,camera,guide,shading\n";
            for(const auto& sample:samples)manifest<<sample.name<<','<<sample.time<<','<<sample.camera<<','<<sample.guide<<','<<sample.shading<<'\n';}
        double previous=glfwGetTime(),started=previous;int frame=0;bool released=false;
        while(!glfwWindowShouldClose(window)){
            // Sample time before event handling so a reset/play event cannot
            // accidentally consume time accumulated before that event.
            double now=glfwGetTime(),dt=now-previous;previous=now;double cameraDt=dt;
            double oldTime=state.simulation.time;bool wasRunning=state.simulation.running;
            glfwPollEvents();
            int width,height;glfwGetFramebufferSize(window,&width,&height);
            if(width==0||height==0){glfwWaitEventsTimeout(.05);continue;}
            if(capture){const auto& sample=samples[frame];
                state.simulation.time=sample.time;state.camera.preset(sample.camera);
                state.shading=sample.shading;state.controls=sample.guide>=0;state.guidePage=std::max(0,sample.guide);state.labels=false;
                state.lighting=sample.lighting;state.simulation.interactive=sample.interactive;state.simulation.waiting=sample.interactive;
                if(rayCapture){state.simulation.time=CAR_START;state.camera.preset(2);state.rayRequested=true;}
            }else{
                if(state.simulation.time<oldTime||!wasRunning)dt=0;
                if(verify&&!timing)dt=.2;
                if(verifyInteractive&&stageAt(state.simulation.time)<ACTION_COUNT){
                    int s=stageAt(state.simulation.time),key=stageControl(s).key;
                    if(inputStage!=s){state.held.fill(false);inputStage=s;testedHold=false;holdFrames=0;
                        std::cout<<"INTERACTIVE "<<s+1<<" / "<<stageName(s)<<std::endl;}
                    if(state.simulation.waiting){keyCallback(window,key,0,GLFW_PRESS,0);
                        if(stageControl(s).operation!=Operation::Hold)keyCallback(window,key,0,GLFW_RELEASE,0);}
                    double u=(state.simulation.time-STAGE_TIME[s])/(STAGE_TIME[s+1]-STAGE_TIME[s]);
                    if(!testedHold&&stageControl(s).operation==Operation::Hold&&u>.4){
                        keyCallback(window,key,0,GLFW_RELEASE,0);holdFrames=8;testedHold=true;}
                    if(holdFrames>0&&--holdFrames==0)keyCallback(window,key,0,GLFW_PRESS,0);
                }
                state.simulation.update(dt,state.mechanismHeld());
                if(verify&&state.simulation.blockedStage>=0)throw std::runtime_error(handoffFailure(state.simulation.blockedStage));
                auto down=[&](int key){return !verify&&!state.rayActive&&glfwGetKey(window,key)==GLFW_PRESS?1.f:0.f;};
                state.camera.move(down(GLFW_KEY_W)-down(GLFW_KEY_S),down(GLFW_KEY_D)-down(GLFW_KEY_A),
                    down(GLFW_KEY_E)-down(GLFW_KEY_Q),float(std::min(cameraDt,.15)),down(GLFW_KEY_LEFT_SHIFT)>0);
                double carDt=oldTime<CAR_START?std::max(0.0,state.simulation.time-CAR_START):dt;
                state.car.update(carDt,state.simulation,verifyInteractive?1.f:down(GLFW_KEY_UP)-down(GLFW_KEY_DOWN),down(GLFW_KEY_LEFT)-down(GLFW_KEY_RIGHT));
            }
            double mx,my;int ww,wh;glfwGetCursorPos(window,&mx,&my);glfwGetWindowSize(window,&ww,&wh);
            if(ww>0&&wh>0){UiLayout layout(ww,wh);state.uiX=float(mx)/layout.factor-layout.left;state.uiY=float(my)/layout.factor;}
            Pose pose=poseAt(float(state.simulation.time));if(!capture)state.car.apply(pose);scene.build(pose,float(state.simulation.time),state.lighting);
            Vec3 eye=state.camera.eye;Mat4 view=lookAt(eye,eye+state.camera.forward(),{0,1,0});
            Mat4 projection=cameraProjection(float(width)/height);
            if(state.rayRequested){
                state.rayRequested=false;state.resumeAfterRay=state.simulation.running;state.simulation.running=false;state.rayActive=true;
                renderer.draw(scene.items,view,projection,eye,width,height,state.shading,state.lighting,state.brightness,state.shadows);
                hud.begin(width,height);hud.rect(320,120,640,60,{.035f,.045f,.05f});
                hud.text(345,142,"TRACING SNAPSHOT / PLEASE WAIT",2,{.83f,.86f,.86f});hud.flush();glfwSwapBuffers(window);
                ray.render(renderer,scene.items,eye,state.camera.forward(),float(width)/height,state.lighting,state.brightness,state.shadows);
                previous=glfwGetTime();
                std::cout<<"Ray trace: "<<ray.triangleCount<<" triangles, "<<ray.width<<" x "<<ray.height<<", "<<fixed(ray.milliseconds)<<" ms"<<std::endl;
            }
            if(state.rayActive)ray.draw(width,height);
            else renderer.draw(scene.items,view,projection,eye,width,height,state.shading,state.lighting,state.brightness,state.shadows);
            overlay(hud,state,width,height,projection*view,pose);
            GLenum error=glGetError();if(error!=GL_NO_ERROR)throw std::runtime_error("OpenGL error: "+std::to_string(error));
            if(capture){std::string name=captureDir+"/"+(rayCapture?std::string("ray_snapshot"):samples[frame].name)+".bmp";captureBMP(name,width,height);std::cout<<"Captured "<<name<<" t="<<state.simulation.time<<std::endl;}
            glfwSwapBuffers(window);++frame;
            if(capture&&frame==(rayCapture?1:int(samples.size())))glfwSetWindowShouldClose(window,GL_TRUE);
            if(verify){
                if(state.simulation.time<CAR_START&&pose.carTravel!=0)throw std::runtime_error("Early car motion");
                if(!released&&state.simulation.time>=CAR_START){released=true;
                    std::cout<<"RELEASE: active="<<fixed(state.simulation.time,4)<<" s, wall="<<fixed(now-started,4)<<" s, frame="<<frame<<std::endl;}
                if(state.simulation.time>=DEMO_END){
                    if(verifyInteractive?pose.carTravel<6:std::abs(pose.carTravel-9)>.001f)throw std::runtime_error("Car exit failed");
                    std::cout<<"PASS: full 17-action / 70-second chain + 7-second car exit; "<<frame<<" GPU frames; mode, stage, camera, pause/reset, guide and shading handlers"<<std::endl;
                    glfwSetWindowShouldClose(window,GL_TRUE);
                }
            }
        }
    }catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';result=1;}
    glfwDestroyWindow(window);glfwTerminate();return result;
}
