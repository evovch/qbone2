#include "viewfindercamera.h"

#include <fstream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include "viewfindercameracapture.h"

#include <QDebug>


ViewfinderCamera::ViewfinderCamera(QObject *parent) : QObject(parent)
{
}

bool ViewfinderCamera::start(void){
    if((fd_vfcam = open("/dev/video1", O_RDWR)) < 0){
        qDebug() << "VF cam start: can't open FD";
        return(false);
    }

    struct v4l2_capability cap;
    if(ioctl(fd_vfcam, VIDIOC_QUERYCAP, &cap) < 0){
        qDebug() << "VF cam start: can't VIDIOC_QUERYCAP";
        return(false);
    }

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
        qDebug() << "VF cam start: no V4L2_CAP_VIDEO_CAPTURE capability";
        return(false);
    }

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 640;
    format.fmt.pix.height = 480;

    if(ioctl(fd_vfcam, VIDIOC_S_FMT, &format) < 0){
        qDebug() << "VF cam start: can't set pix format";
        return(false);
    }

    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;

    if(ioctl(fd_vfcam, VIDIOC_REQBUFS, &bufrequest) < 0){
        qDebug() << "VF cam start: can't VIDIOC_REQBUFS";
        return(false);
    }

    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0; /* Queueing buffer index 0. */

    // Put the buffer in the incoming queue.
    if(ioctl(fd_vfcam, VIDIOC_QBUF, &bufferinfo) < 0){
        qDebug() << "VF cam start: can't VIDIOC_QBUF";
        return(false);
    }

    // Activate streaming
    type = bufferinfo.type;
    if(ioctl(fd_vfcam, VIDIOC_STREAMON, &type) < 0){
        qDebug() << "VF cam start: can't VIDIOC_STREAMON";
        return(false);
    }

    buffer_start = mmap(
        NULL,
        bufferinfo.length,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd_vfcam,
        bufferinfo.m.offset
    );

    if(buffer_start == MAP_FAILED){
        qDebug() << "VF cam start: MAP_FAILED";
        return(false);
    }

    memset(buffer_start, 0, bufferinfo.length);

    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;

    // Dequeue the buffer.
    if(ioctl(fd_vfcam, VIDIOC_DQBUF, &bufferinfo) < 0){
        qDebug() << "VF cam start: can't VIDIOC_QBUF";
        return(false);
    }

    ViewfinderCameraCapture *vfc = new ViewfinderCameraCapture(&fd_vfcam, buffer_start, &bufferinfo);
    connect(vfc, SIGNAL(frameCaptured(QByteArray)), this, SLOT(onFrameCaptured(QByteArray)));
    vfc->start();

    return(true);
}

bool ViewfinderCamera::stop(void){
    if(ioctl(fd_vfcam, VIDIOC_STREAMOFF, &type) < 0){
        qDebug() << "VF cam stop: VIDIOC_STREAMOFF";
        return(false);
    }

    return(true);
}

void ViewfinderCamera::onFrameCaptured(QByteArray bytes) {
    // qDebug() << "new frame length: " << bytes.size();

    emit(frameReady(bytes));
}
