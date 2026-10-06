#pragma once
// Invariant tests use the same simulation/controller functions as the window.
static int selfTest(){
    int checks=0;auto check=[&](bool value,const char* name){++checks;if(!value)throw std::runtime_error(std::string("Check failed: ")+name);};
    check(RaySnapshot::selfTest(),"actual ray/triangle intersections and normal transform");
    Vec3 q=transform(translate({4,2,0})*rotateZ(PI/2)*scale({2,1,1}),{1,0,0});
    check(length(q-Vec3{4,4,0})<.0001f,"T R S order");
    Simulation s;s.update(5);check(s.time==0,"paused initially");s.running=true;s.update(32.7);s.running=false;s.update(5);
    check(s.time==32.7,"pause");s.reset();check(s.time==0&&!s.running,"reset");
    Simulation irregular,uniform;irregular.running=uniform.running=true;
    for(int i=0;i<100;++i){irregular.update(.037);irregular.update(.413);uniform.update(.45);}
    check(std::abs(irregular.time-uniform.time)<1e-9,"frame partition independent clock");
    s.running=true;s.update(69.999);check(s.time<70&&poseAt(float(s.time)).carTravel==0,"no early release");
    s.update(.001);check(stageAt(s.time)==12,"release at 70 active seconds");
    Pose last=poseAt(0);
    for(int i=0;i<=9240;++i){float t=i/120.f;Pose p=poseAt(t);
        check(std::abs(p.gearSmall+1.5f*p.gearLarge)<.0001f,"opposite 3:2 gear ratio");
        check(std::abs(p.wheelSpin*WHEEL_RADIUS+p.wheelTravel)<.0001f,"rolling wheel distance");
        check(std::abs(length(sliderPin(p.crank)-crankPin(p.crank))-ROD_LENGTH)<.0001f,"rigid connecting rod");
        check(p.weightDrop==p.gateLift,"constant rope length");
        check(std::abs(p.carSpin*CAR_WHEEL_RADIUS+p.carTravel)<.0001f,"car wheel distance");
        check(length(p.hand-last.hand)<.04f&&length(p.pin-last.pin)<.04f,"glove and component continuity");
        check(length(p.wheel-last.wheel)<.025f&&std::abs(p.carTravel-last.carTravel)<.025f,"continuous rolling objects");
        check(std::abs(p.crank-last.crank)<.025f,"continuous crank rotation");
        check(p.rack>=last.rack&&p.rack-last.rack<.01f,"one-way ratchet advances");
        if(t<70)check(p.carTravel==0,"car locked through chain");
        if(p.carTravel>0)check(p.gateLift>1.39f&&p.bridge==0&&p.coupling>.64f,"all exit prerequisites");
        if(p.attached)check(length(p.pin-(p.hand+Vec3{0,-.4f,0}))<.0001f,"local glove attachment");
        last=p;
    }
    s.interactive=true;s.reset();CarState car;
    for(int stage=0;stage<12;++stage){
        double start=s.time;s.update(100);check(s.time==start&&s.waiting,"stage waits for user");
        check(!s.activate((stage+1)%12),"out-of-order action rejected");
        check(s.activate(stage),"requested mechanism activated");
        s.update((STAGE_TIME[stage+1]-STAGE_TIME[stage])*.5);
        check(s.time>start&&s.time<STAGE_TIME[stage+1],"action visibly progresses");
        car.update(.5,s,1,1);if(stage<11)check(car.travel==0,"drive input cannot bypass interlock");
        s.update(100);check(s.time==STAGE_TIME[stage+1],"does not skip subsequent stage");
    }
    check(!s.waiting&&s.running&&s.time==70,"manual sequence releases car");
    car.reset();car.update(3,s,1,0);check(car.position.z>7&&car.travel>6,"drive forward onto apron");
    float yaw=car.yaw;car.update(.6,s,1,1);check(car.yaw>yaw+.1f,"left steering changes heading");
    car.speed=0;float before=car.travel;car.update(.6,s,-1,0);check(car.travel<before,"reverse signed distance");
    Pose driven;car.apply(driven);check(std::abs(driven.carSpin*CAR_WHEEL_RADIUS+car.travel)<.0001f,"manual wheel rotation");
    s.running=false;Vec3 parked=car.position;car.update(3,s,1,1);check(length(car.position-parked)==0,"pause freezes driving");
    car.reset();check(car.travel==0&&car.yaw==-PI/2&&!car.manual,"car reset");
    Camera camera;Vec3 eye=camera.eye;camera.move(1,0,0,1,false);check(std::abs(length(camera.eye-eye)-4)<.0001f,"free forward camera");
    eye=camera.eye;camera.move(0,1,1,1,true);check(std::abs(length(camera.eye-eye)-10)<.0001f,"diagonal speed normalized");
    auto direction=camera.forward();camera.look(50,-20);check(length(direction-camera.forward())>.1f,"mouse look");
    camera.preset(1);check(camera.presetId==1,"overview reset");
    App ui;auto click=[&](UiRect r){return ui.clickUi(r.x+r.w/2,r.y+r.h/2);};
    click(modeButton);check(ui.simulation.interactive&&ui.simulation.waiting,"interactive UI");
    click(playButton);check(ui.simulation.running&&!ui.simulation.waiting,"next button activates movement");
    ui.simulation.update(2);click(lightButton);check(ui.lighting==1&&ui.simulation.time==2,"independent lighting");
    check(!click(shadeButton),"closed tools ignore clicks");click(toolsButton);click(shadeButton);check(ui.shading==1,"shading button");
    click(shadeButton);check(ui.shading==2,"Gouraud selected");click(shadeButton);check(ui.shading==0,"Phong selected");
    click(resetButton);check(ui.simulation.time==0&&ui.car.travel==0,"full reset button");
    click(rayButton);check(ui.rayRequested,"ray snapshot button");
    std::cout<<"PASS: "<<checks<<" checks: timing, 12 manual actions, dependencies, glove, pivots, car, camera and UI\n";return 0;
}
static void verifyHandlers(GLFWwindow* w){
    auto check=[](bool b,const char* text){if(!b)throw std::runtime_error(text);};auto& a=app(w);
    auto key=[&](int k){keyCallback(w,k,0,GLFW_PRESS,0);};
    key(GLFW_KEY_SPACE);a.simulation.update(2);check(a.simulation.time==0,"Pause callback");
    key(GLFW_KEY_I);
    for(int i=0;i<12;++i){
        key(i<5?GLFW_KEY_5+i:GLFW_KEY_F);check(a.simulation.running,"Mechanism key callback");
        a.simulation.update(100);check(a.simulation.time==STAGE_TIME[i+1],"Mechanism key dependency");
    }
    for(int i=1;i<=4;++i){key(GLFW_KEY_0+i);check(a.camera.presetId==i,"View callback");}
    Vec3 eye=a.camera.eye;scroll(w,0,1);check(length(a.camera.eye-eye)>.7f,"Dolly callback");
    a.looking=true;cursor(w,40,20);check(a.camera.presetId==0,"Look callback");key(GLFW_KEY_ESCAPE);check(!a.looking,"Mouse release");
    key(GLFW_KEY_L);check(a.shading==2,"Gouraud callback");
    key(GLFW_KEY_L);check(a.shading==0,"Phong callback");
    key(GLFW_KEY_B);check(a.lighting==1,"Light callback");
    key(GLFW_KEY_I);key(GLFW_KEY_R);check(a.simulation.time==0&&a.car.travel==0,"Reset callback");key(GLFW_KEY_SPACE);
    a.camera.preset(1);a.lighting=0;
}
