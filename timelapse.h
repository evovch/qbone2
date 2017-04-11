#ifndef TIMELAPSE_H
#define TIMELAPSE_H

#include <QObject>

#include "btypes.h" // for 'limits' type

class QTimer;
class bCamera;
class stepperSpi;

class Timelapse : public QObject
{
    Q_OBJECT

public:
    explicit Timelapse(QObject *parent = 0);

    bool isRunning() {return tlShooting;}

    void setCamera(bCamera *cam);
    void runLinear(limits l, uint frames, uint realtimeDelaySecs);
    void cancelShooting();
    void setMotors(stepperSpi* slider, stepperSpi* pan, stepperSpi* tilt, stepperSpi* zoom);

private:
    bCamera *camera; //TODO singleton? If yes - use standard singleton template!
    unsigned int shotCount;

    QTimer *timer;

    stepperSpi *motorPan;
    stepperSpi *motorTilt;
    stepperSpi *motorZoom;
    stepperSpi *motorSlider;

    unsigned int framesCount; //TODO better use signed/unsigned specificator

    int snapSlider;
    int snapPan;
    int snapTilt;
    int snapZoom;

    bool tlShooting;

signals:

public slots:
    void shot(void);

};

#endif // TIMELAPSE_H
