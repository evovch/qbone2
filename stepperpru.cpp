//
//  stepper.cpp
//  bone
//
//  Created by korytov on 13/04/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#include "stepperpru.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <QDebug>

#define pruDramPositionOffset 0
#define pruDramMaxDelayOffset 4
#define pruDramMinDelayOffset 8
#define pruDramAccelDeltaOffset 12
#define pruDramDirectionOffset 16
#define pruDramTargetOffset 20
#define pruDramAccelLengthOffset 24
#define pruDramRequiredDirectionOffset 28
#define pruDramAccelDistanceOffset 32
#define pruDramSetPositionOffset 40

#define pruRDramSubOffset 50 
#define pruDramAccelTableSubOffset 100

#define RDRAM_POSITION_M1 0
#define RDRAM_MAXDELAY_M1 4
#define RDRAM_MINDELAY_M1 8
#define RDRAM_ACCELDELTA_M1 12
#define RDRAM_ACCELDISTANCE_M1 16
#define RDRAM_TARGET_M1 20
#define RDRAM_TICKSTOSTEP_M1 24
#define RDRAM_CURRENTDELAY_M1 28
#define RDRAM_PINOFFSSET_M1 32
#define RDRAM_DIRECTION_M1 36
#define RDRAM_ACCELLENGTH_M1 40
#define RDRAM_REQUIREDDIRECTION_M1 44

#define motorDRAMOffset_M1 0;
#define motorDRAMOffset_M2 1500;
#define motorDRAMOffset_M3 3000;
#define motorDRAMOffset_M4 4500;
//#define motorDRAMOffset_M5 6000;

void stepperPru::dumpPruDram(){
    std::cout << "PRU DRAM dump:\n";
    std::cout << "pruDramPositionOffset:" << getPruDram(pruDramPositionOffset) << "\r\n";
    std::cout << "pruDramMaxDelayOffset:" << getPruDram(pruDramMaxDelayOffset) << "\r\n";
    std::cout << "pruDramMinDelayOffset:" << getPruDram(pruDramMinDelayOffset) << "\r\n";
    std::cout << "pruDramAccelDeltaOffset:" << getPruDram(pruDramAccelDeltaOffset) << "\r\n";
    std::cout << "pruDramDirectionOffset:" << getPruDram(pruDramPositionOffset) << "\r\n";
    std::cout << "pruDramAccelLengthOffset:" << getPruDram(pruDramTargetOffset) << "\r\n";
    std::cout << "pruDramPositionOffset:" << getPruDram(pruDramAccelLengthOffset) << "\r\n";
    std::cout << "pruDramRequiredDirectionOffset:" << getPruDram(pruDramRequiredDirectionOffset) << "\r\n";

    std::cout << "PRU RDRAM dump:\n";
    std::cout << "RDRAM_POSITION_M1:" << getPruDram(pruRDramSubOffset + RDRAM_POSITION_M1) << "\r\n";
    std::cout << "RDRAM_MAXDELAY_M1:" << getPruDram(pruRDramSubOffset + RDRAM_MAXDELAY_M1) << "\r\n";
    std::cout << "RDRAM_MINDELAY_M1:" << getPruDram(pruRDramSubOffset + RDRAM_MINDELAY_M1) << "\r\n";
    std::cout << "RDRAM_ACCELDELTA_M1:" << getPruDram(pruRDramSubOffset + RDRAM_ACCELDELTA_M1) << "\r\n";
    std::cout << "RDRAM_ACCELDISTANCE_M1:" << getPruDram(pruRDramSubOffset + RDRAM_ACCELDISTANCE_M1) << "\r\n";
    std::cout << "RDRAM_TARGET_M1:" << getPruDram(pruRDramSubOffset + RDRAM_TARGET_M1) << "\r\n";
    std::cout << "RDRAM_TICKSTOSTEP_M1:" << getPruDram(pruRDramSubOffset + RDRAM_TICKSTOSTEP_M1) << "\r\n";
    std::cout << "RDRAM_CURRENTDELAY_M1:" << getPruDram(pruRDramSubOffset + RDRAM_CURRENTDELAY_M1) << "\r\n";
    std::cout << "RDRAM_PINOFFSSET_M1:" << getPruDram(pruRDramSubOffset + RDRAM_PINOFFSSET_M1) << "\r\n";
    std::cout << "RDRAM_DIRECTION_M1:" << getPruDram(pruRDramSubOffset + RDRAM_DIRECTION_M1) << "\r\n";
    std::cout << "RDRAM_ACCELLENGTH_M1:" << getPruDram(pruRDramSubOffset + RDRAM_ACCELLENGTH_M1) << "\r\n";
    std::cout << "RDRAM_REQUIREDDIRECTION_M1:" << getPruDram(pruRDramSubOffset + RDRAM_REQUIREDDIRECTION_M1) << "\r\n";
    
    std::cout << std::flush;
}

unsigned int stepperPru::getCurrentDelay(){
    return getPruDram(pruRDramSubOffset + RDRAM_CURRENTDELAY_M1);
}

void stepperPru::pollRequiredDirection(){
    unsigned int rd;
    
    while(!terminate) {
        usleep(100000);
        rd = getRequiredDirection();
//        std::cout << "pollind required direction:" << rd << "\r\n" << std::flush;
        setDirection(rd);
    }
}

