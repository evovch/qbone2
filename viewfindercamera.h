#ifndef VIEWFINDERCAMERA_H
#define VIEWFINDERCAMERA_H

#include <linux/videodev2.h>

#include <QObject>

class ViewfinderCameraCapture;

class ViewfinderCamera : public QObject
{
    Q_OBJECT

public:
    explicit ViewfinderCamera(QObject *parent = 0);
    //TODO destructor?

    bool start(void);

signals:
    void frameReady(QByteArray);
    void stopped(void);

public slots:
    void onFrameCaptured(QByteArray bytes);
    bool stop(void);

private:
    int fd_vfcam;
    void* buffer_start;
    struct v4l2_buffer bufferinfo;
    int type;

    ViewfinderCameraCapture *vfc;
    bool vfc_terminator;
    bool isRunning;

};

#endif // VIEWFINDERCAMERA_H
