#ifndef MAINTCPSOCKET_H
#define MAINTCPSOCKET_H

#include "btypes.h" // for tHash
#include <string>

#include <QObject>

class QTcpSocket;

class MainTcpSocket : public QObject
{
    Q_OBJECT

private:
    QTcpSocket* socket;

    std::string bufferAll;

public:
    explicit MainTcpSocket(QTcpSocket* s, QObject* parent = 0);
    int connNo;
    
public slots:
    void onDataReady(tHash data);
    void onReadyRead(void);

signals:
    void newCall(tHash);
};

#endif // MAINTCPSOCKET_H
