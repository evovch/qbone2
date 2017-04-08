//
//  bcontrol.cpp
//  bone
//
//  Created by korytov on 26/03/16.
//  Copyright (c) 2016 korytov. All rights reserved.
//
#include "bcontrol.h"

#include <sstream>

#include <QNetworkInterface>

#include "bcamera.h"
#include "stepperspi.h"
#include "pwlistener.h"
#include "mainwidget.h"

bControl::bControl(QObject *parent) : QObject(parent) {
//    qRegisterMetaType<std::string>("std::string");

    mw = new MainWidget();
    mw->show();

    // List all interfaces
    QList<QNetworkInterface> listOfAllInterfaces = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface curInterface, listOfAllInterfaces) {
        qDebug() << curInterface.humanReadableName();
    }

    QString iface = QNetworkInterface::interfaceFromName("lo").addressEntries().at(0).ip().toString();
    qDebug() << "Setting iface = " << iface;
    mw->setIp(iface);


    pwl = new PWListener(49); //or 49
    QThread *threadPWL = new QThread();
    connect(pwl, SIGNAL(grabbedSignal(int)), this, SLOT(onPWSignal(int)));
    pwl->moveToThread(threadPWL);
    threadPWL->start();
    
    std::cout << "bcontrol up\n" << std::flush;
  
    l6470Setup ms; //defaults are taken from btypes.h

/*
    ms.m_stp = 16;
    ms.mot_acc = 800;
    ms.max_spd = 1500;
    ms.kval_hold = 120;
    ms.kval_run = ms.kval_hold;
    ms.slp_acc = 0x62;
    ms.int_spd = 0x18C6;
    ms.st_slp = 0x16;
*/

    spiBus0 = new FS_SPI(500000, "/dev/spidev0.0");
    spiBus1 = new FS_SPI(500000, "/dev/spidev0.3");
    spiBus2 = new FS_SPI(500000, "/dev/spidev0.2");
    spiBus3 = new FS_SPI(500000, "/dev/spidev0.1");

    ms.m_stp = 8;
    ms.mot_acc = 350;
    ms.max_spd = 600;

    ms.int_spd = 0x18C6;
    ms.slp_acc = 0xd;
    ms.st_slp = 0xc;

    ms.kval_hold = 60;
    ms.kval_run = 65;

    m_tilt = new stepperSpi(spiBus1, 2, 83, ms);
 //   QThread *thread_tilt = new QThread();
 //   m_tilt->moveToThread(thread_tilt);
 //   thread_tilt->start();

    m_pan  = new stepperSpi(spiBus2, 1, 203, ms);
//    QThread *thread_pan = new QThread();
//    m_pan->moveToThread(thread_pan);
//    thread_pan->start();

    ms.m_stp = 16;
    ms.mot_acc = 500;
    ms.max_spd = 200;
    ms.kval_hold = 100;
    ms.kval_run = ms.kval_hold;

    m_zoom = new stepperSpi(spiBus0, 3, 157, ms);

//    QThread *thread_zoom = new QThread();
//    m_zoom->moveToThread(thread_zoom);
//    thread_zoom->start();

    m_focus = new stepperSpi(spiBus3, 2, 110, ms);;
    m_slider = m_focus;

    tl = new Timelapse();
    tl->setMotors(m_slider, m_pan, m_tilt, m_zoom);

    m_tilt->setRange(0, 100000);
    m_pan->setRange(0, 100000);
    m_zoom->setRange(0, 100000);
    m_focus->setRange(0, 100000);
    m_slider->setRange(0, 100000);

    m_tilt->setCenter();
    m_pan->setCenter();

    m_zoom->flipReverse();
    m_tilt->flipReverse();
    
    liveViewOn = false;
    
    fixedPoint p1, p2, p3;
    p1.pan = 100;
    p1.tilt = 100;
    p1.name = std::string("A");
    p1.id = std::string("p1");
    
    p2.pan = 1000;
    p2.tilt = 1000;
    p2.name = std::string("B");
    p2.id = std::string("p2");
    p2.timelapseMember = 1;
    
    p3.pan = 2000;
    p3.tilt = 100;
    p3.name = std::string("C");
    p3.id = std::string("p3");

    addFixedPoint(p1.id, p1);
    addFixedPoint(p2.id, p2);
    addFixedPoint(p3.id, p3);
    
    fpCounter = 1;

    timelapseIsRunning = false;
    
    tlSettings.tlFrames = 100;
    tlSettings.tlDelay = 5;
    tlSettings.tlDirection = 1;
    tlSettings.tlProgress = 0;
    tlSettings.tlShutterdelay = 0.5;
    tlSettings.tlMirrorup = 0;
    tlSettings.tlStabdelay = 1;

    QTimer *timerTiny = new QTimer(this);
    connect(timerTiny, SIGNAL(timeout()), this, SLOT(onTimerTiny()));
    timerTiny->start(50);

    QTimer *timerHeavy = new QTimer(this);
    connect(timerHeavy, SIGNAL(timeout()), this, SLOT(onTimerHeavy()));
    timerHeavy->start(250);

    currentConnectionsCount = 0;

    cam = NULL;
}

