#include "viewfindercameracapture.h"
#include <sys/ioctl.h>

#include <QDebug>

ViewfinderCameraCapture::ViewfinderCameraCapture(bool *in_terminator, int *in_fd_vfcam,  void* in_buffer_start, struct v4l2_buffer *in_bufferinfo){
    fd_vfcam = in_fd_vfcam;
    buffer_start = in_buffer_start;
    bufferinfo = in_bufferinfo;

    terminator = in_terminator;

    setTerminationEnabled();

    qDebug() << "ViewfinderCameraCapture thread constructed";
}

void ViewfinderCameraCapture::run(void)
{
    while(1)
    {
        if(*terminator)break;
        // Queue the next one.
        if(ioctl(*fd_vfcam, VIDIOC_QBUF, bufferinfo) < 0){
            qDebug() << "VF cam capture: can't VIDIOC_QBUF";
            break;
        }

        // Dequeue the buffer.
        if(ioctl(*fd_vfcam, VIDIOC_DQBUF, bufferinfo) < 0){
            qDebug() << "VF cam capture: can't VIDIOC_QBUF";
            break;
        }

        QByteArray f((const char*)buffer_start, bufferinfo->bytesused);
        f.append("--cutjpeg--");

        emit frameCaptured(f);
//        qDebug() << "next frame";

        usleep(10);
    }
}
