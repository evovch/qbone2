#include "timelapse.h"

Timelapse::Timelapse(QObject *parent) :
    QObject(parent)
{
    shotCount = 0;
    tlShooting = false;

    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(shot()));
    timer->start(1000);
}

void Timelapse::setCamera(bCamera *cam) {
    camera = cam;
}

void Timelapse::setMotors(stepperSpi *slider, stepperSpi *pan, stepperSpi *tilt, stepperSpi *zoom) {
   motorSlider = slider;
   motorPan = pan;
   motorTilt = tilt;
   motorZoom = zoom;
}

void Timelapse::runLinear(limits l, uint fc, uint realtimeDelaySecs) {
    shotCount = 0;
    framesCount = fc;

    snapSlider = ( l.maxSlider - l.minSlider ) / framesCount;
    snapPan = ( l.maxPan - l.minPan ) / framesCount;
    snapTilt = ( l.maxTilt - l.minTilt ) / framesCount;
    snapZoom = ( l.maxZoom - l.minZoom ) / framesCount;

    qDebug() << "realtimeDelaySecs: " << realtimeDelaySecs;
    qDebug() << "snapSlider: " << snapSlider;
    qDebug() << "snapPan: " << snapPan;
    qDebug() << "snapTilt: " << snapTilt;
    qDebug() << "snapZoom: " << snapZoom;
    qDebug() << "framesCount: " << framesCount;

    if(snapSlider != 0)motorSlider->async_seek_blocking(l.minSlider);
    if(snapPan != 0)motorPan->async_seek_blocking(l.minPan);
    if(snapTilt != 0)motorTilt->async_seek_blocking(l.minTilt);
    if(snapZoom != 0)motorZoom->async_seek_blocking(l.minZoom);

    timer->start(realtimeDelaySecs * 1000);
    tlShooting = true;

    qDebug() << timer->isActive();
}

void Timelapse::cancelShooting() {
    tlShooting = false;
}

void Timelapse::shot() {
    if(!tlShooting)return;

    shotCount++;

    qDebug() << "shot!";

    camera->captureGPIO(1000);

    if(snapSlider != 0)motorSlider->pulse(snapSlider);
    if(snapPan != 0)motorPan->pulse(snapPan);
    if(snapTilt != 0)motorTilt->pulse(snapTilt);
    if(snapZoom != 0)motorZoom->pulse(snapZoom);

    if(shotCount == framesCount)cancelShooting();
}