void bControl::setCurrentConnectionsCount(int c) {
    currentConnectionsCount = c;
}

void bControl::onTimerTiny() {
    senderTiny();
}

void bControl::onTimerHeavy() {
    senderHeavy();
}

void bControl::setCamera(bCamera *c) {
    cam = c;
    tl->setCamera(cam);

    camFlushTerminator = false;
    connect(cam, SIGNAL(logToClient(QString)), this, SLOT(onLogToClient(QString)));
}

fixedPoints bControl::getFixedPoints(void) {
    return fPoints;
}

int bControl::getFixedPointsSize(void) {
    return fPoints.size();
}

void bControl::removeFixedPoint(std::string id) {
    if(fPoints.find(id) == fPoints.end())return;
    fPoints.erase(id);
    updateFixedPointsVector();
}

void bControl::toggleTimelapseFixedPoint(std::string id) {
    std::map<std::string, fixedPoint>::iterator it = fPoints.find(id);
    
    std::cout << "toggling FP in timelapse\r\n";
    if (it != fPoints.end()) {
        if(it->second.timelapseMember == 0) {
            fixedPointsVector tlPoints;
            std::copy_if(fPointsVector.begin(), fPointsVector.end(), std::back_inserter(tlPoints), [this](fixedPoint fp) { return fp.timelapseMember == 1; } );
            if(tlPoints.size() >= 3)return;
            it->second.timelapseMember = 1;
        }
        else it->second.timelapseMember = 0;
    }
    updateFixedPointsVector();
}

void bControl::selectFixedPoint(std::string id) {
    if(fPoints.find(id) == fPoints.end())return;
    currentFixedPointId = id;

    m_pan->async_seek(fPoints.at(currentFixedPointId).pan);
    m_tilt->async_seek(fPoints.at(currentFixedPointId).tilt);
    m_zoom->async_seek(fPoints.at(currentFixedPointId).zoom);
    m_focus->async_seek(fPoints.at(currentFixedPointId).mfocus);
    m_slider->async_seek(fPoints.at(currentFixedPointId).slider);
/*
    if(cam != nullptr) {
        cam->focusSetValue(fPoints.at(currentFixedPointId).focus);
        std::cout << "fp focus value: " << fPoints.at(currentFixedPointId).focus << "\r\n" << std::flush;
    }
*/
}

void bControl::addFixedPoint(std::string id, fixedPoint fp) {
    fp.timelapseMember = 0;

//FIXME: test it
    qDebug() << "looking for point" << QString::fromStdString(id);
    if(fPoints.find(id) == fPoints.end()) {
        fPoints.insert(std::make_pair(id, fp));
        qDebug() << "not found";
    }
    else {
        fPoints.at(id) = fp;
        qDebug() << "found";
    }

    updateFixedPointsVector();
}

void bControl::updateFixedPointsVector() {
    fPointsVector.clear();
    
    for (std::map<std::string, fixedPoint>::iterator it=fPoints.begin(); it!=fPoints.end(); ++it) {
        fPointsVector.push_back(it->second);
    }

    std::sort (fPointsVector.begin(), fPointsVector.end(), [this](fixedPoint p1, fixedPoint p2) { return p1.pan < p2.pan; });
}

void bControl::onNewCall(tHash in){
    inputCallback(in);
}

