#ifndef LVTCPSERVER_H
#define LVTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include "bcamera.h"

class LvTcpServer : public QObject
{
    Q_OBJECT

private:
    QTcpServer *server;
    bCamera *camera;

public:
    explicit LvTcpServer(QObject *parent = 0);

    void setCamera(bCamera *cam);

signals:
    
public slots:
    void _onNewConnection(void);
};

#endif // LVTCPSERVER_H
