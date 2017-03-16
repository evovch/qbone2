//
//  stepperpru.h
//  bone
//
//  Created by korytov on 13/04/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#ifndef __bone__stepperspi__
#define __bone__stepperspi__

#include <QObject>
#include <iostream>
#include "gpioint.h"
#include "l6470/l6470.h"
#include "stepperspiwatcher.h"
#include "btypes.h"

#endif /* defined(__bone__stepperspi__) */


class stepperSpi : public QObject {
    Q_OBJECT
    
private:
    gpioInt* dirPin;
    
    long int c_pos;

    stepperSpiWatcher *watcher;

    int rangeMin;
    int rangeMax;
    unsigned int id;
    
    const unsigned int absoluteRangeMax = 100000;

    bool terminate;

    L6470 *motor;

public:
    stepperSpi(ISPI *spiBus, unsigned int csnLine, l6470Setup ms, QObject *parent = 0);

    void setRange(unsigned int min, unsigned int max);
    void setRangeMin(unsigned int n);
    void setRangeMax(unsigned int n);
    void async_seek_core(int pos);
    void async_seek_blocking(unsigned int pos);
    void async_seek(int pos);
    void async_rotate(int dir);
    
    void resetRangeMax();
    
    int getPosition();
    
    int getRangeMin(){return rangeMin;}
    int getRangeMax(){return rangeMax;}
    int getPowerStatus();
    bool getReverseDir();
    
    void flipReverse();
    
    void setCenter();
    void setNull();

    void pulse(int distance);

    l6470Status *motorStatus;
    l6470Setup *motorSetup;
    bool reverseDir;

 signals:
    void watchLimitsRequest(int, int);

 public slots:
    void _onWatcherLimit(void);
    void _onUpdateMotorStatus(void);
    void receivePosition(long p);
    void togglePower();

};
