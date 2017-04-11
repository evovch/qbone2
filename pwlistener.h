#ifndef PWLISTENER_H
#define PWLISTENER_H

#include <QObject>
#include <QTime>

class gpioInt;

class PWListener : public QObject
{
    Q_OBJECT

public:
    explicit PWListener(int gpioId, QObject *parent = 0);
    ~PWListener();

signals:
    void grabbedSignal(int);

public slots:
     void _onGpioEdge(uint gpioId, bool level);

private:
     gpioInt *buttonGpio;
     int lastInterval;
     int ticksCount;

     QTime *timer; //TODO really? Not QTimer?

};

#endif // PWLISTENER_H
