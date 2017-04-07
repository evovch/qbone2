#ifndef VIEWFINDERCAMERA_H
#define VIEWFINDERCAMERA_H

#include <QObject>
#include <linux/videodev2.h>
#include "viewfindercameracapture.h"

class ViewfinderCamera : public QObject
{
    Q_OBJECT

private:
    int fd_vfcam;
    void* buffer_start;
    struct v4l2_buffer bufferinfo;
    int type;

    ViewfinderCameraCapture *vfc;
    bool vfc_terminator;
    bool isRunning;

public:
    explicit ViewfinderCamera(QObject *parent = 0);

    bool start(void);

signals:
    void frameReady(QByteArray);
    void stopped(void);

public slots:
    void onFrameCaptured(QByteArray bytes);
    bool stop(void);
};

#endif // VIEWFINDERCAMERA_H
