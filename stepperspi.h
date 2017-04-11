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

#include "btypes.h" // for l6470Setup
#include "l6470/l6470.h"

#endif // __bone__stepperspi__       //TODO why here and not at the end of the file?!

class gpioInt;
class stepperSpiWatcher;

class stepperSpi : public QObject {
    Q_OBJECT
    
public:
    stepperSpi(ISPI *spiBus, unsigned int csnLine, unsigned int resetGpioNum, l6470Setup ms, QObject *parent = 0);

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

private:
    gpioInt* dirPin;

    long int c_pos;

    stepperSpiWatcher *watcher;

    int rangeMin;
    int rangeMax;
    unsigned int id;
    gpioInt *resetGpio;

    const unsigned int absoluteRangeMax = 100000;

    bool terminate;

    L6470 *motor;

};
