#ifndef GPIOINT_H
#define GPIOINT_H

#include <QObject>
#include <QThread>

class gpioInt : public QThread
{
    Q_OBJECT

private:
    void pollingLoop(unsigned int gpio);
    unsigned int gpioNum;

protected:
    void run(void);

public:
    explicit gpioInt(unsigned int gpio, QObject *parent = 0);

    int setValue(unsigned int value);
    unsigned int getValue(void);
    int setDir(unsigned int out_flag);

    static int gpio_export(unsigned int gpio);
    static int gpio_unexport(unsigned int gpio);
    static int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
    static int gpio_set_value(unsigned int gpio, unsigned int value);
    static int gpio_get_value(unsigned int gpio, unsigned int *value);
    static int gpio_set_edge(unsigned int gpio, char *edge);
    static int gpio_fd_open(unsigned int gpio);
    static int gpio_fd_close(int fd);

signals:
     void gpioEdge(unsigned int, bool);
    
public slots:
    
};

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

#endif // GPIOINT_H
