#ifndef VIEWFINDERCAMERACAPTURE_H
#define VIEWFINDERCAMERACAPTURE_H

#include <linux/videodev2.h>

#include <QThread>

class ViewfinderCameraCapture : public QThread
{
    Q_OBJECT

public:
    explicit ViewfinderCameraCapture(bool *in_terminator, int *in_fd_vfcam,  void* in_buffer_start, struct v4l2_buffer *in_bufferinfo);
    //TODO destructor?

signals:
    void frameCaptured(QByteArray);

private:
    int *fd_vfcam;
    void *buffer_start;
    struct v4l2_buffer *bufferinfo;
    bool *terminator;

    void run(void);

};

#endif // VIEWFINDERCAMERACAPTURE_H
