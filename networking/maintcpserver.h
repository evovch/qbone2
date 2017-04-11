#ifndef MAINTCPSERVER_H
#define MAINTCPSERVER_H

#include <QObject>

class QTcpServer;
class bCamera;
class bControl;

class MainTcpServer : public QObject
{
    Q_OBJECT

public:
    explicit MainTcpServer(QObject *parent = 0);
    //TODO destructor?

    void setCamera(bCamera* cam) { camera = cam; }
    void setControl(bControl* ctl) { control = ctl; }
    //int getCurrentConnectionsCount(void) { return currentConnectionsCount; } //TODO not used?

signals:
    
public slots:
    void _onNewConnection(void);
    void onDisconnected(void);

private:
    QTcpServer* server; //TODO Singleton?
    bCamera* camera; //TODO Singleton?
    bControl* control; //TODO Singleton?

    unsigned int currentConnectionsCount;

};

#endif // MAINTCPSERVER_H
