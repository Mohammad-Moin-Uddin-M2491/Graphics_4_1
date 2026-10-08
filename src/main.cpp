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
const UiRect lightButton{842,8,260,30},toolsButton{1113,8,145,30};
const UiRect shadowButton{42,118,166,30},shadeButton{220,118,286,30},dimButton{660,118,38,30},brightButton{808,118,38,30};
const UiRect rayButton{862,118,365,30},closeGuideButton{1086,68,141,32};
const UiRect controlsTab{42,163,190,30},sequenceTab{244,163,305,30};
struct App {
    Simulation simulation;Camera camera;CarState car;
    bool looking=false,labels=false,controls=false;
    int guidePage=0;std::array<bool,GLFW_KEY_LAST+1> held{};
    double lastX=0,lastY=0;int shading=0,lighting=0;
    float brightness=1,uiX=-1,uiY=-1;bool shadows=true;
    bool rayRequested=false,rayActive=false,resumeAfterRay=false;
    void closeRay(){rayActive=false;simulation.running=resumeAfterRay;}
    void reset(){simulation.reset();car.reset();held.fill(false);}
    int mechanismHeld()const{int s=stageAt(simulation.time);if(s>=ACTION_COUNT)return 0;
        int key=stageControl(s).key;return held[key]?key:0;}
    void toggleMode(){simulation.interactive=!simulation.interactive;reset();if(!simulation.interactive)simulation.running=true;}
    void togglePlay(){
        if(simulation.waiting){simulation.activate();return;}
        simulation.running=!simulation.running;
    }
    bool clickUi(float x,float y){
        if(rayActive){closeRay();return true;}
        if(playButton.contains(x,y))togglePlay();
        else if(resetButton.contains(x,y))reset();
        else if(modeButton.contains(x,y))toggleMode();
        else if(lightButton.contains(x,y))lighting=(lighting+1)%4;
        else if(toolsButton.contains(x,y))controls=!controls;
        else if(controls&&closeGuideButton.contains(x,y))controls=false;
        else if(controls&&controlsTab.contains(x,y))guidePage=0;
        else if(controls&&sequenceTab.contains(x,y))guidePage=1;
        else if(controls&&dimButton.contains(x,y))brightness=std::max(.4f,brightness-.2f);
        else if(controls&&brightButton.contains(x,y))brightness=std::min(2.0f,brightness+.2f);
        else if(controls&&shadowButton.contains(x,y))shadows=!shadows;
        else if(controls&&shadeButton.contains(x,y))shading=(shading+1)%3;
        else if(controls&&rayButton.contains(x,y))rayRequested=true;
        else return controls;
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
    case GLFW_KEY_B:a.lighting=(a.lighting+1)%4;break;
    case GLFW_KEY_H:a.labels=!a.labels;break;
    case GLFW_KEY_C:a.controls=!a.controls;releaseMouse(w);break;
    case GLFW_KEY_T:a.rayRequested=true;releaseMouse(w);break;
    default:a.simulation.operate(key);break;}
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
    int width,height;glfwGetWindowSize(w,&width,&height);
    if(width>0&&height>0){UiLayout layout(width,height);a.clickUi(float(x)/layout.factor-layout.left,float(y)/layout.factor);}
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
    button(playButton,a.simulation.waiting?"NEXT [F]":a.simulation.running?"PAUSE":"PLAY");
    button(resetButton,"RESET");button(lightButton,std::string("LIGHT: ")+lightingPreset(a.lighting).name+" [B]");
    button(toolsButton,"GUIDE [C]",a.controls);
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
        h.text(42,76,"CONTROLS AND DEMO GUIDE",2.3f,white);button(closeGuideButton,"X CLOSE [C]");
        h.text(42,101,"BOTH MODES UNLESS MARKED / CAMERA WORKS WHILE PAUSED / LIVE STATUS BELOW",1.35f,muted);
        button(shadowButton,a.shadows?"SHADOWS: ON":"SHADOWS: OFF");
        const char* modes[]={"PHONG / FRAGMENT","FLAT / FACE","GOURAUD / VERTEX"};
        button(shadeButton,std::string("SHADE: ")+modes[a.shading]);button(dimButton,"-");button(brightButton,"+");
        h.text(530,128,"EXPOSURE",1.35f,muted);h.text(715,128,std::to_string(int(std::round(a.brightness*100)))+"%",1.45f,white);
        button(rayButton,"T / RAY-TRACED SNAPSHOT");
        button(controlsTab,"GENERAL CONTROLS",a.guidePage==0);button(sequenceTab,"MECHANISMS AND SEQUENCE",a.guidePage==1);
        auto row=[&](float x,float y,const char* key,const char* text){h.text(x,y,key,1.55f,accent);h.text(x+156,y,text,1.5f,white);};
        if(a.guidePage==0){
            h.text(42,211,"FREE CAMERA / BOTH MODES",1.7f,accent);
            row(42,240,"W / S","MOVE FORWARD / BACKWARD");
            row(42,262,"A / D","STRAFE LEFT / RIGHT");
            row(42,284,"Q / E","MOVE DOWN / UP");
            row(42,306,"LEFT SHIFT","HOLD FOR FASTER CAMERA");
            row(42,328,"RIGHT MOUSE","HOLD TO LOOK / RELEASE FOR UI");
            row(42,350,"MOUSE WHEEL","MOVE CLOSER / FARTHER");
            row(42,372,"1 / 2 / 3","WHOLE CHAIN / PUMP / GLOVE");
            row(42,394,"4 / 5","CAR AND APRON / NEW FRONT ROW");
            row(42,416,"6","CUP / STRIKER / ROCKER / PIN");
            row(42,438,"ESC","CLOSE GUIDE / RELEASE MOUSE");
            h.text(42,463,"CAR / BOTH MODES / UNLOCKS AT 70 SEC",1.6f,accent);
            row(42,491,"UP / DOWN","ACCELERATE / BRAKE AND REVERSE");
            row(42,515,"LEFT / RIGHT","STEER FRONT WHEELS");
            h.text(42,540,"ARROWS TAKE OVER AUTOMATIC EXIT. SPACE PAUSES CAR.",1.3f,muted);
            h.text(654,211,"PLAYBACK AND DISPLAY / BOTH MODES",1.7f,accent);
            row(654,240,"I / MODE","SWITCH MODE AND RESET ALL");
            row(654,264,"R / RESET","RESET CAR AND EVERY MECHANISM");
            row(654,288,"SPACE / PLAY","PAUSE / RESUME ACTIVE ACTION");
            row(654,312,"F / NEXT","RELEASE READY PASSIVE ACTION");
            row(654,336,"C / GUIDE","OPEN OR CLOSE THIS GUIDE");
            row(654,360,"H","TOGGLE NUMBERED PART LABELS");
            row(654,384,"L","SWITCH REAL PHONG / GOURAUD");
            row(654,408,"B / LIGHT","STUDIO / DAY / SUNSET / NIGHT");
            row(654,432,"T / SNAPSHOT","TRACE CURRENT CAMERA AND POSE");
            row(654,456,"LEFT CLICK","OPERATE BUTTONS AND TABS");
            row(654,480,"WINDOW X","CLOSE APP / ALT+F4 ALSO WORKS");
            h.text(654,510,"TOOLS: SHADOWS / SHADERS / +/- EXPOSURE 40-200%",1.3f,muted);
            h.text(654,530,"SNAPSHOT: T, ESC OR LEFT CLICK RETURNS TO OPENGL",1.3f,muted);
        }else{
            h.text(42,211,"INTERACTIVE ONLY / EACH KEY OPERATES ITS OWN PART",1.55f,accent);
            const int stages[]={0,1,2,4,6,8,9,10,11};
            const char* keys[]={"HOLD G","TAP Z X4","HOLD X","HOLD V","HOLD P","HOLD U","PRESS J","HOLD K","HOLD O"};
            const char* actions[]={"GRIP AND CARRY STARTER PIN","WIND GEAR IN QUARTER TURNS","ROTATE CAM / LOWER STOP","SWEEP WIPER INTO BAR","COMPRESS CRANK / LIFT STOP","TIP CUP / REMOVE STRIKER STOP","RELEASE PENDULUM STRIKER","ROCK ARM INTO PIN COLLAR","LIFT PIN OUT OF CABLE DRUM"};
            for(int i=0;i<9;++i){row(42,242+i*27,keys[i],actions[i]);
                if(stage==stages[i])h.rect(31,240+i*27,4,16,accent);}
            h.text(42,493,"RELEASE A HELD KEY TO STOP ITS PART MID-STROKE.",1.4f,white);
            h.text(42,515,"F / NEXT / SPACE RELEASES OTHER READY ACTIONS.",1.4f,white);
            h.text(42,537,"FUTURE PARTS STAY LOCKED. I OR R RESETS THE ORDER.",1.35f,muted);
            h.text(654,211,"70 SEC / 17 ACTIONS / 18TH ACTION IS CAR EXIT",1.55f,accent);
            for(int i=0;i<ACTION_COUNT;++i){
                std::string number=(i<9?"0":"")+std::to_string(i+1);
                std::string title=stageName(i);title=title.substr(0,title.find(" /"));
                std::string line=number+"  "+title;
                h.text(654,240+i*17.5f,line,1.4f,i==stage?accent:white);
                h.text(968,240+i*17.5f,fixed(STAGE_TIME[i],0)+"-"+fixed(STAGE_TIME[i+1],0)+" SEC",1.4f,muted);
                h.text(1136,240+i*17.5f,std::string(1,char(stageControl(i).key)),1.4f,accent);
            }
        }
        h.rect(40,561,1188,1,{.25f,.34f,.38f});
        h.text(42,574,"NOW: "+std::string(stageName(stage)),1.65f,white);
        h.text(42,599,"ACTION: "+nextAction(a),1.5f,accent);
        h.text(840,578,a.simulation.interactive?"INTERACTIVE / IN ORDER":"PRESENTATION / AUTOMATIC",1.4f,muted);
        h.text(840,601,"LOOK / WHEEL: CLOSE GUIDE FIRST",1.3f,muted);
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
    h.text(22,y+49,"WASD + QE CAMERA   RIGHT MOUSE LOOK   1 WHOLE CHAIN   5 ROW / 6 CONTACTS   SPACE PAUSE   R RESET   C GUIDE",1.16f,muted);
    h.flush();
}
#include "validation.h"