void bControl::inputCallback(tHash in){
    std::string dev = in.dev;
    std::string key = in.key;
    std::string value = in.value;
    std::vector<std::string> params;
    params.resize(10);
    params = in.params;
    
//    std::cout << dev << " --- "<< key << " --- " << value << " ---- " << params[0] << "\n" << std::flush;
//    std::cout << "hurah!\n" << std::flush;
    
    if(dev == "motor_pan") {
        if(key == "set_position")m_pan->async_seek(std::stoi(value));
        if(key == "set_null")m_pan->setNull();
        if(key == "set_center")m_pan->setCenter();
        if(key == "set_speed")m_pan->async_rotate(std::stoi(value));
        if(key == "flip_reverse")m_pan->flipReverse();
        
        if(key == "set_limit")m_pan->setRangeMax(m_pan->getPosition());
        if(key == "reset_limit")m_pan->resetRangeMax();

        if(key == "toggle_power")m_pan->togglePower();
    }
    if(dev == "motor_tilt") {
        if(key == "set_position")m_tilt->async_seek(std::stoi(value));
        if(key == "set_null")m_tilt->setNull();
        if(key == "set_center")m_tilt->setCenter();
        if(key == "set_speed")m_tilt->async_rotate(std::stoi(value));
        if(key == "flip_reverse")m_tilt->flipReverse();
        
        if(key == "set_limit")m_tilt->setRangeMax(m_tilt->getPosition());
        if(key == "reset_limit")m_tilt->resetRangeMax();

        if(key == "toggle_power")m_tilt->togglePower();
    }
    if(dev == "motor_zoom") {
        if(key == "set_position")m_zoom->async_seek(std::stoi(value));
        if(key == "set_null")m_zoom->setNull();
        if(key == "set_center")m_zoom->setCenter();
        if(key == "set_speed")m_zoom->async_rotate(std::stoi(value));
        if(key == "flip_reverse")m_zoom->flipReverse();
        
        if(key == "set_limit")m_zoom->setRangeMax(m_zoom->getPosition());
        if(key == "reset_limit")m_zoom->resetRangeMax();

        if(key == "toggle_power")m_zoom->togglePower();
    }
    if(dev == "motor_focus") {
        if(key == "set_position")m_focus->async_seek(std::stoi(value));
        if(key == "set_null")m_focus->setNull();
        if(key == "set_center")m_focus->setCenter();
        if(key == "set_speed")m_focus->async_rotate(std::stoi(value));
        if(key == "flip_reverse")m_focus->flipReverse();
        
        if(key == "set_limit")m_focus->setRangeMax(m_focus->getPosition());
        if(key == "reset_limit")m_focus->resetRangeMax();

        if(key == "toggle_power")m_focus->togglePower();
    }
    if(dev == "motor_slider") {
        if(key == "set_position")m_slider->async_seek(std::stoi(value));
        if(key == "set_null")m_slider->setNull();
        if(key == "set_center")m_slider->setCenter();
        if(key == "set_speed")m_slider->async_rotate(std::stoi(value));
        if(key == "flip_reverse")m_slider->flipReverse();
        
        if(key == "set_limit")m_slider->setRangeMax(m_zoom->getPosition());
        if(key == "reset_limit")m_slider->resetRangeMax();

        if(key == "toggle_power")m_slider->togglePower();
    }
    
    if(dev == "fixed_point") {
        if(key == "set_current"){
            fixedPoint fp;
            
            fp.pan = m_pan->getPosition();
            fp.tilt = m_tilt->getPosition();
            fp.zoom = m_zoom->getPosition();
            fp.mfocus = m_focus->getPosition();
            fp.slider = m_slider->getPosition();
            
            if(cam != nullptr)fp.focus = cam->getFocusValue();
            
            std::string id = "fp" + std::to_string(fpCounter++);
/*
            if(params[0] != "" )
                fp.name = params[0];
            else
                fp.name = id;
*/
            fp.id = id;
            if(value != "" ) {
                fp.name = value;
                fp.id = value;
            }
            else if (params[0] != "") {
                fp.name = params[0];
                fp.id = params[0];
            }
            else
                fp.name = id;

            addFixedPoint(fp.id, fp);
        }
        if(key == "select") {
            selectFixedPoint(value);
        }
        if(key == "remove") {
            removeFixedPoint(value);
        }
        if(key == "toggle_timelapse") {
            toggleTimelapseFixedPoint(value);
        }
    }

    if(dev == "viewfinder_cam" && key == "toggle") {
        cam->toggleViewfinderCam();

        if(cam->getLvActive())liveViewOn=1;
        if(cam->getVfActive())viewfinderCamOn=1;
    }

    if(dev == "live_view" && key == "toggle") {
        cam->toggleLiveView();

        if(cam->getLvActive())liveViewOn=1;
        if(cam->getVfActive())viewfinderCamOn=1;
    }
    
    if(dev == "live_view" && key == "set_zoom") {
        cam->setConfig("d1a3", value, 3);
    }
    
    if(dev == "focus" && key == "step") {
        if( value=="up" ) {
            cam->focusUp();
        }
        else if( value=="down" ) {
            cam->focusDown();
        }
        else if( value=="up_much" ) {
            cam->focusUpMuch();
        }
        else if( value=="down_much" ) {
            cam->focusDownMuch();
        }
    }
    else if(dev == "focus" && key == "set_value") {
        cam->focusSetValue(std::stoi(value));
    }
    
    if(dev == "shutter" && key == "capture") {
        cam->captureGPIO();
    }
    if(dev == "timelapse" && key == "toggle") {
       if(tl->isRunning())doStopTimelapseNew();
               else {
                   if(value == "demo")doRunTimelapseNew(true);
                   else doRunTimelapseNew(false);
               }
    }
    if(dev == "timelapse"){
        if (key == "set_delay") {
            tlSettings.tlDelay = std::stoi(value);
        }
        if (key == "set_direction") {
            tlSettings.tlDirection = std::stoi(value);
        }
        if (key == "set_frames") {
            tlSettings.tlFrames = std::stoi(value);
        }
        if (key == "set_shutterdelay") {
            tlSettings.tlShutterdelay = std::stoi(value);
        }
        if (key == "set_mirrorup") {
            tlSettings.tlMirrorup = std::stoi(value);
        }
    }
    if(dev == "cam"){
        //FIXME
        
        if(key == "refresh")cam->updateCamInfo();

        if(key == "set_d")cam->setCamParamD(value);
        if(key == "set_s")cam->setCamParamS(value);
        if(key == "set_af")cam->setCamParamAf(value);
        if(key == "set_mode")cam->setCamParamMode(value);
        if(key == "set_iso")cam->setCamParamIso(value);
        if(key == "set_exp")cam->setCamParamExp(value);
        if(key == "set_focusmetermode")cam->setCamParamFocusmetermode(value);
        else if(key == "set_autofocusarea") {
            cam->setAutofocusarea(value);
        }
        
        if(key == "get_files"){
            if(value != "") {
                camFlushTerminator = false;
                cam->flushCamera(std::stoi(value), &camFlushTerminator);
            }
            else if (value != "stop"){
                camFlushTerminator = true;
            }
            else {
                camFlushTerminator = false;
                cam->flushCamera();
            }
        }
        if(key == "af_trigger"){
            if(value == "1")cam->triggerAf(true);
            else cam->triggerAf(false);
        }
        if(key == "sr_trigger"){
            if(value == "1")cam->triggerSr(true);
            else cam->triggerSr(false);
        }
        if(key == "usb_power"){
            if(value == "1")cam->setUsbPower(1);
            else cam->setUsbPower(0);
        }
    }
}

