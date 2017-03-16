#ifndef MAINTCPSERVER_H
#define MAINTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include "bcamera.h"
#include "bcontrol.h"
#include "btypes.h"

class MainTcpServer : public QObject
{
    Q_OBJECT

private:
    QTcpServer *server;
    bCamera *camera;
    bControl *control;

    int currentConnectionsCount;

public:
    explicit MainTcpServer(QObject *parent = 0);

    void setCamera(bCamera *cam);
    void setControl(bControl *cam);
    int currentConnections(void);

signals:
    
public slots:
    void _onNewConnection(void);
    void onDisconnected(void);
};

#endif // MAINTCPSERVER_H
