#ifndef MAINTCPSOCKET_H
#define MAINTCPSOCKET_H

#include <QObject>

#include "btypes.h" // for tHash
#include <string>

class QTcpSocket;

class MainTcpSocket : public QObject
{
    Q_OBJECT

public:
    explicit MainTcpSocket(QTcpSocket* s, QObject* parent = 0);
    int connNo;
    
public slots:
    void onDataReady(tHash data);
    void onReadyRead(void);

signals:
    void newCall(tHash);

private:
    QTcpSocket* socket;

    std::string bufferAll;

};

#endif // MAINTCPSOCKET_H
