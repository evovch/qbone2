#ifndef LVTCPSOCKET_H
#define LVTCPSOCKET_H

#include <QTcpSocket>
#include <QByteArray>
#include "bcamera.h"

class LvTcpSocket : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *socket;
public:
    explicit LvTcpSocket(QTcpSocket *s, QObject *parent = 0);
    
signals:
    
public slots:
    void onFrameReady(QByteArray frame);
};

#endif // LVTCPSOCKET_H
