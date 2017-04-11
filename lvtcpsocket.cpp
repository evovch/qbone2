#include "lvtcpsocket.h"

#include <QTcpSocket>

LvTcpSocket::LvTcpSocket(QTcpSocket *s, QObject *parent) :
    QObject(parent),
    socket(s)
{
}

void LvTcpSocket::onFrameReady(QByteArray frame)
{
    if (socket->bytesToWrite() > 1024 * 1024) {
        return;
    }
    socket->write(frame.data(), frame.size());
}
