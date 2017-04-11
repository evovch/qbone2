//
//  stepperspi.cpp
//  bone
//
//  Created by korytov on 13/04/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#include <QDebug>

#include "stepperspi.h"

#include "gpioint.h"
#include "stepperspiwatcher.h"

// CS GPIO
// driver 1: 000
// driver 2: 100
// driver 3: 010
// driver 4: 110
// driver 5: 001

stepperSpi::stepperSpi(ISPI *spiBus, unsigned int csnLine, unsigned int resetGpioNum, l6470Setup ms, QObject* /*parent*/) {
    id = csnLine;

    resetGpio = new gpioInt(resetGpioNum);
    resetGpio->setDir(1);

    resetGpio->setValue(1);

    motor = new L6470(spiBus, csnLine);
    motorStatus = new l6470Status;
    motorSetup = new l6470Setup;
    motorSetup->max_spd = ms.max_spd;

    qDebug() << "motor init: " << csnLine;
    qDebug() << "m_stp =" << ms.m_stp << "; max_spd =" << ms.max_spd << "; mot_acc =" << ms.mot_acc << "; mot_acc =" << ms.mot_acc << ";";
    qDebug() << "kval_hold =" << ms.kval_hold << "; kval_run =" << ms.kval_run << "; int_spd =" << ms.int_spd << "; st_slp =" << ms.st_slp << "; slp_acc =" << ms.slp_acc << ";";

    motor->initMotion(ms.m_stp, ms.max_spd, ms.mot_acc, ms.mot_acc);
    motor->initBEMF(ms.kval_hold, ms.kval_run, ms.int_spd, ms.st_slp, ms.slp_acc);

    /*
    motor->setMinSpeed(0);
    motor->setParam(dSPIN_STALL_TH, 127);
    motor->setFullStepThreshold(150000);
    motor->setParam(dSPIN_CONFIG_POW_SR, dSPIN_CONFIG_SR_180V_us);
    motor->setParam(dSPIN_OCD_TH, 10); //of 15
*/

    motorStatus->powerStatus = true;

    watcher = new stepperSpiWatcher(motorStatus);

    QThread *thread = new QThread();
    watcher->moveToThread(thread);
    QObject::connect(watcher, SIGNAL(limitReached()), this, SLOT(_onWatcherLimit()), Qt::QueuedConnection);
    QObject::connect(watcher, SIGNAL(updateMotorStatus()), this, SLOT(_onUpdateMotorStatus()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(watchLimitsRequest(int,int)), watcher, SLOT(_onWatchLimitsRequest(int,int)), Qt::QueuedConnection);
    thread->start();

    motorStatus->expired = false;
    _onUpdateMotorStatus();

    qDebug() << "motor ping: " << motor->getParam(dSPIN_CONFIG);

    int st = motor->getParam(dSPIN_STATUS);
    qDebug() << "status after init: " << st;

    qDebug() << "motor position: " << getPosition();

    c_pos = 0;
    
    rangeMin = 0;
    rangeMax = 100000;
    reverseDir = false;
}

void stepperSpi::setRange(unsigned int min, unsigned int max) {
    rangeMin = min;
    rangeMax = max;
}

void stepperSpi::resetRangeMax() {
    rangeMax = absoluteRangeMax;
}

void stepperSpi::setRangeMin(unsigned int n) {
    rangeMin = n;
}
void stepperSpi::setRangeMax(unsigned int n) {
    rangeMax = n;
}

void stepperSpi::_onUpdateMotorStatus() {
    int st = motor->getParam(dSPIN_STATUS);
//    motor->getStatus();


//    qDebug() << "Status for ID: "  << id;
//    qDebug() << "Error: "  << motor->getError();

    if ((st & dSPIN_STATUS_NOTPERF_CMD))qDebug() << "dSPIN_ERR_NOEXEC";
    if ((st & dSPIN_STATUS_WRONG_CMD))qDebug() << "dSPIN_ERR_BADCMD";
    if ((st & dSPIN_STATUS_UVLO) == 0)qDebug() << "dSPIN_ERR_UVLO";
    if ((st & dSPIN_STATUS_TH_SD) == 0)qDebug() << "dSPIN_ERR_THSHTD";
    if ((st & dSPIN_STATUS_OCD) == 0)qDebug() << "dSPIN_ERR_OVERC";
    if ((st & dSPIN_STATUS_STEP_LOSS_A) == 0)qDebug() << "dSPIN_ERR_STALLA";
    if ((st & dSPIN_STATUS_STEP_LOSS_B) == 0)qDebug() << "dSPIN_ERR_STALLB";

    motorStatus->position = motor->getPosition();

//    qDebug() << "status:" << (st & dSPIN_STATUS_MOT_STATUS);
    if((st & dSPIN_STATUS_MOT_STATUS)==0) {
        motorStatus->isMoving = 0;
    }
    else {
        motorStatus->isMoving = 1;
    }

    motorStatus->powerStatus = st & dSPIN_STATUS_HIZ;
    motorStatus->stalled = (st & dSPIN_STATUS_STEP_LOSS_A) && (st & dSPIN_STATUS_STEP_LOSS_B);
/*
    qDebug() <<  "powerStatus: " << motorStatus->powerStatus;
    qDebug() <<  "position: " << motorStatus->position;
    qDebug() <<  "stalled: " << motorStatus->stalled << "\n";
*/
    motorStatus->dir = 1;
    if((st & dSPIN_STATUS_DIR) == 0)motorStatus->dir = 0;

    motorStatus->expired = false;
}

void stepperSpi::async_seek(int pos) {
    if(pos > rangeMax)pos = rangeMax;
    if(pos < rangeMin)pos = rangeMin;

    motor->softStop();
    while(motor->isBusy()) {
        usleep(0.01 * 1000 * 1000);
    }

    motor->gotoPosABS(pos);
    if(!motorStatus->nowWatched)emit watchLimitsRequest(rangeMin, rangeMax);
}

void stepperSpi::_onWatcherLimit() {
    motor->softStop();
    while(motor->isBusy()) {
        usleep(0.01 * 1000 * 1000);
    }
}

void stepperSpi::async_seek_blocking(unsigned int pos) {
    while(pos != c_pos) {
        async_seek(pos);
        usleep(0.1 * 1000 * 1000);
    }
    usleep(0.1 * 1000 * 1000);
}

void stepperSpi::async_rotate(int dir) {
    if(dir > 100)dir=100;
    else if(dir < -100)dir=-100;
    if(reverseDir)dir = -1*dir;

 //   if(motorStatus->position >= rangeMax && dir > 0)return;
 //    if(motorStatus->position <= rangeMin && dir < 0)return;

    float speed = motorSetup->max_spd * ((float)(dir/100.0));

    qDebug() << "async_rotate at speed: " << speed;

    qDebug() << "motor should run";

    if(dir >= 0)motor->run(1, speed);
    if(dir < 0)motor->run(0, speed * -1);

//    QThread::sleep(5);

//    qDebug() << "motor should run2";

    if(!motorStatus->nowWatched)emit watchLimitsRequest(rangeMin, rangeMax);
}

void stepperSpi::receivePosition(long int p) {
    c_pos = p;
}

int stepperSpi::getPosition() {
    return motorStatus->position;
}

void stepperSpi::setCenter(){
    motor->setPosition(((rangeMax-rangeMin) / 2) + rangeMin);
    if(!motorStatus->nowWatched)emit watchLimitsRequest(rangeMin, rangeMax);
}

void stepperSpi::setNull(){
    motor->setPosition(0);
    if(!motorStatus->nowWatched)emit watchLimitsRequest(rangeMin, rangeMax);
}

void stepperSpi::flipReverse(){
    reverseDir = !reverseDir;
}

void stepperSpi::pulse(int distance){
    uint newPos = getPosition() + distance;
    async_seek(newPos);
}

void stepperSpi::togglePower(){
    if(motorStatus->powerStatus == 0) {
        motor->softHiZ();
    }
    else {
        motor->move(0);
    }
    _onUpdateMotorStatus();
}

int stepperSpi::getPowerStatus(){
    return motorStatus->powerStatus;
}

bool stepperSpi::getReverseDir(){
    return reverseDir;
}
