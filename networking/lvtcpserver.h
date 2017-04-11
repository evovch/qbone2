#ifndef LVTCPSERVER_H
#define LVTCPSERVER_H

#include <QObject>

class QTcpServer;
class bCamera;

class LvTcpServer : public QObject
{
    Q_OBJECT

public:
    explicit LvTcpServer(QObject *parent = 0);
    //TODO destructor?

    void setCamera(bCamera* cam) { camera = cam; }

private:
    QTcpServer* server;
    bCamera* camera;

signals:
    
public slots:
    void _onNewConnection(void);

};

#endif // LVTCPSERVER_H