//make TL control thread
void bControl::doRunTimelapse(bool demo) {
    /*
    if(timelapseIsRunning)return;
    
    tlThreadTerminator = false;
    std::cout << "starting TL thread\n" << std::flush;
    tlThread = new std::thread( &bControl::runLapseQbic, this, tlSettings.tlFrames, tlSettings.tlDelay, demo );  //p1, p2, p3, frames, secs, demo
    */
}

void bControl::doStopTimelapse() {
    /*
    if(!timelapseIsRunning)return;
    
    std::cout << "stopping TL thread\n" << std::flush;
    tlThreadTerminator = true;
    tlThread->join();
    std::cout << "stopped TL thread\n" << std::flush;
    */
}

void bControl::doRunTimelapseNew(bool demo) {
    /*
    if(tl->isRunning())return;

    limits l;

    fixedPointsVector tlPoints;
    std::copy_if(fPointsVector.begin(), fPointsVector.end(), std::back_inserter(tlPoints), [this](fixedPoint fp) { return fp.timelapseMember == 1; } );

    if(tlPoints.size() < 2) {
        timelapseIsRunning = false;
        return;
    }

    std::sort (tlPoints.begin(), tlPoints.end(), [this](fixedPoint p1, fixedPoint p2) { return p1.slider < p2.slider; });

    l.minSlider = tlPoints[0].slider;
    l.maxSlider = tlPoints[1].slider;
    l.minPan = tlPoints[0].pan;
    l.maxPan = tlPoints[1].pan;
    l.minTilt = tlPoints[0].tilt;
    l.maxTilt = tlPoints[1].tilt;
    l.minZoom = tlPoints[0].zoom;
    l.maxZoom = tlPoints[1].zoom;

    std::cout << "starting TL\n" << std::flush;
    tl->runLinear(l, tlSettings.tlFrames, tlSettings.tlDelay);
    */
}

