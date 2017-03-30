QT       += core network gui widgets multimedia

#CONFIG += console

release: DESTDIR = ../qbone2-build-release
debug:   DESTDIR = ../qbone2-build-debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

#TEMPLATE = app

#INCLUDEPATH += /opt/arm/include

QMAKE_CXXFLAGS += -std=c++11 -fpermissive
LIBS += -lgphoto2 -lusb-1.0 -lopencv_videoio -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

#QMAKE_CXXFLAGS += -std=c++11 -fpermissive -pthread -Iu8glib/src/ -I/opt/arm/include -I/opt/arm/include/arm-linux-gnueabihf -I/home/korytov/libs/include
#QMAKE_LFLAGS += -pthread -L/opt/arm/lib/arm-linux-gnueabihf -L/home/korytov/libs/lib -L/opt/arm/lib -Wl,-O1 -Wl,-rpath,/opt/qt-arm/lib,-rpath,/home/korytov/libs/lib,-rpath,/opt/arm/lib/arm-linux-gnueabihf,-rpath,/opt/arm/lib -lusb-1.0 -lgphoto2 -lPocoNet -lPocoFoundation -lPocoUtil -lPocoXML -lrt
#QMAKE_LFLAGS += -pthread -L/opt/arm/lib/arm-linux-gnueabihf -L/opt/arm/lib -Wl,-O1 -Wl,-rpath,/opt/arm/lib,-rpath,/opt/arm/arm-linux-gnueabihf/lib -lgphoto2 -lPocoFoundation -lrt -ludev -lturbojpeg -lusb-1.0
#QMAKE_CFLAGS += -Iu8glib/src/ -I/opt/arm/include -I/opt/arm/include/arm-linux-gnueabihf -I/home/korytov/libs/include
#LIBS += /home/korytov/libs/lib/libu8glib.a /opt/arm/lib/libusb-1.0.a
#LIBS += /opt/arm/lib/libusb-1.0.a

SOURCES += main.cpp \
    bcontrol.cpp \
    bcamera.cpp \
    lvtcpserver.cpp \
    lvtcpsocket.cpp \
    timelapse.cpp \
    maintcpserver.cpp \
    maintcpsocket.cpp \
    lvloop.cpp \
    gpioint.cpp \
    pwlistener.cpp \
    stepperspi.cpp \
    stepperspiwatcher.cpp \
    mainwidget.cpp \
    viewfindercamera.cpp \
    viewfindercameracapture.cpp


HEADERS += \
    btypes.h \
    bcontrol.h \
    bcamera.h \
    lvtcpserver.h \
    lvtcpsocket.h \
    timelapse.h \
    maintcpserver.h \
    maintcpsocket.h \
    lvloop.h \
    gpioint.h \
    pwlistener.h \
    stepperspi.h \
    stepperspiwatcher.h \
    l6470/l6470-support.h \
    l6470/l6470.h \
    bus_protocol/fs_spi.h \
    bus_protocol/ispi.h \
    mainwidget.h \
    viewfindercamera.h \
    viewfindercameracapture.h

FORMS += \
    mainwidget.ui

