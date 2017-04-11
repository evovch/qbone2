#ifndef LVTCPSOCKET_H
#define LVTCPSOCKET_H

#include <QObject>
#include <QByteArray>

class QTcpSocket;

class LvTcpSocket : public QObject
{
    Q_OBJECT

public:
    explicit LvTcpSocket(QTcpSocket* s, QObject* parent = 0);
    //TODO destructor?

signals:
    
public slots:
    void onFrameReady(QByteArray frame);

private:
    QTcpSocket* socket;

};

#endif // LVTCPSOCKET_H
