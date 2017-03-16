#include <QCoreApplication>
#include <QDebug>
#include "../../bcamera.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "running cam trigger";

    bCamera *cam = new bCamera(60, 50, 51, 0, true); //AF: P9_16, SR: P9_14

    qDebug() << "on";
    cam->triggerAf(true);
    cam->triggerSr(true);
    usleep(0.01 * 1000 * 1000);
    qDebug() << "off";
    cam->triggerSr(false);
    cam->triggerAf(false);


    return a.exec();
}
