#pragma once
// Exercise the production clock, poses, controls and geometry interlocks.
static int selfTest(){
    int checks=0;auto check=[&](bool value,const char* name){++checks;if(!value)throw std::runtime_error(std::string("Check failed: ")+name);};
    check(RaySnapshot::selfTest(),"ray/triangle intersection and normal transform");
    check(STAGE_TIME.back()==CAR_START&&ACTION_COUNT==17,"17 actions release at 70 seconds");
    Vec3 q=transform(translate({4,2,0})*rotateZ(PI/2)*scale({2,1,1}),{1,0,0});
    check(length(q-Vec3{4,4,0})<.0001f,"T R S order");
    Simulation s;s.update(5);check(s.time==0,"paused initially");s.running=true;s.update(32.7);s.running=false;s.update(5);
    check(s.time==32.7,"pause");s.reset();check(s.time==0&&!s.running,"reset");
    Simulation irregular,uniform;irregular.running=uniform.running=true;
    for(int i=0;i<100;++i){irregular.update(.037);irregular.update(.413);uniform.update(.45);}
    check(std::abs(irregular.time-uniform.time)<1e-9,"frame partition independent clock");
    s.running=true;s.update(69.999);check(s.time<70&&poseAt(float(s.time)).carTravel==0,"no early release");
    s.update(.001);check(stageAt(s.time)==ACTION_COUNT,"release at 70 active seconds");
    check(std::abs(poseAt(10).gearLarge-2*PI)<1e-5f,"one winding turn");
    check(poseAt(29).crank==0&&poseAt(24).crank==CRANK_INITIAL,"single crank stroke");
    Pose last=poseAt(0);
    for(int i=0;i<=9240;++i){float t=i/120.f;Pose p=poseAt(t);
        check(std::abs(p.gearSmall+1.5f*p.gearLarge)<.0001f,"opposite 3:2 gear ratio");
        check(std::abs(p.wheelSpin*WHEEL_RADIUS+p.wheelTravel)<.0001f,"rolling wheel distance");
        check(std::abs(length(sliderPin(p.crank)-crankPin(p.crank))-ROD_LENGTH)<.0001f,"rigid connecting rod");
        check(p.weightDrop==p.gateLift,"constant counterweight rope length");
        check(std::abs(p.carSpin*CAR_WHEEL_RADIUS+p.carTravel)<.0001f,"car wheel distance");
        check(length(p.hand-last.hand)<.04f&&length(p.pin-last.pin)<.04f,"glove and attachment continuity");
        check(length(p.wheel-last.wheel)<.03f&&std::abs(p.carTravel-last.carTravel)<.025f,"rolling continuity");
        check(std::abs(p.crank-last.crank)<.025f,"crank continuity");
        check(p.rack>=last.rack&&p.rack-last.rack<.01f,"single rack advance");
        check(length(p.relayBall-last.relayBall)<.025f,"new ball and cup continuity");
        check(length(pendulumBob(p.pendulum)-pendulumBob(last.pendulum))<.015f,"pendulum continuity");
        check(std::abs(p.rocker-last.rocker)<.005f&&std::abs(p.releasePin-last.releasePin)<.005f,"rocker and pin continuity");
        check(std::abs(p.ballSpin*BALL_RADIUS-p.ballTravel)<.0001f,"new ball rolls by s/r");
        check(std::abs(p.releaseDrum*.42f+p.bolt)<.0001f,"cable drum travel");
        check(std::abs(length(rackFollower(p.rack)-Vec3{6.30f+.95f*p.rack,2.25f,.78f})-RACK_LINK_LENGTH)<.0001f,"rigid rack lift link");
        check(std::abs(p.ballGate+rackFollower(p.rack).y-rackFollower(0).y)<.0001f,"constant ball stop cable length");
        if(p.ballTravel>0)check(p.ballGate>.949f,"ball waits until stop clears sphere");
        if(p.cup!=0)check(p.ballTravel==poseAt(34).ballTravel,"cup waits for ball");
        if(p.pendulum<radians(45))check(p.cup>=radians(31.99f),"pendulum waits for retaining finger");
        if(p.rocker!=0)check(p.pendulum<=-radians(44.99f),"rocker waits for striker");
        if(p.releasePin>0)check(p.rocker<=-radians(24.99f),"pin waits for collar contact");
        if(p.bolt>0)check(p.releasePin>.649f,"cable waits for pin withdrawal");
        if(t<70)check(p.carTravel==0,"car stays locked through every stage");
        if(p.carTravel>0)check(p.gateLift>1.39f&&p.bridge==0&&p.coupling>.64f,"exit prerequisites");
        if(p.attached)check(length(p.pin-(p.hand+Vec3{0,-.4f,0}))<.0001f,"local glove attachment");
        if(t>=34)check(length(p.relayBall-transform(cupMatrix(p.cup),BALL_END-CUP_PIVOT))<.0001f,"ball remains in captive cup");
        last=p;
    }
    check(std::abs((BALL_END.y-BALL_RADIUS)-(CUP_PIVOT.y+.10f))<.001f,"ball touches cup floor");
    Vec3 hit=pendulumBob(-radians(45));
    check(std::abs((hit.x-.27f)-(ROCKER_PIVOT.x+1.11f))<.06f&&hit.y>1.1f&&hit.y<1.85f,"striker contacts rocker pad");
    Vec3 tip=rockerTip(-radians(25));
    check(std::abs(tip.x-RELEASE_PIN.x)<.01f&&std::abs(tip.y+.09f-(RELEASE_PIN.y+.55f))<.01f,"rocker contacts release collar");
    s.interactive=true;s.reset();CarState car;
    for(int stage=0;stage<ACTION_COUNT;++stage){
        double start=s.time,duration=STAGE_TIME[stage+1]-start;auto control=stageControl(stage);
        s.update(100,control.key);check(s.time==start&&s.waiting,"stage waits for fresh input");
        check(!s.operate('Y'),"unrelated key cannot advance stage");
        if(control.operation!=Operation::Release)check(!s.activate(),"F cannot bypass individual operation");
        check(s.operate(control.key),"correct mechanism key accepted");
        if(control.operation==Operation::QuarterTurn){
            s.update(100);check(s.time==start+duration*.25&&s.waiting,"one tap advances exactly a quarter turn");
            for(int tap=0;tap<3;++tap){s.operate(control.key);s.update(100);}
        }else{
            s.update(duration*.35,control.key);check(s.time>start&&s.time<STAGE_TIME[stage+1],"action visibly progresses");
            if(control.operation==Operation::Hold){double partial=s.time;s.update(1);check(s.time==partial,"release key holds actual partial pose");}
            car.update(.5,s,1,1);check(car.travel==0,"drive cannot bypass interlock");
            s.running=false;double paused=s.time;s.update(1,control.key);check(s.time==paused,"pause during manual operation");s.running=true;
            s.update(100,control.key);
        }
        check(s.time==STAGE_TIME[stage+1],"large frame cannot skip next mechanism");
    }
    check(!s.waiting&&s.running&&s.time==70,"hands-on chain releases car");
    car.reset();car.update(3,s,1,0);check(car.position.z>7&&car.travel>6,"drive onto apron");
    float yaw=car.yaw;car.update(.6,s,1,1);check(car.yaw>yaw+.1f,"left steering changes heading");
    car.speed=0;float before=car.travel;car.update(.6,s,-1,0);check(car.travel<before,"reverse signed distance");
    Pose driven;car.apply(driven);check(std::abs(driven.carSpin*CAR_WHEEL_RADIUS+car.travel)<.0001f,"manual wheel spin");
    s.running=false;Vec3 parked=car.position;car.update(3,s,1,1);check(length(car.position-parked)==0,"pause freezes car");
    car.reset();s.reset();check(car.travel==0&&s.turnTarget==0&&s.waiting,"reset car and queued gear turns");
    Camera camera;Vec3 eye=camera.eye;camera.move(1,0,0,1,false);check(std::abs(length(camera.eye-eye)-4)<.0001f,"free camera");
    eye=camera.eye;camera.move(0,1,1,1,true);check(std::abs(length(camera.eye-eye)-10)<.0001f,"normalized fast diagonal");
    auto direction=camera.forward();camera.look(50,-20);check(length(direction-camera.forward())>.1f,"mouse look");
    for(auto size:std::array<std::array<int,2>,3>{{{{1440,900}},{{1280,720}},{{1920,800}}}}){
        UiLayout layout(size[0],size[1]);check(layout.height>=719.99f,"guide fits short or wide windows");
        float px=(closeGuideButton.x+layout.left)*layout.factor;
        check(std::abs(px/layout.factor-layout.left-closeGuideButton.x)<.001f,"guide draw and click scaling match");
    }
    App ui;auto click=[&](UiRect r){return ui.clickUi(r.x+r.w/2,r.y+r.h/2);};
    click(modeButton);check(ui.simulation.interactive&&ui.simulation.waiting,"mode UI");
    click(playButton);check(ui.simulation.time==0&&ui.simulation.waiting,"Next cannot operate glove");
    check(!click(shadeButton),"closed guide ignores tool clicks");click(toolsButton);click(sequenceTab);check(ui.guidePage==1,"sequence tab");
    click(shadeButton);check(ui.shading==1,"flat button");click(shadeButton);check(ui.shading==2,"Gouraud button");click(shadeButton);check(ui.shading==0,"Phong button");
    click(shadowButton);check(!ui.shadows,"shadow button");click(dimButton);click(brightButton);check(std::abs(ui.brightness-1)<.0001f,"exposure buttons");
    click(lightButton);check(ui.lighting==1,"lighting button");click(rayButton);check(ui.rayRequested,"snapshot button");
    click(closeGuideButton);check(!ui.controls,"obvious guide close button");
    std::cout<<"PASS: "<<checks<<" checks: 17 actions, 70 seconds, partial manual controls, contacts, continuity, reset, car, camera, guide and rays\n";return 0;
}
static void verifyHandlers(GLFWwindow* w){
    auto check=[](bool b,const char* text){if(!b)throw std::runtime_error(text);};auto& a=app(w);
    auto key=[&](int k,int action=GLFW_PRESS){keyCallback(w,k,0,action,0);};
    key(GLFW_KEY_SPACE);a.simulation.update(2);check(a.simulation.time==0,"Pause callback");
    key(GLFW_KEY_I);
    for(int i=0;i<ACTION_COUNT;++i){
        auto control=stageControl(i);double start=a.simulation.time;
        key(GLFW_KEY_Y);a.simulation.update(1,a.mechanismHeld());check(a.simulation.time==start,"Invalid mechanism callback");
        if(control.operation==Operation::QuarterTurn){
            for(int tap=0;tap<4;++tap){
                key(control.key);key(control.key,GLFW_REPEAT);key(control.key,GLFW_RELEASE);
                a.simulation.update(100,a.mechanismHeld());check(a.simulation.time==start+(tap+1),"Quarter turn callback / ignore repeat");
            }
        }else{
            key(control.key);check(a.simulation.running,"Mechanism key callback");
            a.simulation.update(.5,a.mechanismHeld());
            key(control.key,GLFW_RELEASE);double partial=a.simulation.time;
            if(control.operation==Operation::Hold){a.simulation.update(1,a.mechanismHeld());check(a.simulation.time==partial,"Hold key release callback");key(control.key);}
            a.simulation.update(100,a.mechanismHeld());key(control.key,GLFW_RELEASE);
        }
        check(a.simulation.time==STAGE_TIME[i+1],"Sequential input dependency");
    }
    for(int i=1;i<=6;++i){key(GLFW_KEY_0+i);check(a.camera.presetId==i,"View callback");}
    Vec3 eye=a.camera.eye;scroll(w,0,1);check(length(a.camera.eye-eye)>.7f,"Dolly callback");
    a.looking=true;cursor(w,40,20);check(a.camera.presetId==0,"Look callback");key(GLFW_KEY_ESCAPE);check(!a.looking,"Mouse release");
    key(GLFW_KEY_C);check(a.controls,"Guide key opens");key(GLFW_KEY_ESCAPE);check(!a.controls,"Escape closes guide");
    key(GLFW_KEY_H);check(a.labels,"Labels callback");key(GLFW_KEY_H);
    key(GLFW_KEY_L);check(a.shading==2,"Gouraud callback");key(GLFW_KEY_L);check(a.shading==0,"Phong callback");
    key(GLFW_KEY_B);check(a.lighting==1,"Light callback");
    key(GLFW_KEY_T);check(a.rayRequested,"Ray request callback");a.rayRequested=false;
    a.rayActive=true;a.resumeAfterRay=false;key(GLFW_KEY_T);check(!a.rayActive&&!a.simulation.running,"Return from ray preserves pause");
    key(GLFW_KEY_I);key(GLFW_KEY_R);check(a.simulation.time==0&&a.car.travel==0&&!a.mechanismHeld(),"Reset callback");key(GLFW_KEY_SPACE);
    a.camera.preset(1);a.lighting=0;
}

