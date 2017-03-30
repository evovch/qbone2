#ifndef VIEWFINDERCAMERACAPTURE_H
#define VIEWFINDERCAMERACAPTURE_H

#include <QObject>
#include <QThread>

#include <linux/videodev2.h>

class ViewfinderCameraCapture : public QThread
{
    Q_OBJECT

public:
    explicit ViewfinderCameraCapture(int *in_fd_vfcam,  void* in_buffer_start, struct v4l2_buffer *in_bufferinfo);

private:
    int *fd_vfcam;
    void *buffer_start;
    struct v4l2_buffer *bufferinfo;

    void run(void);

signals:
    void frameCaptured(QByteArray);

};

#endif // VIEWFINDERCAMERACAPTURE_H
