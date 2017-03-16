#include <QCoreApplication>
#include "mailloop.h"

//#define TI_MODE 1

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MailLoop ml;

//     m1->togglePower();
/*
    qDebug() << "turn 1";
   m1->async_rotate(50);
   QThread::sleep(5);
   qDebug() << "turn 2";
   m1->async_rotate(-100);

//       m1->async_seek(1000);

*/

//    stepperSpi *m2 = new stepperSpi(spiBus, 3, ms);
//    stepperSpi *m3 = new stepperSpi(spiBus, 4, ms);

    /*
    m1->async_seek(1000);
    m2->async_seek(1000);
    m3->async_seek(1000);
*/
//    m_zoom->togglePower();

    return a.exec();
}
