#include "maintcpserver.h"

#include <iostream>

#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

#include "btypes.h" // for tHash
#include "bcontrol.h" // to enable 'connect'!
#include "maintcpsocket.h"

MainTcpServer::MainTcpServer(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);
    currentConnectionsCount = 0;
    control = NULL;

    int rc = 0;
    while (rc < 5) {
        if (!server->listen(QHostAddress("127.0.0.1"), 60000))
        {
            qDebug() << "MAIN server could not start";
            std::cout << "MAIN server could not start: " << server->errorString().toStdString() << "\r\n";
        } else {
            qDebug() << "MAIN server started!";
            std::cout << "MAIN server started\r\n";

            QObject::connect(server, SIGNAL(newConnection()), this, SLOT(_onNewConnection()));
            return;
        }

        QThread::sleep(5);
        std::cout << "retrying in 5 sec\r\n";

        rc++;
    }

    std::cout << "giving up on network\r\n";
}

void MainTcpServer::_onNewConnection(void)
{
    qDebug() << "incoming MAIN conection";
    QTcpSocket* socket = server->nextPendingConnection();
    MainTcpSocket* mainsocket = new MainTcpSocket(socket);

    QObject::connect(control, SIGNAL(dataReady(tHash)), mainsocket, SLOT(onDataReady(tHash)));
    QObject::connect(mainsocket, SIGNAL(newCall(tHash)), control, SLOT(onNewCall(tHash)));

    currentConnectionsCount++;
    mainsocket->connNo = currentConnectionsCount;

    if (control != NULL) control->setCurrentConnectionsCount(currentConnectionsCount);

    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}

void MainTcpServer::onDisconnected(void)
{
    currentConnectionsCount--;
    if (control != NULL) control->setCurrentConnectionsCount(currentConnectionsCount);
}