void bControl::doStopTimelapseNew(void) {
    if(!tl->isRunning())return;

    std::cout << "stopping TL\n" << std::flush;
    tl->cancelShooting();
}

unsigned int bControl::timelapseStatus(void) {
    if (tl->isRunning()) return 1;
    return 0;
}

bool bControl::cmpFpsByPan(fixedPoint p1, fixedPoint p2) {
    return(p1.pan < p2.pan);
}

void bControl::runLapseQbic(unsigned int frames, unsigned int secs, bool demo){
/*
    if(timelapseIsRunning)return;
    timelapseIsRunning = true;
    
    fixedPointsVector tlPoints;
    std::copy_if(fPointsVector.begin(), fPointsVector.end(), std::back_inserter(tlPoints), [this](fixedPoint fp) { return fp.timelapseMember == 1; } );
    
    if(tlPoints.size() < 3) {
        timelapseIsRunning = false;
        return;
    }
    
    std::sort (tlPoints.begin(), tlPoints.end(), [this](fixedPoint p1, fixedPoint p2) { return p1.pan < p2.pan; });
//    std::sort (fps.begin(), fps.end(), bControl::cmpFpsByPan);

    double x1 = tlPoints[0].pan;
    double x2 = tlPoints[1].pan;
    double x3 = tlPoints[2].pan;
    
    double y1 = tlPoints[0].tilt;
    double y2 = tlPoints[1].tilt;
    double y3 = tlPoints[2].tilt;
    
    //calculate square function
    double ca = ((y3 - y1)*(x2 - x1) - (y2 - y1)*(x3 - x1)) / ((x3*x3 - x1*x1)*(x2 - x1) - (x2*x2 - x1*x1)*(x3 - x1));
    double cb = (y2 - y1 - ca*(x2*x2 - x1*x1)) / (x2 - x1);
    double cc = y1 - (ca*x1*x1 + cb*x1);
    
    std::cout << "parabolic func coeffs: " << ca << ":" << cb << ":" << cc << "\r\n";
    
    double stepSize = (x3 - x1) / frames;
    int stepDuration = secs;
    int stepsLeft = frames;

    double s_x = x1;
    double s_y = y1;
    short rev = 1;
    if(tlSettings.tlDirection == 0){
        s_x = x3;
        s_y = y3;
        rev = -1;
    }

    std::thread tp_s( &stepperSpi::async_seek_blocking, m_pan, s_x );
    std::thread tt_s( &stepperSpi::async_seek_blocking, m_tilt, s_y );
//    std::thread tt_s( &stepperSpi::async_seek_blocking, m_tilt, s_y );
    tp_s.join();
    tt_s.join();
    
    timespec tsb, tsa;
    
    while(stepsLeft--) {
        if(tlThreadTerminator)break;
        clock_gettime(CLOCK_REALTIME, &tsb);
        unsigned long tbefore = tsb.tv_sec * 1000000 + tsb.tv_nsec/1000;
        
        if(!demo)cam->captureGPIO();
        
        if(!demo)usleep((tlSettings.tlMirrorup + tlSettings.tlShutterdelay) * 1000 * 1000);

        double newPan = m_pan->getPosition() + stepSize*rev;
        double newTilt = ca*newPan*newPan + cb*newPan + cc;
        
        std::cout << "new lapse pos - " << newPan << ":" << newTilt << "\r\n";
        
        std::ostringstream strs;
        strs << "TL: " << frames-stepsLeft << "/" << frames << "," << std::round(newPan)  << "x" << std::round(newTilt);
        
        //run 2 motors simultaneously
        std::thread tp( &stepperSpi::async_seek_blocking, m_pan, std::round(newPan) );
        std::thread tt( &stepperSpi::async_seek_blocking, m_tilt, std::round(newTilt) );
//        std::thread tt( &stepperSpi::async_seek_blocking, m_tilt, std::round(newTilt) );
        tp.join();
        tt.join();
        
        if(!demo)usleep(tlSettings.tlStabdelay * 1000 * 1000);
        
        clock_gettime(CLOCK_REALTIME, &tsa);
        unsigned long tafter = tsa.tv_sec * 1000000 + tsa.tv_nsec/1000;

        double leftToWait = ((stepDuration * 1000000) - (tafter-tbefore));
        if(leftToWait > 0) {
            std::cout << "tafter: " << tafter << ", tbefore: " << tbefore << ", spent: " << (tafter-tbefore)/1000 << ", left to wait: " << leftToWait << "\r\n";
            if(!demo)usleep(leftToWait);
        }
        
        tlSettings.tlProgress = std::round(((1.0*frames - 1.0*stepsLeft)/frames) * 1000);
    }
    
    timelapseIsRunning = false;
    std::cout << "terminating TL\n" << std::flush;

*/
}

