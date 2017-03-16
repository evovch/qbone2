//
//  stepperpru.h
//  bone
//
//  Created by korytov on 13/04/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#ifndef __bone__stepperpru__
#define __bone__stepperpru__

#include <iostream>
#include <thread>
#include "gpioint.h"

#endif /* defined(__bone__stepperpru__) */


class stepperPru {
    
private:
    gpioInt* dirPin;

    unsigned char* pruDramStart;
    
    unsigned int c_pos;

//    int maxSps;
    unsigned int rangeMin;
    unsigned int rangeMax;
    
    const unsigned int absoluteRangeMax = 25600;
    
    unsigned int maxAccelLength;
    
    std::thread *mt;
    bool terminate;
    
    bool reversed;
    
    void setDirection(unsigned int val);
    void setPruDram(unsigned int offset, unsigned int val);
    int getPruDram(unsigned int offset);
    void pollRequiredDirection();

//    int currentSps;
    
public:
//    void setCurrentSps(int sps);
//    int getCurrentSps();
//  bool get_direction(void);
    stepperPru(int mNum, unsigned char* pruDataMem_byte, unsigned int accelLength, unsigned int dirPinGrp, unsigned int n_dirPin, bool rev = false, unsigned int speedFactor = 1);

    void setRange(unsigned int min, unsigned int max);
    void setRangeMin(unsigned int n);
    void setRangeMax(unsigned int n);
    void async_seek(unsigned int pos);
    void async_seek_blocking(unsigned int pos);
    void async_rotate(int dir);

    void togglePower();
    
    void resetRangeMax();
    
    void setAccelLength(unsigned int val);
    unsigned int getAccelLength();
    unsigned getPosition();
    int getPowerStatus();

    unsigned int getRequiredDirection();
    void dumpPruDram();
    
    unsigned int getCurrentDelay();
    
    int getRangeMin(){return rangeMin;}
    int getRangeMax(){return rangeMax;}
    
    void flipReverse();
    
    void setCenter();
    void setNull();

    void pulse(int distance);
};
