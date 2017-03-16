#include "maintcpserver.h"
#include "maintcpsocket.h"
#include <QDebug>

MainTcpServer::MainTcpServer(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);
    currentConnectionsCount = 0;
    control = NULL;

    int rc = 0;
    while(rc < 5) {
        if(!server->listen(QHostAddress("192.168.1.100"), 60000))
        {
            qDebug() << "MAIN server could not start";
            std::cout << "MAIN server could not start: " << server->errorString().toStdString() << "\r\n";
        }
        else
        {
            qDebug() << "MAIN server started!";
            std::cout << "MAIN server started\r\n";

            QObject::connect(server, SIGNAL(newConnection()),
                    this, SLOT(_onNewConnection()));
            return;

        }

        QThread::sleep(5);
        std::cout << "retrying in 5 sec\r\n";

        rc++;
    }

    std::cout << "giving up on network\r\n";
}

void MainTcpServer::setCamera(bCamera *cam) {
    camera = cam;
}

void MainTcpServer::setControl(bControl *ctl) {
    control = ctl;
}


void MainTcpServer::_onNewConnection(void)
{
    qDebug() << "incoming MAIN conection";
    QTcpSocket *socket = server->nextPendingConnection();
    MainTcpSocket *mainsocket = new MainTcpSocket(socket);

    QObject::connect(control, SIGNAL(dataReady(tHash)), mainsocket, SLOT(onDataReady(tHash)));
    QObject::connect(mainsocket, SIGNAL(newCall(tHash)), control, SLOT(onNewCall(tHash)));

    currentConnectionsCount++;
    mainsocket->connNo = currentConnectionsCount;

    if(control != NULL)control->setCurrentConnectionsCount(currentConnectionsCount);

    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}

void MainTcpServer::onDisconnected(void) {
    currentConnectionsCount--;
     if(control != NULL)control->setCurrentConnectionsCount(currentConnectionsCount);
}

int MainTcpServer::currentConnections(void) {
    return currentConnectionsCount;
}
