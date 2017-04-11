#include "lvtcpserver.h"

#include <QDebug>
#include <QTcpServer>

#include "lvtcpsocket.h" // to enable 'connect'!
#include "bcamera.h" // to enable 'connect'!

LvTcpServer::LvTcpServer(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);

    if (!server->listen(QHostAddress("127.0.0.1"), 60005)) {
        qDebug() << "Server could not start";
    } else {
        qDebug() << "Server started!";
    }

    QObject::connect(server, SIGNAL(newConnection()),
            this, SLOT(_onNewConnection()));
}

void LvTcpServer::_onNewConnection(void)
{
    qDebug() << "incoming LV conection";
    QTcpSocket* socket = server->nextPendingConnection();
    LvTcpSocket* lvsocket = new LvTcpSocket(socket);
    QObject::connect(camera, SIGNAL(frameReady(QByteArray)), lvsocket, SLOT(onFrameReady(QByteArray)));
}
