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
const UiRect shadowButton{18,54,170,30},shadeButton{200,54,305,30},dimButton{728,54,38,30},brightButton{896,54,38,30};
const UiRect rayButton{960,54,298,30};
struct App {
    Simulation simulation;Camera camera;CarState car;
    bool looking=false,labels=false,controls=false;
    double lastX=0,lastY=0;int shading=0,lighting=0;
    float brightness=1,uiX=-1,uiY=-1;bool shadows=true;
    bool rayRequested=false,rayActive=false,resumeAfterRay=false;
    void closeRay(){rayActive=false;simulation.running=resumeAfterRay;}
    void reset(){simulation.reset();car.reset();}
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
        else if(controls&&dimButton.contains(x,y))brightness=std::max(.4f,brightness-.2f);
        else if(controls&&brightButton.contains(x,y))brightness=std::min(2.0f,brightness+.2f);
        else if(controls&&shadowButton.contains(x,y))shadows=!shadows;
        else if(controls&&shadeButton.contains(x,y))shading=(shading+1)%3;
        else if(controls&&rayButton.contains(x,y))rayRequested=true;
        else return false;
        return true;
    }
};
static App& app(GLFWwindow* w){return *static_cast<App*>(glfwGetWindowUserPointer(w));}
static void releaseMouse(GLFWwindow* w){app(w).looking=false;glfwSetInputMode(w,GLFW_CURSOR,GLFW_CURSOR_NORMAL);}
static void keyCallback(GLFWwindow* w,int key,int,int action,int){
    if(action!=GLFW_PRESS)return;
    auto& a=app(w);
    if(a.rayActive){if(key==GLFW_KEY_T||key==GLFW_KEY_ESCAPE)a.closeRay();return;}
    switch(key){
    case GLFW_KEY_ESCAPE:releaseMouse(w);break;
    case GLFW_KEY_SPACE:a.togglePlay();break;
    case GLFW_KEY_R:a.reset();break;
    case GLFW_KEY_I:a.toggleMode();break;
    case GLFW_KEY_F:a.simulation.activate();break;
    case GLFW_KEY_5:case GLFW_KEY_6:case GLFW_KEY_7:case GLFW_KEY_8:case GLFW_KEY_9:a.simulation.activate(key-GLFW_KEY_5);break;
    case GLFW_KEY_1:case GLFW_KEY_2:case GLFW_KEY_3:case GLFW_KEY_4:a.camera.preset(key-GLFW_KEY_0);break;
    case GLFW_KEY_L:a.shading=a.shading==2?0:2;break;
    case GLFW_KEY_B:a.lighting=(a.lighting+1)%4;break;
    case GLFW_KEY_H:a.labels=!a.labels;break;
    case GLFW_KEY_C:a.controls=!a.controls;releaseMouse(w);break;
    case GLFW_KEY_T:a.rayRequested=true;releaseMouse(w);break;
    default:break;}
}
static void mouseButton(GLFWwindow* w,int button,int action,int){
    auto& a=app(w);double x,y;glfwGetCursorPos(w,&x,&y);
    if(a.rayActive){if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS)a.closeRay();return;}
    if(button==GLFW_MOUSE_BUTTON_RIGHT){
        if(action==GLFW_PRESS){a.looking=true;a.lastX=x;a.lastY=y;glfwSetInputMode(w,GLFW_CURSOR,GLFW_CURSOR_DISABLED);}
        else releaseMouse(w);
        return;
    }
    if(button!=GLFW_MOUSE_BUTTON_LEFT||action!=GLFW_PRESS||a.looking)return;
    int width,height;glfwGetWindowSize(w,&width,&height);
    if(width>0&&height>0)a.clickUi(float(x)*1280/width,float(y)*1280/width);
}
static void cursor(GLFWwindow* w,double x,double y){auto& a=app(w);
    if(a.looking)a.camera.look(float(x-a.lastX),float(y-a.lastY));
    a.lastX=x;a.lastY=y;
}
static void scroll(GLFWwindow* w,double,double y){auto& a=app(w);if(!a.rayActive)a.camera.eye=a.camera.eye+a.camera.forward()*float(y)*.75f;}
static void focus(GLFWwindow* w,int focused){if(!focused)releaseMouse(w);}
static std::string fixed(double v,int precision=1){std::ostringstream s;s<<std::fixed<<std::setprecision(precision)<<v;return s.str();}
static void overlay(Hud& h,const App& a,int width,int height,const Mat4& vp,const Pose& p){
    h.begin(width,height);
    const Vec3 white{.83f,.86f,.86f},muted{.48f,.55f,.57f},accent{.72f,.58f,.36f};
    if(a.rayActive){
        h.rect(0,0,1280,44,{.035f,.045f,.05f});h.text(20,16,"CPU RAY-TRACED SNAPSHOT / T OR CLICK TO RETURN",1.6f,white);
        h.rect(0,h.height-44,1280,44,{.035f,.045f,.05f});
        h.text(20,h.height-31,"TRIANGLE INTERSECTIONS / SHADOW RAYS / ONE REFLECTION BOUNCE / 800 PX WIDE",1.4f,accent);
        h.text(20,h.height-13,"FROZEN FRAME / RETURN TO OPENGL FOR CAMERA, DRIVING AND ANIMATION",1.2f,muted);h.flush();return;
    }
    h.rect(0,0,1280,44,{.035f,.045f,.05f});h.text(18,16,"AUTOMOTIVE CHAIN REACTION",1.5f,white);
    auto button=[&](UiRect r,const std::string& text,bool active=false){h.button(r,text,r.contains(a.uiX,a.uiY),active);};
    button(modeButton,a.simulation.interactive?"MODE: INTERACTIVE [I]":"MODE: PRESENTATION [I]");
    button(playButton,a.simulation.waiting?"NEXT [F]":a.simulation.running?"PAUSE":"PLAY");
    button(resetButton,"RESET");button(lightButton,std::string("LIGHT: ")+lightingPreset(a.lighting).name+" [B]");
    button(toolsButton,"HELP / TOOLS",a.controls);
    if(a.controls){
        h.rect(0,44,1280,46,{.045f,.06f,.07f});button(shadowButton,a.shadows?"SHADOWS: ON":"SHADOWS: OFF");
        const char* modes[]={"PHONG (FRAGMENT)","FLAT","GOURAUD (VERTEX)"};
        button(shadeButton,std::string("L: ")+modes[a.shading]);button(dimButton,"-");button(brightButton,"+");
        h.text(780,64,"LIGHT "+std::to_string(int(std::round(a.brightness*100)))+"%",1.3f,white);
        button(rayButton,"T: RAY-TRACED SNAPSHOT");
        h.rect(18,103,700,221,{.035f,.048f,.056f});
        const char* help[]={"CAMERA: WASD MOVE / Q DOWN / E UP / SHIFT FAST",
            "HOLD RIGHT MOUSE TO LOOK / RELEASE OR ESC FOR UI",
            "SCROLL MOVE CLOSER / 1 OVERVIEW / 2-4 CLOSE VIEWS",
            "CAR: UP FORWARD / DOWN REVERSE / LEFT-RIGHT STEER",
            "CAR UNLOCKS AT 70 SEC / CAMERA WORKS WHILE DRIVING",
            "I SWITCH MODE + RESET / F ACTIVATE NEXT MECHANISM",
            "5 PIN / 6 GEARS / 7 LEVER / 8 WHEEL / 9 WIPER",
            "SPACE PAUSE OR RESUME / R RESET / H OBJECT LABELS",
            "L PHONG / GOURAUD COMPARE / B LIGHT / C CLOSE HELP",
            "T RAY-TRACED STILL / T RETURN / MOIN 2107111"};
        for(int i=0;i<10;++i)h.text(31,115+i*20,help[i],1.35f,i==5?accent:muted);
    }
    int stage=stageAt(a.simulation.time);
    if(a.labels){
        const Vec3 points[]={p.hand+Vec3{0,.4f,0},{-9,4.6f,0},{-6.7f,4.2f,0},p.wheel+Vec3{0,1.05f,0},
            {-.45f,2.95f,0},{1.6f,2.5f,0},{4.7f,2.95f,.78f},{6.93f,2.1f,1.1f},
            {6.7f,1.8f,.1f},{7.65f,3.95f,0},{9,1.8f,4},{10.8f,1.6f,1.25f},p.carPosition+Vec3{0,2,0}};
        for(int i=0;i<13;++i)h.badge(vp,points[i],i+1,stage==i);
    }
    float y=h.height-65;h.rect(0,y,1280,65,{.035f,.045f,.05f});
    h.rect(22,y-3,1236,2,{.16f,.19f,.2f});h.rect(22,y-3,1236*float(std::min(a.simulation.time,70.0))/70,2,accent);
    h.text(22,y+9,stageName(stage),1.65f,white);
    h.text(860,y+9,fixed(std::min(a.simulation.time,70.0))+" / 70 SEC   "+(stage==12?"RELEASED":"CAR LOCKED"),1.3f,accent);
    std::string prompt=a.simulation.waiting?"READY: PRESS F OR NEXT TO ACTIVATE THIS MECHANISM":
        !a.simulation.running?"PAUSED: SPACE TO RESUME":stage==12?"ARROWS DRIVE / R RESET FOR ANOTHER RUN":
        a.simulation.interactive?"MECHANISM ACTIVE / NEXT ACTION UNLOCKS WHEN IT FINISHES":"PRESENTATION / I SWITCHES TO HANDS-ON MODE AND RESETS";
    h.text(22,y+29,prompt,1.35f,accent);
    const char* shadeNames[]={"PHONG","FLAT","GOURAUD"};
    h.text(1090,y+29,std::string("L: ")+shadeNames[a.shading],1.3f,muted);
    h.text(22,y+49,"WASD + QE CAMERA   RIGHT MOUSE LOOK   1 OVERVIEW   F NEXT ACTION   SPACE PAUSE   R RESET   C HELP",1.16f,muted);
    h.flush();
}
#include "validation.h"

