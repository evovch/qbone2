#include "pwlistener.h"
#include <QDebug>

PWListener::PWListener(int gpioId, QObject *parent) : QObject(parent)
{
    buttonGpio = new gpioInt(gpioId);
    buttonGpio->start();

    timer = new QTime();
    timer->start();

    ticksCount = 0;
    lastInterval = 0;

    QObject::connect(buttonGpio, SIGNAL(gpioEdge(unsigned int, bool)), this, SLOT(_onGpioEdge(uint, bool)));


}

void PWListener::_onGpioEdge(uint gpioId, bool level) {
    qDebug() << "got edge";
    if (level == true)return;

    int elapsed = timer->elapsed();
    int currentInterval = std::round((float)elapsed / 10);
    qDebug() << "got interval: " << currentInterval << " - elapsed: " << elapsed;

    if(currentInterval == lastInterval)ticksCount++;
    else {
        lastInterval = currentInterval;
        ticksCount = 0;
    }
    timer->restart();

    if (ticksCount == 3) {
        emit grabbedSignal(lastInterval);
        qDebug() << "grabbedSignal: " << lastInterval;

        lastInterval = 0;
        ticksCount = 0;
    }
}

PWListener::~PWListener()
{

}

