#include "gpioint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <QDebug>

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

gpioInt::gpioInt(unsigned int gpio, QObject *parent) :
    QThread(parent)
{
    gpioNum = gpio;
    gpio_export(gpioNum);
}

int gpioInt::setValue(unsigned int value)
{
    return gpio_set_value(gpioNum, value);
}

unsigned int gpioInt::getValue(void)
{
    unsigned int v;

    gpio_get_value(gpioNum, &v);

    return v;
}

int gpioInt::setDir(unsigned int out_flag)
{
    return gpio_set_dir(gpioNum, out_flag);
}

void gpioInt::run(void) {
    pollingLoop(gpioNum);
}

void gpioInt::pollingLoop(unsigned int gpio)
{
    struct pollfd fdset[2];
    int nfds = 2;
    int gpio_fd, timeout, rc;

    int len;

    gpio_export(gpio);
    gpio_set_dir(gpio, 0);
    gpio_set_edge(gpio, "both");

    timeout = POLL_TIMEOUT;

    gpio_fd = gpio_fd_open(gpio);

    while (1) {
        char buf[MAX_BUF];
        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd = gpio_fd;
        fdset[0].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (rc < 0) {
            qDebug() << "npoll() failed!";
            return;
        }

        if (rc == 0) {
            printf(".");
        }

        if (fdset[0].revents & POLLPRI) {
            len = read(fdset[0].fd, buf, MAX_BUF);

            unsigned int v;
            gpio_get_value(gpio, &v);

//            qDebug() << "GPIO interrupt occurred";

//            printf("\npoll() GPIO %d interrupt occurred\n", gpio);

            if(v == 1)emit gpioEdge(gpio, true);
            else emit gpioEdge(gpio, false);
        }

    }

    gpio_fd_close(gpio_fd);
}


/****************************************************************
 * gpio_export
 ****************************************************************/
int gpioInt::gpio_export(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);

    return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpioInt::gpio_unexport(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpioInt::gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/direction");
        return fd;
    }

    if (out_flag)
        write(fd, "out", 4);
    else
        write(fd, "in", 3);

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpioInt::gpio_set_value(unsigned int gpio, unsigned int value)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-value");
        return fd;
    }

    if (value == 1)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpioInt::gpio_get_value(unsigned int gpio, unsigned int *value)
{
    int fd, len;
    char buf[MAX_BUF];
    char ch;

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        perror("gpio/get-value");
        return fd;
    }

    read(fd, &ch, 1);

    if (ch != '0') {
        *value = 1;
    } else {
        *value = 0;
    }

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpioInt::gpio_set_edge(unsigned int gpio, char *edge)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-edge");
        return fd;
    }

    write(fd, edge, strlen(edge) + 1);
    close(fd);
    return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpioInt::gpio_fd_open(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY | O_NONBLOCK );
    if (fd < 0) {
        perror("gpio/fd_open");
    }
    return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpioInt::gpio_fd_close(int fd)
{
    return close(fd);
}
