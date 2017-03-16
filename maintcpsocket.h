#ifndef MAINTCPSOCKET_H
#define MAINTCPSOCKET_H

#include <QTcpSocket>
#include <QByteArray>
#include "bcamera.h"
#include "btypes.h"
#include <string.h>

class MainTcpSocket : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *socket;

    std::string bufferAll;

public:
    explicit MainTcpSocket(QTcpSocket *s, QObject *parent = 0);
    int connNo;
    
public slots:
    void onDataReady(tHash data);
    void onReadyRead();

signals:
    void newCall(tHash);
};

#endif // MAINTCPSOCKET_H
