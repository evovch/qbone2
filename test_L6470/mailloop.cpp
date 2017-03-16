#include "mailloop.h"
#include <QDebug>
#include "../stepperspi.h"
#include "../bus_protocol/fs_spi.h"
#include "../gpioint.h"
#include "../btypes.h"

//#define TI_MODE 1

MailLoop::MailLoop(QObject *parent) : QObject(parent)
{
    l6470Setup ms;

 /*
#define MAX_SPD 300.0
#define KVAL_HOLD 200
#define KVAL_ACC 200
#define KVAL_DEC 200
#define KVAL_RUN 200
#define INT_SPD 0x18C6
#define ST_SLP 0x16
#define SLP_ACC 0x62
#define SLP_DEC 0x62
#define M_STP 128
#define MOT_ACC 100.0
    */

    ms.m_stp = 8;
    ms.mot_acc = 250;
    ms.kval_hold = 70;
    ms.kval_run = ms.kval_hold;

#ifdef TI_MODE
    FS_SPI *spiBus = new FS_SPI(500000, "/dev/spidev1.0");
#else
    FS_SPI *spiBus = new FS_SPI(500000, "/dev/spidev0.2");
#endif

    stepperSpi *m1 = new stepperSpi(spiBus, 2, ms);
    QThread::sleep(2);
    m1->async_rotate(10);
     QThread::sleep(2);
     qDebug() << "pos: " << m1->getPosition();
}

MailLoop::~MailLoop()
{

}

