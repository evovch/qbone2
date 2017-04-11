#include "maintcpsocket.h"

#include <iostream>

#include <QByteArray>
#include <QTcpSocket> // to enable 'connect'!

MainTcpSocket::MainTcpSocket(QTcpSocket *s, QObject *parent) :
    QObject(parent)
{
    bufferAll = std::string("");
    socket = s;
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void MainTcpSocket::onDataReady(tHash data)
{
    std::string s = data.dev + ":" + data.key + ":" + data.value + ":";

    for (unsigned int i=0; i < data.params.size(); i++) {
        s += data.params[i]+',';
    }

    s += "\n";

    if (socket->bytesToWrite() > 1024) {
        return;
    }

    qint64 res = socket->write(QByteArray(s.c_str()));
    if (-1 == res) {
        qDebug() << "socket failure: aborting";
        socket->close();
        this->deleteLater();
    }
}

void MainTcpSocket::onReadyRead(void)
{
    char buffer[20000];

    if(socket->bytesAvailable() != 0) {
        int n = socket->read(buffer, socket->bytesAvailable());
        bufferAll += std::string(buffer, n);
    }
    std::size_t found;
    while(1) {
        found = bufferAll.find("\n");
        if(found == std::string::npos)break;
        std::string chunk = bufferAll.substr(0, found);
        bufferAll.erase(0,found+1);

        QStringList tokens = QString(chunk.c_str()).split(':');

        std::vector<std::string> params;
        if(tokens.size() < 3) {
            std::cout << "corrupted data:" << chunk << "\n" << std::flush;
            continue;
        }
        if(tokens.size() == 4) {
            QStringList ptokens = tokens.at(3).split(',');
            for(int i=0; i < ptokens.size(); i++){
                params.push_back(ptokens.at(i).toStdString());
            }
        }
        tHash out;
        out.dev = tokens.at(0).toStdString();
        out.key = tokens.at(1).toStdString();
        out.value = tokens.at(2).toStdString();
        out.params = params;

        emit newCall(out);
    }
}
