#ifndef TIMELAPSE_H
#define TIMELAPSE_H

#include <QObject>
#include "bcamera.h"
//#include "stepperpru.h"
#include "stepperspi.h"

class Timelapse : public QObject
{
    Q_OBJECT
private:
    bCamera *camera;
    uint shotCount;

    QTimer *timer;

//    stepperPru *motorSlider;
//    stepperPru *motorPan;
//    stepperPru *motorTilt;
//    stepperPru *motorZoom;

    stepperSpi *motorPan;
    stepperSpi *motorTilt;
    stepperSpi *motorZoom;
    stepperSpi *motorSlider;

    int framesCount;

    int snapSlider;
    int snapPan;
    int snapTilt;
    int snapZoom;

    bool tlShooting;

public:
    explicit Timelapse(QObject *parent = 0);

    bool isRunning() {return tlShooting;}

    void setCamera(bCamera *cam);
    void runLinear(limits l, uint frames, uint realtimeDelaySecs);
    void cancelShooting();
//    void setMotors(stepperPru* slider, stepperPru* pan, stepperPru* tilt, stepperPru* zoom);
    void setMotors(stepperSpi* slider, stepperSpi* pan, stepperSpi* tilt, stepperSpi* zoom);

signals:
    
public slots:
    void shot(void);
};

#endif // TIMELAPSE_H