int main(int argc,char** argv){
    bool capture=false,verify=false,timing=false,rayCapture=false,verifyInteractive=false;std::string captureDir="docs/final-preview";
    try{
        if(argc>1&&std::string(argv[1])=="--self-test")return selfTest();
        if(argc>1&&std::string(argv[1])=="--capture"){capture=true;if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
        if(argc>1&&std::string(argv[1])=="--capture-ray"){capture=rayCapture=true;captureDir="docs/ray-preview";if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
        if(argc>1&&std::string(argv[1])=="--verify-playback")verify=true;
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
        if(verify)verifyHandlers(window);
        if(verifyInteractive)keyCallback(window,GLFW_KEY_I,0,GLFW_PRESS,0);
        int inputStage=-1,holdFrames=0;bool testedHold=false;
        const float sampleTimes[]={0,3,8,12,16,19.5f,22.5f,26.5f,29,31.5f,34,36,38,40,42,44,46,48,50,51.5f,54.5f,58,62.5f,66.5f,69,70,73.5f,77,0,40,40,40,40,40,40,0,38};
        const int sampleCameras[]={1,3,3,3,1,1,2,2,5,5,6,6,6,6,6,6,6,6,6,2,2,2,4,4,4,1,4,4,1,6,6,6,6,6,6,1,1};
        constexpr int SAMPLE_COUNT=sizeof(sampleTimes)/sizeof(sampleTimes[0]);
        double previous=glfwGetTime(),started=previous;int frame=0;bool released=false;
        while(!glfwWindowShouldClose(window)){
            // Sample time before event handling so a reset/play event cannot
            // accidentally consume time accumulated before that event.
            double now=glfwGetTime(),dt=now-previous;previous=now;double cameraDt=dt;
            double oldTime=state.simulation.time;bool wasRunning=state.simulation.running;
            glfwPollEvents();
            int width,height;glfwGetFramebufferSize(window,&width,&height);
            if(width==0||height==0){glfwWaitEventsTimeout(.05);continue;}
            if(capture){state.simulation.time=sampleTimes[frame];state.camera.preset(sampleCameras[frame]);
                state.shading=frame==30?2:frame==31?1:0;state.controls=frame>=35;state.guidePage=frame==36?1:0;state.labels=frame==28;
                state.lighting=frame==32?1:frame==33?2:frame==34?3:0;
                state.simulation.interactive=frame==36;state.simulation.waiting=frame==36;
                if(rayCapture){state.simulation.time=70;state.camera.preset(2);state.rayRequested=true;}
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
                auto down=[&](int key){return !verify&&!state.rayActive&&glfwGetKey(window,key)==GLFW_PRESS?1.f:0.f;};
                state.camera.move(down(GLFW_KEY_W)-down(GLFW_KEY_S),down(GLFW_KEY_D)-down(GLFW_KEY_A),
                    down(GLFW_KEY_E)-down(GLFW_KEY_Q),float(std::min(cameraDt,.15)),down(GLFW_KEY_LEFT_SHIFT)>0);
                double carDt=oldTime<CAR_START?std::max(0.0,state.simulation.time-CAR_START):dt;
                state.car.update(carDt,state.simulation,verifyInteractive?1.f:down(GLFW_KEY_UP)-down(GLFW_KEY_DOWN),down(GLFW_KEY_LEFT)-down(GLFW_KEY_RIGHT));
            }
            double mx,my;int ww,wh;glfwGetCursorPos(window,&mx,&my);glfwGetWindowSize(window,&ww,&wh);
            if(ww>0&&wh>0){UiLayout layout(ww,wh);state.uiX=float(mx)/layout.factor-layout.left;state.uiY=float(my)/layout.factor;}
            Pose pose=poseAt(float(state.simulation.time));if(!capture)state.car.apply(pose);scene.build(pose,float(state.simulation.time));
            Vec3 eye=state.camera.eye;Mat4 view=lookAt(eye,eye+state.camera.forward(),{0,1,0});
            Mat4 projection=perspective(radians(43),float(width)/height,.05f,220);
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
            if(capture){std::string name=captureDir+"/frame_"+(frame<10?"0":"")+std::to_string(frame)+".bmp";captureBMP(name,width,height);std::cout<<"Captured "<<name<<" t="<<state.simulation.time<<std::endl;}
            glfwSwapBuffers(window);++frame;
            if(capture&&frame==(rayCapture?1:SAMPLE_COUNT))glfwSetWindowShouldClose(window,GL_TRUE);
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