bool bControl::senderTiny() {
//    qDebug()  << "senderTiny";
    tHash r;
    
    r.dev = "motor_pan";
    r.key = "status_position";
    r.value = std::to_string(m_pan->getPosition());
    emit(dataReady(r));

    r.key = "power_status";
    r.value = std::to_string(m_pan->getPosition());
    emit(dataReady(r));
    
    r.dev = "motor_tilt";
    r.key = "status_position";
    r.value = std::to_string(m_tilt->getPosition());
    emit(dataReady(r));
    
    r.dev = "motor_zoom";
    r.key = "status_position";
    r.value = std::to_string(m_zoom->getPosition());
    emit(dataReady(r));
    
    r.dev = "motor_focus";
    r.key = "status_position";
    r.value = std::to_string(m_focus->getPosition());
    emit(dataReady(r));
    
    r.dev = "motor_slider";
    r.key = "status_position";
    r.value = std::to_string(m_slider->getPosition());
    emit(dataReady(r));
    
    return true;
}

bool bControl::senderHeavy() {
    tHash r1;
    r1.dev = "fixed_point";
    r1.key = "clear";
    r1.value = "";
     emit(dataReady(r1));
    
    for (fixedPointsVector::iterator it=fPointsVector.begin(); it!=fPointsVector.end(); ++it) {
        std::vector<std::string> p;
        
        tHash r;
        r.dev = "fixed_point";
        r.key = "get";
        r.value = it->id;
        
        p.push_back(it->name);
        p.push_back(std::to_string(it->timelapseMember));
        p.push_back(std::to_string(it->pan));
        p.push_back(std::to_string(it->tilt));
        p.push_back(std::to_string(it->zoom));
        p.push_back(std::to_string(it->mfocus));
        p.push_back(std::to_string(it->slider));
        
        r.params = p;
        
        emit(dataReady(r));
    }
    
    r1.dev = "fixed_point";
    r1.key = "refresh";
    r1.value = "";
    emit(dataReady(r1));

    r1.dev = "live_view";
    r1.key = "status";
    if(cam->getLvActive())r1.value = "on";
        else r1.value = "off";
    emit(dataReady(r1));

    r1.dev = "viewfinder_cam";
    r1.key = "status";
    if(cam->getVfActive())r1.value = "on";
        else r1.value = "off";
    emit(dataReady(r1));
    
    if(fPoints.find(currentFixedPointId) != fPoints.end()) {
        r1.dev = "fixed_point";
        r1.key = "select";
        r1.value = currentFixedPointId;
        emit(dataReady(r1));
    }

    r1.dev = "timelapse";
    r1.key = "status";
    r1.value = std::to_string(timelapseStatus());
    emit(dataReady(r1));
    
    r1.dev = "timelapse";
    r1.key = "progress";
    r1.value = std::to_string(tlSettings.tlProgress);
    emit(dataReady(r1));
    
    if(cam != NULL) {
        camInfoType ci = cam->getCamInfo();
        
        r1.dev = "cam";
        r1.key = "current_mode";
        r1.value = ci.mode;
        emit(dataReady(r1));
        
        r1.dev = "cam";
        r1.key = "current_iso";
        r1.value = ci.iso;
        emit(dataReady(r1));
        
        r1.dev = "cam";
        r1.key = "current_d";
        r1.value = ci.d;
        emit(dataReady(r1));
        
        r1.dev = "cam";
        r1.key = "current_s";
        r1.value = ci.s;
        emit(dataReady(r1));
        
        r1.dev = "cam";
        r1.key = "current_exp";
        r1.value = ci.exp;
        emit(dataReady(r1));
        
        r1.dev = "cam";
        r1.key = "autofocusarea";
        r1.value = ci.autofocusarea;
        emit(dataReady(r1));

        r1.dev = "cam";
        r1.key = "lightmeter";
        r1.value = ci.lightmeter;
        emit(dataReady(r1));
        
        r1.dev = "focus";
        r1.key = "current_value";
        r1.value = std::to_string(cam->getFocusValue());
        emit(dataReady(r1));

        r1.dev = "cam";
        r1.key = "usb_power";
        r1.value = std::to_string(cam->getUsbPower());
        emit(dataReady(r1));

        r1.dev = "cam";
        r1.key = "focusmetermode";
        r1.value = ci.focusmetermode;
        emit(dataReady(r1));
        
        r1.dev = "cam";
        r1.key = "current_af";
 //       r1.value =
        //        bs->send(r1);
    }
    
    r1.dev = "motors";
    r1.key = "info";
    r1.value = "";
    std::vector<std::string> p1;
    p1.push_back(std::to_string(m_pan->getRangeMin()));
    p1.push_back(std::to_string(m_pan->getRangeMax()));
    p1.push_back(std::to_string(m_tilt->getRangeMin()));
    p1.push_back(std::to_string(m_tilt->getRangeMax()));
    p1.push_back(std::to_string(m_zoom->getRangeMin()));
    p1.push_back(std::to_string(m_zoom->getRangeMax()));
    p1.push_back(std::to_string(m_focus->getRangeMin()));
    p1.push_back(std::to_string(m_focus->getRangeMax()));
    p1.push_back(std::to_string(m_slider->getRangeMin()));
    p1.push_back(std::to_string(m_slider->getRangeMax()));

    p1.push_back(std::to_string(m_pan->getPowerStatus()));
    p1.push_back(std::to_string(m_tilt->getPowerStatus()));
    p1.push_back(std::to_string(m_zoom->getPowerStatus()));
    p1.push_back(std::to_string(m_focus->getPowerStatus()));
    p1.push_back(std::to_string(m_slider->getPowerStatus()));

    p1.push_back(std::to_string(m_pan->getReverseDir()));
    p1.push_back(std::to_string(m_tilt->getReverseDir()));
    p1.push_back(std::to_string(m_zoom->getReverseDir()));
    p1.push_back(std::to_string(m_focus->getReverseDir()));
    p1.push_back(std::to_string(m_slider->getReverseDir()));

    r1.params = p1;
    emit(dataReady(r1));
    
    p1 = std::vector<std::string>();
    
      return true;
}