stepperPru::stepperPru(int mNum, unsigned char* pruDataMem_byte, unsigned int accelLength, unsigned int dirPinGrp, unsigned int n_dirPin, bool rev, unsigned int speedFactor) {
    unsigned int mOffset = 0;
    if(mNum == 1)mOffset=motorDRAMOffset_M1;
    if(mNum == 2)mOffset=motorDRAMOffset_M2;
    if(mNum == 3)mOffset=motorDRAMOffset_M3;
    if(mNum == 4)mOffset=motorDRAMOffset_M4;
//    if(mNum == 5)mOffset=motorDRAMOffset_M5;
    pruDataMem_byte += mOffset;
    pruDramStart = pruDataMem_byte;
    
    reversed = rev;
    
    dirPin = new gpioInt(n_dirPin);
    dirPin->setDir(1); // 1 = out
    
    int position = 0;
    int accelDelta = 1500;
    int maxDelay = 240000;
    accelLength = 300;

    maxAccelLength = accelLength;
    setPruDram(pruDramMaxDelayOffset, maxDelay);
    setPruDram(pruDramAccelDeltaOffset, accelDelta);
    setPruDram(pruDramAccelLengthOffset, maxAccelLength);
    
    //upload accel table
    double currentDelay = maxDelay;
    long now = 0;
    for(int i = 0; i <= accelLength; i++) {
        double divider = 9.0 - ((double)i/accelLength)*5;
//        std::cout <<  divider << "\r\n";
        
        setPruDram(pruDramAccelTableSubOffset + i*4, (int)((currentDelay/divider)/speedFactor));

        now += currentDelay;
        std::cout <<  now << ";" << (int)(currentDelay/divider) << "\r\n";
        currentDelay = (maxDelay * (exp(-0.0000008*now)));
        
//        currentDelay = maxDelay - 0.03*now;
        
//        currentDelay = currentDelay - (accelDelta*(currentDelay/maxDelay));
    }
    
    c_pos = 0;
    c_pos = getPruDram(pruDramPositionOffset);
    
    rangeMin = 0;
    rangeMax = absoluteRangeMax;
//    async_seek(0); //set default target
    setDirection(1);
    
    terminate = false;
    mt = new std::thread( &stepperPru::pollRequiredDirection, this );
//dumpPruDram();
}

void stepperPru::setPruDram(unsigned int offset, unsigned int val) {
//    std::cout << (int)pruDramStart << ":" << (int)offset << "\n" << std::flush;
    memcpy(pruDramStart + offset, &val, 4);
}

int stepperPru::getPruDram(unsigned int offset) {
//    std::cout << (int)pruDramStart << ":" << (int)offset << "\n" << std::flush;
    int ret;
    memcpy(&ret, pruDramStart + offset, 4);
    
    return ret;
}

void stepperPru::setRange(unsigned int min, unsigned int max) {
    rangeMin = min;
    rangeMax = max;
}

void stepperPru::resetRangeMax() {
    rangeMax = absoluteRangeMax;
}

void stepperPru::setRangeMin(unsigned int n) {
    rangeMin = n;
}
void stepperPru::setRangeMax(unsigned int n) {
    rangeMax = n;
}

void stepperPru::setDirection(unsigned int dir) {
    int rdir = 0;
    int fdir = 1;
    if(reversed){
        rdir = 1;
        fdir = 0;
    }
    
    if(dir == 0 ) {
        dirPin->setValue(rdir);
        setPruDram(pruDramDirectionOffset, 0);
        return;
    }
    if(dir == 1) {
        dirPin->setValue(fdir);
        setPruDram(pruDramDirectionOffset, 1);
        return;
    }
}

void stepperPru::async_seek(unsigned int pos) {
    if(pos > rangeMax)pos = rangeMax;
    if(pos < rangeMin)pos = rangeMin;
    setPruDram(pruDramTargetOffset, pos);
}

void stepperPru::async_seek_blocking(unsigned int pos) {
    while(pos != getPosition()) {
        async_seek(pos);
        usleep(0.1 * 1000 * 1000);
    }
    usleep(0.1 * 1000 * 1000);
}

void stepperPru::async_rotate(int dir) {
    if(dir==0) {
        setPruDram(pruDramAccelLengthOffset, maxAccelLength);
        setPruDram(pruDramTargetOffset, 999999);
    }
    if(dir > 0) {
        setPruDram(pruDramAccelLengthOffset, maxAccelLength*((float)dir/300));
        setPruDram(pruDramTargetOffset, rangeMax);
    }
    if(dir < 0) {
        setPruDram(pruDramAccelLengthOffset, maxAccelLength*((-1*(float)dir)/300));
        setPruDram(pruDramTargetOffset, rangeMin);
    }
}

unsigned int stepperPru::getPosition() {
    c_pos = getPruDram(pruDramPositionOffset);
    return c_pos;
}

void stepperPru::setAccelLength(unsigned int val) {
    setPruDram(pruDramAccelLengthOffset, val);
}

unsigned int stepperPru::getAccelLength(){
    return getPruDram(pruDramAccelLengthOffset);
}

unsigned int stepperPru::getRequiredDirection(){
    return getPruDram(pruDramRequiredDirectionOffset);
}

void stepperPru::setCenter(){
    std::cout << "setting center\r\n";
    setPruDram(pruDramSetPositionOffset, rangeMax/2);
//    setPruDram(pruDramSetPositionOffset, 999999);
    setPruDram(pruDramTargetOffset, rangeMax/2);
}

void stepperPru::setNull(){
    std::cout << "setting null\r\n";
    setPruDram(pruDramSetPositionOffset, 0);
//    setPruDram(pruDramSetPositionOffset, 999999);
    setPruDram(pruDramTargetOffset, 0);
}

void stepperPru::flipReverse(){
    async_seek(getPosition());
    usleep(0.5 * 1000 * 1000);
    reversed = !reversed;
}

void stepperPru::pulse(int distance){
    uint newPos = getPosition() + distance;
    async_seek(newPos);
}

void stepperPru::togglePower(){
    //not supported
}

int stepperPru::getPowerStatus(){
    //not supported
}
