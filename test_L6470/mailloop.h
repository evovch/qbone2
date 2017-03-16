#ifndef MAILLOOP_H
#define MAILLOOP_H

#include <QObject>

class MailLoop : public QObject
{
    Q_OBJECT
public:
    explicit MailLoop(QObject *parent = 0);
    ~MailLoop();

signals:

public slots:
};

#endif // MAILLOOP_H