void bControl::onLogToClient(QString str) {
    tHash r1;

    r1.dev = "log";
    r1.key = "string";
    r1.value = str.toStdString();

    emit(dataReady(r1));
}

void bControl::onPWSignal(int num) {
    qDebug() << "FP by PW: " <<  num;
    std::string fpid = "";
    switch (num) {
        case 5	: {
            if(fPointsVector.size() < 1)return;
            fpid = fPointsVector.at(0).id;
            break;
        }
        case 6	: {
            if(fPointsVector.size() < 2)return;
            fpid = fPointsVector.at(1).id;
            break;
        }
        case 7	: {
            if(fPointsVector.size() < 3)return;
            fpid = fPointsVector.at(2).id;
            break;
        }
        case 8	: {
            if(fPointsVector.size() < 4)return;
            fpid = fPointsVector.at(3).id;
            break;
        }
        case 9	: {
            if(fPointsVector.size() < 5)return;
            fpid = fPointsVector.at(4).id;
            break;
        }
        case 10	: {
            if(fPointsVector.size() < 6)return;
            fpid = fPointsVector.at(5).id;
            break;
        }
        default: {
            return;
        }
    }
    if(fpid == currentFixedPointId) {
        cam->triggerAf(true, 0.4);
        cam->triggerSr(true, 0.4);
        return;
    }
    selectFixedPoint(fpid);
}
