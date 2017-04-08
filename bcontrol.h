//
//  bcontrol.h
//  bone
//
//  Created by korytov on 07/05/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#ifndef __bone__bcontrol__
#define __bone__bcontrol__

#include <iostream>
#include <thread>

#include <QObject>

#include "btypes.h"


//#include "lvconn.h"
//#include "lvconnfactory.h"
#include "timelapse.h"

#include "bus_protocol/fs_spi.h"

class stepperSpi;
class PWListener;
class MainWidget;
class bCamera;

class bControl : public QObject {
    Q_OBJECT

public:
    bControl(QObject *parent = 0);

    fixedPoints getFixedPoints(void);
    int getFixedPointsSize(void);
    void addFixedPoint(std::string id, fixedPoint fp);
    void removeFixedPoint(std::string id);
    void selectFixedPoint(std::string id);

    void inputCallback(tHash in);
    void setCamera(bCamera *c);
    
    unsigned int timelapseStatus(void);
    
    void setCurrentConnectionsCount(int c);
    
private:
    stepperSpi *m_tilt, *m_pan, *m_zoom, *m_focus, *m_slider;

    MainWidget *mw;

    ISPI *spiBus0, *spiBus1, *spiBus2, *spiBus3;
    
    Timelapse *tl;

    bool camFlushTerminator;

    PWListener *pwl, *pwl2;

    bool liveViewOn, viewfinderCamOn;
    bCamera *cam;

    int currentConnectionsCount;
    
    int fpCounter;

//    std::thread *tlThread;
    bool tlThreadTerminator;
    bool timelapseIsRunning;

/*
    std::thread *bcThread;
    bool bcThreadTerminator;
 */

    fixedPoints fPoints;
    fixedPointsVector fPointsVector;
    std::string currentFixedPointId;
    
    tlParams tlSettings;
    
    bool senderTiny();
    bool senderHeavy();

    bool cmpFpsByPan(fixedPoint p1, fixedPoint p2);
    void updateFixedPointsVector();
    void toggleTimelapseFixedPoint(std::string id);
    
    void doRunTimelapse(bool demo);
    void doStopTimelapse();
    void runLapseQbic(unsigned int frames, unsigned int secs, bool demo);
    
    void doRunTimelapseNew(bool demo);
    void doStopTimelapseNew(void);

signals:
    void dataReady(tHash);

public slots:
    void onTimerTiny(void);
    void onTimerHeavy(void);
    void onNewCall(tHash in);
    void onLogToClient(QString str);
    void onPWSignal(int num);

};

#endif // __bone__bcontrol__
