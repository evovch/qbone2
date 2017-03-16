#ifndef PWLISTENER_H
#define PWLISTENER_H

#include <QObject>
#include <QTime>
#include "gpioint.h"

class PWListener : public QObject
{
    Q_OBJECT

private:
     gpioInt *buttonGpio;
     int lastInterval;
     int ticksCount;

     QTime *timer;

public:
    explicit PWListener(int gpioId, QObject *parent = 0);
    ~PWListener();

signals:
    void grabbedSignal(int);

public slots:
     void _onGpioEdge(uint gpioId, bool level);
};

#endif // PWLISTENER_H
