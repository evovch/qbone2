#include "viewfindercamera.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <QDebug>

#include "viewfindercameracapture.h"

ViewfinderCamera::ViewfinderCamera(QObject *parent) : QObject(parent)
{
    isRunning = false;
}

bool ViewfinderCamera::start(void){
    if (isRunning) return false;    //TODO return what?

    if ((fd_vfcam = open("/dev/video1", O_RDWR)) < 0) {
        qDebug() << "VF cam start: can't open FD";
        return false;
    }

    struct v4l2_capability cap;
    if (ioctl(fd_vfcam, VIDIOC_QUERYCAP, &cap) < 0) {
        qDebug() << "VF cam start: can't VIDIOC_QUERYCAP";
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        qDebug() << "VF cam start: no V4L2_CAP_VIDEO_CAPTURE capability";
        return false;
    }

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 640;
    format.fmt.pix.height = 480;

    if (ioctl(fd_vfcam, VIDIOC_S_FMT, &format) < 0) {
        qDebug() << "VF cam start: can't set pix format";
        return false;
    }

    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;

    if (ioctl(fd_vfcam, VIDIOC_REQBUFS, &bufrequest) < 0) {
        qDebug() << "VF cam start: can't VIDIOC_REQBUFS";
        return false;
    }

    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0; /* Queueing buffer index 0. */

    // Put the buffer in the incoming queue.
    if (ioctl(fd_vfcam, VIDIOC_QBUF, &bufferinfo) < 0) {
        qDebug() << "VF cam start: can't VIDIOC_QBUF";
        return false;
    }

    // Activate streaming
    type = bufferinfo.type;
    if (ioctl(fd_vfcam, VIDIOC_STREAMON, &type) < 0) {
        qDebug() << "VF cam start: can't VIDIOC_STREAMON";
        return false;
    }

    buffer_start = mmap(
        NULL,
        bufferinfo.length,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd_vfcam,
        bufferinfo.m.offset
    );

    if (buffer_start == MAP_FAILED) {
        qDebug() << "VF cam start: MAP_FAILED";
        return false;
    }

    memset(buffer_start, 0, bufferinfo.length);

    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;

    // Dequeue the buffer.
    if (ioctl(fd_vfcam, VIDIOC_DQBUF, &bufferinfo) < 0) {
        qDebug() << "VF cam start: can't VIDIOC_QBUF";
        return false;
    }

    vfc_terminator = false;
    vfc = new ViewfinderCameraCapture(&vfc_terminator, &fd_vfcam, buffer_start, &bufferinfo); //TODO corresponding 'delete'?
    connect(vfc, SIGNAL(frameCaptured(QByteArray)), this, SLOT(onFrameCaptured(QByteArray)));
    connect(vfc, SIGNAL(finished()), this, SLOT(stop()));
    vfc->start();

    isRunning = true;

    return true;
}

bool ViewfinderCamera::stop(void){
    if (!isRunning) return false;    //TODO return what?

    disconnect(vfc, SIGNAL(finished()), this, SLOT(stop()));

    qDebug() << "terminating VFC thread";
    vfc_terminator = true;
    vfc->wait();
    qDebug() << "VFC thread terminated";
    delete(vfc); //TODO implement check to avoid double-free

    if (ioctl(fd_vfcam, VIDIOC_STREAMOFF, &type) < 0) {
        qDebug() << "VF cam stop: VIDIOC_STREAMOFF";
    }

    if (close(fd_vfcam) < 0) {
        qDebug() << "VF cam stop: can't close FD";
    }

    munmap(buffer_start, bufferinfo.length); //TODO implement check of the return value

    qDebug() << "VF cam stop: FD closed";

    isRunning = false;

    emit(stopped());

    return true;
}

void ViewfinderCamera::onFrameCaptured(QByteArray bytes) {
    // qDebug() << "new frame length: " << bytes.size();

    emit(frameReady(bytes));
}
