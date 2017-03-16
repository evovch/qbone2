#include "stepperspiwatcher.h"
#include <QDebug>

stepperSpiWatcher::stepperSpiWatcher(l6470Status *ms, QObject *parent) : QObject(parent)
{
    motorStatus = ms;
    motorStatus->nowWatched = false;
}

void stepperSpiWatcher::_onWatchLimitsRequest(int low, int high) {
    motorStatus->nowWatched = true;

    while(1) {
        motorStatus->expired = true;
        emit updateMotorStatus();

        while(motorStatus->expired){
            usleep(10000);
        }

        if(motorStatus->isMoving == 0) {
            qDebug() << "exitting by not_moving";
            break;
        }

        if( /* (motorStatus->stalled == 0) || */ ((low >= motorStatus->position) && (motorStatus->dir == 0)) || ((high <= motorStatus->position) && (motorStatus->dir == 1) )) {
               emit limitReached();
                qDebug() << "position reached:" << motorStatus->position;
            break;
        }

        usleep(0.05 * 1000 * 1000);
    }
    motorStatus->nowWatched = false;
}

stepperSpiWatcher::~stepperSpiWatcher()
{
}
