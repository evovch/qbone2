#include "lvloop.h"

#include <QDebug>
#include <QTimer>

LvLoop::LvLoop(QObject *parent) :
    QObject(parent)
{
    isRunning = false;
    busy = false;
    isEnabled = false;

    lvTimer = new QTimer();
    QObject::connect(lvTimer, SIGNAL(timeout()), this, SLOT(onTimer()), Qt::DirectConnection);
    lvTimer->start(50);

    qDebug() << "starting LvLoop thread";
}

void LvLoop::onTimer(void) {
    if (!isRunning) return;

    emit lvLoopTimer();
}

void LvLoop::enable() {
    if (isEnabled) return;
    qDebug() << "LvLoop::enable()";
    blockers = 0;

    isEnabled = true;
    start();
}

void LvLoop::disable() {
    if (!isEnabled) return;
    qDebug() << "LvLoop::disable()";

    end();
    isEnabled = false;
}

void LvLoop::start() {
    if (!isEnabled) return;
    qDebug() << "LvLoop::start()";

    isRunning = true;

    emit lvLoopStart();
}

void LvLoop::stop() {
    if (!isEnabled) return;
    qDebug() << "LvLoop::stop()";

    isRunning = false;
}

void LvLoop::end() {
    if (!isEnabled) return;
    qDebug() << "LvLoop::end()";

//    isRunning = false;

    stop();

    emit lvLoopEnd();

//    int retval = gp_camera_capture_preview_done_nikon(camera, context);
//    qDebug() << "closing LV capture: " << retval;
}

void LvLoop::incBlockers() {
    if (!isEnabled) return;
    blockers++;
    end();
}

void LvLoop::decBlockers() {
    if (!isEnabled) return;
    blockers--;
//    usleep(1 * 1000 * 1000);
    if (blockers == 0 && isEnabled) start();
}
