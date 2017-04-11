#ifndef LVLOOP_H
#define LVLOOP_H

#include <QObject>
#include <QByteArray>

class QTimer;

class LvLoop : public QObject
{
    Q_OBJECT

private:
    QTimer* timer;

    int blockers;
    bool isRunning;
    bool isEnabled;
    bool busy;

    QTimer* lvTimer;

public:
    explicit LvLoop(QObject *parent = 0);

    void init();
    void start();
    void stop();

    void enable();
    void disable();
    void end();
    void incBlockers();
    void decBlockers();

signals:
    void frameReady(QByteArray);
    void lvLoopEnd();
    void lvLoopStart();
    void lvLoopTimer();

public slots:
    void onTimer(void);
    
};

#endif // LVLOOP_H