int main(int argc,char** argv){
    bool capture=false,verify=false,timing=false,rayCapture=false;std::string captureDir="docs/final-preview";
    try{
        if(argc>1&&std::string(argv[1])=="--self-test")return selfTest();
        if(argc>1&&std::string(argv[1])=="--capture"){capture=true;if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
        if(argc>1&&std::string(argv[1])=="--capture-ray"){capture=rayCapture=true;captureDir="docs/ray-preview";if(argc>2)captureDir=argv[2];std::filesystem::create_directories(captureDir);}
        if(argc>1&&std::string(argv[1])=="--verify-playback")verify=true;
        if(argc>1&&std::string(argv[1])=="--verify-timing"){verify=true;timing=true;}
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
    glfwSetErrorCallback([](int code,const char* text){std::cerr<<"GLFW "<<code<<": "<<text<<'\n';});
    if(!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,4);if(capture||verify)glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    GLFWwindow* window=glfwCreateWindow(1440,900,"COG | Interactive Automotive Chain Reaction | 2107111",nullptr,nullptr);
    if(!window){glfwTerminate();return 1;}glfwMakeContextCurrent(window);glfwSwapInterval(capture||verify?0:1);
    int result=0;
    try{
        if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))throw std::runtime_error("GLAD could not load OpenGL");
        std::cout<<"OpenGL: "<<glGetString(GL_VERSION)<<"\nRenderer: "<<glGetString(GL_RENDERER)<<std::endl;glEnable(GL_MULTISAMPLE);
        App state;state.simulation.running=!capture;
        glfwSetWindowUserPointer(window,&state);glfwSetKeyCallback(window,keyCallback);glfwSetMouseButtonCallback(window,mouseButton);
        glfwSetCursorPosCallback(window,cursor);glfwSetScrollCallback(window,scroll);glfwSetWindowFocusCallback(window,focus);
        Renderer renderer;Hud hud;Scene scene;RaySnapshot ray;
        if(verify)verifyHandlers(window);
        const float sampleTimes[]={0,3,12,20,24,30,35,40,49,52,55,59,62.5f,67,70,77,12,12,52,62.5f,77,0};
        const int sampleCameras[]={1,3,3,3,1,1,2,2,2,2,2,2,4,4,1,4,3,3,2,4,4,1};
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
                state.shading=frame==17?2:0;state.controls=frame==21;state.labels=frame==18;
                state.lighting=frame==19?2:frame==20?3:0;
                if(rayCapture){state.simulation.time=70;state.camera.preset(2);state.rayRequested=true;}
            }else{
                if(state.simulation.time<oldTime||!wasRunning)dt=0;
                if(verify&&!timing)dt=.2;
                state.simulation.update(dt);
                auto down=[&](int key){return !verify&&!state.rayActive&&glfwGetKey(window,key)==GLFW_PRESS?1.f:0.f;};
                state.camera.move(down(GLFW_KEY_W)-down(GLFW_KEY_S),down(GLFW_KEY_D)-down(GLFW_KEY_A),
                    down(GLFW_KEY_E)-down(GLFW_KEY_Q),float(std::min(cameraDt,.15)),down(GLFW_KEY_LEFT_SHIFT)>0);
                double carDt=oldTime<CAR_START?std::max(0.0,state.simulation.time-CAR_START):dt;
                state.car.update(carDt,state.simulation,down(GLFW_KEY_UP)-down(GLFW_KEY_DOWN),down(GLFW_KEY_LEFT)-down(GLFW_KEY_RIGHT));
            }
            double mx,my;int ww,wh;glfwGetCursorPos(window,&mx,&my);glfwGetWindowSize(window,&ww,&wh);
            if(ww>0){state.uiX=float(mx)*1280/ww;state.uiY=float(my)*1280/ww;}
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
            if(capture&&frame==(rayCapture?1:22))glfwSetWindowShouldClose(window,GL_TRUE);
            if(verify){
                if(state.simulation.time<CAR_START&&pose.carTravel!=0)throw std::runtime_error("Early car motion");
                if(!released&&state.simulation.time>=CAR_START){released=true;
                    std::cout<<"RELEASE: active="<<fixed(state.simulation.time,4)<<" s, wall="<<fixed(now-started,4)<<" s, frame="<<frame<<std::endl;}
                if(state.simulation.time>=DEMO_END){
                    if(std::abs(pose.carTravel-9)>.001f)throw std::runtime_error("Car exit failed");
                    std::cout<<"PASS: full 70-second chain + 7-second car exit; "<<frame<<" GPU frames; mode, stage, camera, pause/reset and shading handlers"<<std::endl;
                    glfwSetWindowShouldClose(window,GL_TRUE);
                }
            }
        }
    }catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';result=1;}
    glfwDestroyWindow(window);glfwTerminate();return result;
}
