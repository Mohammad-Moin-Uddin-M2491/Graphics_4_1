#pragma once
// Exercise the production clock, poses, controls and geometry interlocks.
static int selfTest(){
    int checks=0;auto check=[&](bool value,const char* name){++checks;if(!value)throw std::runtime_error(std::string("Check failed: ")+name);};
    check(RaySnapshot::selfTest(),"ray/triangle intersection and normal transform");
    check(std::abs(lampAttenuation(4)/lampAttenuation(8)-4)<1e-6f,"lamp inverse square falloff");
    for(int preset=0;preset<4;++preset){auto light=lightingPreset(preset);
        check(length(light.lampPosition-LAMP_BULB)==0,"visible bulb equals lighting origin");
        check((length(light.keyColor)==0)==(preset==0||preset==3),"studio/night have no hidden directional source");
    }
    for(int preset=1;preset<=6;++preset)for(float aspect:{1.6f,16.f/9,2.4f}){
        Camera c;c.preset(preset);Mat4 vp=cameraProjection(aspect)*lookAt(c.eye,c.eye+c.forward(),{0,1,0});
        for(float x:{-.9f,0.f,.9f})for(float y:{-.8f,0.f,.8f}){
            Vec3 ray=cameraRay(c.forward(),x,y,aspect),world=c.eye+ray*10,clip=transform(vp,world);
            float w=vp.m[3]*world.x+vp.m[7]*world.y+vp.m[11]*world.z+vp.m[15];
            check(std::abs(clip.x/w-x)<1e-5f&&std::abs(clip.y/w-y)<1e-5f,"camera ray projects back to requested pixel");
        }
    }
    Mat4 projection=cameraProjection(1.6f);
    check(std::abs(transform(projection,{0,0,-CAMERA_NEAR}).z/CAMERA_NEAR+1)<1e-5f&&
        std::abs(transform(projection,{0,0,-CAMERA_FAR}).z/CAMERA_FAR-1)<1e-5f,"perspective near/far clip planes");
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
    check(std::abs(poseAt(float(STAGE_TIME[Cam])).gearLarge-2*PI)<1e-5f,"one winding turn");
    check(poseAt(float(STAGE_TIME[Ball])).crank==0&&poseAt(float(STAGE_TIME[Crank])).crank==CRANK_INITIAL,"single crank stroke");
    int beforeSamples=checks;
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
        if(p.cup!=0)check(p.ballTravel==poseAt(float(STAGE_TIME[Cup])).ballTravel,"cup waits for ball");
        if(p.pendulum<radians(45))check(cupFingerGap(p)>=0,"swinging striker stays clear of retaining finger");
        if(p.rocker!=0)check(p.pendulum<=-radians(44.99f),"rocker waits for striker");
        if(p.releasePin>0)check(p.rocker<=-radians(24.99f),"pin waits for collar contact");
        if(p.bolt>0)check(p.releasePin>.649f,"cable waits for pin withdrawal");
        if(t<70)check(p.carTravel==0,"car stays locked through every stage");
        if(p.carTravel>0)check(p.gateLift>1.39f&&p.bridge==0&&p.coupling>.64f,"exit prerequisites");
        if(p.attached)check(length(p.pin-(p.hand+Vec3{0,-.4f,0}))<.0001f,"local glove attachment");
        if(t>=STAGE_TIME[Cup])check(length(p.relayBall-transform(cupMatrix(p.cup),BALL_END-CUP_PIVOT))<.0001f,"ball remains in captive cup");
        last=p;
    }
    int sampledChecks=checks-beforeSamples;
    // Focused boundary cases are separate from the repeated continuity samples.
    // Each valid handoff has an adversarial physical pose that MUST stop it.
    for(int next=1;next<=ACTION_COUNT;++next){
        Pose valid=poseAt(float(STAGE_TIME[next]));
        check(handoffReady(next,valid),handoffFailure(next));
        Pose bad=valid;
        switch(next){
        case Gears:bad.pin.x+=.2f;break;case Cam:bad.gearLarge=0;break;
        case Wheel:bad.lever=0;break;case Wiper:bad.wheel.x-=.2f;break;
        case Bar:bad.wiper=0;break;case Crank:bad.bar=0;break;
        case Ball:bad.ballGate=0;break;case Cup:bad.relayBall.x+=.3f;break;
        case Pendulum:bad.cup=0;bad.relayBall=BALL_END;break;
        case Rocker:bad.pendulum=0;break;case Pin:bad.rocker=0;break;
        case Drum:bad.releasePin=0;break;case Catch:bad.bolt=0;break;
        case Weight:bad.catchAngle=0;break;case Bridge:bad.gateLift=0;break;
        case Starter:bad.bridge=radians(5);break;case CarExit:bad.coupling=0;break;
        }
        check(!handoffReady(next,bad),"misaligned or obstructed handoff rejected");
        Simulation fault;fault.time=STAGE_TIME[next];fault.running=true;
        check(!fault.checkBoundary(next,bad),"production boundary guard rejects bad observed pose");
        fault.update(100);check(fault.time==STAGE_TIME[next]&&!fault.running&&fault.blockedStage==next,"blocked stage cannot advance with elapsed time");
        fault.interactive=true;check(!fault.activate(),"input cannot override failed clearance");
        fault.reset();check(fault.blockedStage==-1,"reset clears boundary fault");
    }
    float release=pendulumReleaseTime();
    check(release>STAGE_TIME[Cup]&&release<STAGE_TIME[Pendulum],"striker releases during cup tipping");
    Pose held=poseAt(release-.001f),released=poseAt(release);
    check(held.pendulum==radians(45)&&cupStopGap(held.cup)<.005f,"striker remains held until finger clears");
    check(cupStopGap(released.cup)>.0049f,"release comes from actual finger clearance");
    Vec3 restingBob=pendulumBob(radians(45));
    check(length(pendulumBob(poseAt(release+.1f).pendulum)-restingBob)>.002f,"striker begins moving within 0.1 seconds of release");
    check(length(pendulumBob(poseAt(release+.5f).pendulum)-restingBob)>.1f,"release has no visible half-second dwell");
    check(poseAt(float(STAGE_TIME[Pendulum])).pendulum<radians(35),"striker already swinging when cup finishes");
    Pose before=poseAt(float(STAGE_TIME[Cup])),unlatched=poseAt(float(STAGE_TIME[Pendulum]));
    check(cupFingerGap(before)>=-.065f&&cupFingerGap(before)<.015f,"finger initially contacts striker");
    check(cupFingerGap(unlatched)>.12f,"finger withdraws with positive clearance");
    check(std::abs(strikerPadGap(poseAt(float(STAGE_TIME[Rocker]))))<.01f,"striker pad contact gap under 0.01");
    check(std::abs(rollerCollarGap(poseAt(float(STAGE_TIME[Pin]))))<.01f,"roller collar contact gap under 0.01");
    check(pinDrumGap(poseAt(0))<0&&pinDrumGap(poseAt(float(STAGE_TIME[Drum])))>.1f,"pin initially blocks then clears drum rim");
    auto cableLength=[](const Pose& p){return DRUM_RADIUS*(PI-p.releaseDrum)+(CABLE_BEND_XY.x-DRUM_CENTER.x)+
        PI*CABLE_GUIDE_RADIUS+(CABLE_BEND_YZ.y-CABLE_BEND_XY.y)+(CABLE_BEND_YZ.z-boltCablePoint(p.bolt).z);};
    float rope=cableLength(poseAt(float(STAGE_TIME[Drum])));
    for(float u:{0.f,.25f,.5f,.75f,1.f}){
        Pose p=poseAt(float(STAGE_TIME[Drum]+u*stageDuration(Drum)));
        check(std::abs(cableLength(p)-rope)<.0001f,"spool wrap and routed cable conserve length");
    }
    check(std::abs(CABLE_BEND_XY.x+CABLE_GUIDE_RADIUS-CABLE_BEND_YZ.x)<.0001f&&
        std::abs(CABLE_BEND_XY.z-CABLE_BEND_YZ.z-CABLE_GUIDE_RADIUS)<.0001f,"guide arcs meet vertical tangent");
    Pose coupled=poseAt(float(STAGE_TIME[Starter]));
    check(handoffReady(Starter,coupled),"shaft fits car-mounted socket before cranking");
    coupled.coupling=.3f;check(!handoffReady(Starter,coupled),"disconnected starter cannot engage car");
    Pose departed=poseAt(CAR_START);
    check(roofClearance(departed)>.1f&&starterShoe(departed).x-.14f>9.88f+.2f,"roof and tire shoe exit clearance");
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
    car.speed=0;float beforeTravel=car.travel;car.update(.6,s,-1,0);check(car.travel<beforeTravel,"reverse signed distance");
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
    check(ui.playLabel()=="G GUIDE","button advertises required held key");
    click(playButton);check(ui.controls&&ui.guidePage==OperatePage&&ui.simulation.time==0,"held-key button opens current guide");
    click(closeGuideButton);check(!click(shadeButton),"closed guide ignores tools");click(toolsButton);
    for(int page=0;page<GuidePageCount;++page){click(guideTab(page));check(ui.guidePage==page,"guide page button");}
    click(guideTab(LightingPage));
    click(shadeButton);check(ui.shading==1,"flat button");click(shadeButton);check(ui.shading==2,"Gouraud button");click(shadeButton);check(ui.shading==0,"Phong button");
    click(shadowButton);check(!ui.shadows,"shadow button");click(dimButton);click(brightButton);check(std::abs(ui.brightness-1)<.0001f,"exposure buttons");
    check(!ui.clickLamp(400,300,1440,900),"guide blocks scene lamp clicks");
    ui.cycleLighting();check(ui.lighting==1,"lighting shortcut");click(rayButton);check(ui.rayRequested,"ray snapshot button");
    click(closeGuideButton);check(!ui.controls,"close guide");
    ui.simulation.time=STAGE_TIME[Gears];ui.simulation.waiting=true;
    check(ui.playLabel()=="Z GUIDE","quarter-turn button advertises key");click(playButton);check(ui.controls&&ui.simulation.waiting,"quarter-turn guide click does not falsely operate");
    ui.simulation.time=STAGE_TIME[Ball];ui.controls=false;
    check(ui.playLabel()=="F GO","release button clearly operates action");click(playButton);check(ui.simulation.running&&!ui.simulation.waiting,"release button advances ready action");
    ui.simulation.time=STAGE_TIME[Cam]+.5;ui.held.fill(false);ui.simulation.running=true;
    check(ui.playLabel()=="X GUIDE","partially held action shows its required key");
    ui.simulation.blockedStage=Cup;click(playButton);check(ui.controls&&ui.guidePage==OperatePage,"blocked button explains failed contact");
    std::cout<<"PASS: 17 nominal + 17 obstructed handoffs; "<<checks-sampledChecks<<" focused geometry/controller/UI assertions\n"
        <<"PASS: 9241 time samples / "<<sampledChecks<<" repeated numeric invariants (not visual coverage)\n";
    return 0;
}
static void verifyLighting(Renderer& renderer){
    auto check=[](bool b,const char* text){if(!b)throw std::runtime_error(text);};
    int triangles=0;
    for(const auto& mesh:renderer.meshes)for(size_t i=0;i<mesh.vertices.size();i+=3){
        const auto& a=mesh.vertices[i];const auto& b=mesh.vertices[i+1];const auto& c=mesh.vertices[i+2];
        Vec3 face=cross(b.position-a.position,c.position-a.position);
        check(length(face)>1e-8f,"Degenerate mesh triangle");
        check(dot(normalize(face),normalize(a.normal+b.normal+c.normal))>.1f,"Triangle winding disagrees with surface normals");++triangles;
    }
    const Vec3 axes[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    std::vector<Draw> boxes;
    for(Vec3 axis:axes)boxes.push_back({0,translate(LAMP_BULB+axis*2),{{.5f,.5f,.5f}}});
    boxes.push_back({2,translate(LAMP_BULB)*scale({LAMP_RADIUS,LAMP_RADIUS,LAMP_RADIUS}),{{1,1,1},32,0,0,2}});
    Vec3 eye{1.2f,9,9},target{1.2f,0,-1.1f};Mat4 view=lookAt(eye,target,{0,1,0}),projection=cameraProjection(1);
    renderer.draw(boxes,view,projection,eye,512,512,0,0,1,true);
    glBindFramebuffer(GL_FRAMEBUFFER,renderer.shadowFbo);
    for(int face=0;face<6;++face){
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_CUBE_MAP_POSITIVE_X+face,renderer.pointTexture,0);
        float depth=0;glReadPixels(Renderer::POINT_RESOLUTION/2,Renderer::POINT_RESOLUTION/2,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&depth);
        check(std::abs(depth*Renderer::POINT_FAR-1.5f)<.002f,"Lamp cube shadow face stores wrong radial distance");
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    std::vector<Draw> probe{
        {6,translate({0,-.05f,0})*scale({20,.1f,20}),{{.5f,.5f,.5f},16,0}},
        {0,translate((LAMP_BULB+target)*.5f)*scale({.8f,.8f,.8f}),{{.5f,.5f,.5f},16,0}}
    };
    float lit[3]{},shadowed[3]{};
    for(int shading=0;shading<3;++shading){
        renderer.draw(probe,view,projection,eye,512,512,shading,0,1,false);glReadPixels(256,256,1,1,GL_RGB,GL_FLOAT,lit);
        renderer.draw(probe,view,projection,eye,512,512,shading,0,1,true);glReadPixels(256,256,1,1,GL_RGB,GL_FLOAT,shadowed);
        check(lit[0]>shadowed[0]+.2f,"Lamp fails to shadow receiver in a shading mode");
    }
    // Move the blocker away: illumination must return at the same receiver.
    probe[1].model=translate({6,2,5});
    renderer.draw(probe,view,projection,eye,512,512,0,0,1,true);glReadPixels(256,256,1,1,GL_RGB,GL_FLOAT,shadowed);
    check(std::abs(shadowed[0]-lit[0])<.04f,"Lamp receiver self-shadows after blocker moves away");
    check(glGetError()==GL_NO_ERROR,"OpenGL lighting verification error");
    std::cout<<"PASS: "<<triangles<<" mesh triangle orientations; six radial shadow faces; lamp occlusion in Phong/Flat/Gouraud; unobstructed receiver\n";
}
static void verifyHandlers(GLFWwindow* w){
    auto check=[](bool b,const char* text){if(!b)throw std::runtime_error(text);};auto& a=app(w);
    auto key=[&](int k,int action=GLFW_PRESS){keyCallback(w,k,0,action,0);};
    // Exercise the mouse handler's shared dispatch, including resize. Hidden GLFW
    // windows cannot reliably warp the OS pointer, so supply its window coordinates.
    for(auto size:std::array<std::array<int,2>,3>{{{{1440,900}},{{960,600}},{{1920,800}}}}){
        glfwSetWindowSize(w,size[0],size[1]);glfwPollEvents();
        int width,height;glfwGetWindowSize(w,&width,&height);a.camera.preset(1);
        Vec3 delta=LAMP_BULB-a.camera.eye,f=a.camera.forward(),r=a.camera.right(),up=cross(r,f);
        float z=dot(delta,f),tangent=std::tan(radians(CAMERA_FOV)*.5f);
        float x=(dot(delta,r)/(z*tangent*float(width)/height)+1)*width*.5f;
        float y=(1-dot(delta,up)/(z*tangent))*height*.5f;
        for(int mode=0;mode<4;++mode){leftClick(w,x,y);check(a.lighting==(mode+1)%4,"Lamp click cycles all four modes");}
        a.controls=true;check(!a.clickLamp(x,y,width,height)&&a.lighting==0,"Guide prevents lamp click-through");a.controls=false;
        a.looking=true;check(!a.clickLamp(x,y,width,height),"Mouse look prevents lamp changes");a.looking=false;
        a.rayActive=true;check(!a.clickLamp(x,y,width,height),"Snapshot blocks lamp changes");a.rayActive=false;
        check(!a.clickLamp(2,2,width,height)&&a.lighting==0,"HUD is not a lighting button");
    }
    glfwSetWindowSize(w,1440,900);glfwPollEvents();
    std::vector<Draw> occluded{{0,translate(a.camera.eye+a.camera.forward()*2)*scale({4,4,4}),dark}};
    occluded.insert(occluded.end(),a.scene->items.begin(),a.scene->items.end());
    check(a.renderer->pick(occluded,a.camera.eye,normalize(LAMP_BULB-a.camera.eye))==0,"Foreground geometry blocks lamp picking");
    key(GLFW_KEY_SPACE);a.simulation.update(2);check(a.simulation.time==0,"Pause callback");
    key(GLFW_KEY_I);
    for(int i=0;i<ACTION_COUNT;++i){
        auto control=stageControl(i);double start=a.simulation.time;
        key(GLFW_KEY_Y);a.simulation.update(1,a.mechanismHeld());check(a.simulation.time==start,"Invalid mechanism callback");
        if(control.operation==Operation::QuarterTurn){
            for(int tap=0;tap<4;++tap){
                key(control.key);key(control.key,GLFW_REPEAT);key(control.key,GLFW_RELEASE);
                a.simulation.update(100,a.mechanismHeld());check(a.simulation.time==start+(tap+1)*stageDuration(i)/4,"Quarter turn callback / ignore repeat");
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

