#include "lvtcpserver.h"
#include "lvtcpsocket.h"
#include <QDebug>

LvTcpServer::LvTcpServer(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);

    if(!server->listen(QHostAddress("192.168.1.100"), 60005))
    {
        qDebug() << "Server could not start";
    }
    else
    {
        qDebug() << "Server started!";
    }

    QObject::connect(server, SIGNAL(newConnection()),
            this, SLOT(_onNewConnection()));

}

void LvTcpServer::setCamera(bCamera *cam) {
    camera = cam;
}

void LvTcpServer::_onNewConnection(void)
{
    qDebug() << "incoming LV conection";
    QTcpSocket *socket = server->nextPendingConnection();
    LvTcpSocket *lvsocket = new LvTcpSocket(socket);
    QObject::connect(camera, SIGNAL(frameReady(QByteArray)), lvsocket, SLOT(onFrameReady(QByteArray)));
}
