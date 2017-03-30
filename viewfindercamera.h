#ifndef VIEWFINDERCAMERA_H
#define VIEWFINDERCAMERA_H

#include <QObject>
#include <linux/videodev2.h>

class ViewfinderCamera : public QObject
{
    Q_OBJECT

private:
    int fd_vfcam;
    void* buffer_start;
    struct v4l2_buffer bufferinfo;
    int type;

public:
    explicit ViewfinderCamera(QObject *parent = 0);

    bool start(void);
    bool stop(void);


signals:
    void frameReady(QByteArray);

public slots:
    void onFrameCaptured(QByteArray bytes);
};

#endif // VIEWFINDERCAMERA_H
