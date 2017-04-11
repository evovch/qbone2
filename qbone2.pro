QT += core network gui widgets multimedia

CONFIG += console

release: DESTDIR = ../qbone2-build-release
debug:   DESTDIR = ../qbone2-build-debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

#TEMPLATE = app

#INCLUDEPATH += /opt/arm/include

#QMAKE_CXXFLAGS += -I/usr/local/include/
#LIBS += -L/home/evovch/Documents/DKV/libgphoto2-2.5.10-dkv-patched/ -lgphoto2

#QMAKE_CXXFLAGS += -std=c++11 -fpermissive -pthread -Iu8glib/src/ -I/opt/arm/include -I/opt/arm/include/arm-linux-gnueabihf -I/home/korytov/libs/include
#QMAKE_LFLAGS += -pthread -L/opt/arm/lib/arm-linux-gnueabihf -L/home/korytov/libs/lib -L/opt/arm/lib -Wl,-O1 -Wl,-rpath,/opt/qt-arm/lib,-rpath,/home/korytov/libs/lib,-rpath,/opt/arm/lib/arm-linux-gnueabihf,-rpath,/opt/arm/lib -lusb-1.0 -lgphoto2 -lPocoNet -lPocoFoundation -lPocoUtil -lPocoXML -lrt
#QMAKE_LFLAGS += -pthread -L/opt/arm/lib/arm-linux-gnueabihf -L/opt/arm/lib -Wl,-O1 -Wl,-rpath,/opt/arm/lib,-rpath,/opt/arm/arm-linux-gnueabihf/lib -lgphoto2 -lPocoFoundation -lrt -ludev -lturbojpeg -lusb-1.0
#QMAKE_CFLAGS += -Iu8glib/src/ -I/opt/arm/include -I/opt/arm/include/arm-linux-gnueabihf -I/home/korytov/libs/include
#LIBS += /home/korytov/libs/lib/libu8glib.a /opt/arm/lib/libusb-1.0.a
#LIBS += /opt/arm/lib/libusb-1.0.a

QMAKE_CXXFLAGS += -std=c++11 -fpermissive -Wall -Wextra -Wpedantic

LIBS += -lusb-1.0
LIBS += -L/usr/local/lib/ -lgphoto2

INCLUDEPATH += networking/

SOURCES += main.cpp \
    bcontrol.cpp \
    bcamera.cpp \
    networking/maintcpserver.cpp \
    networking/maintcpsocket.cpp \
    networking/lvtcpserver.cpp \
    networking/lvtcpsocket.cpp \
    timelapse.cpp \
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
    networking/maintcpserver.h \
    networking/maintcpsocket.h \
    networking/lvtcpserver.h \
    networking/lvtcpsocket.h \
    timelapse.h \
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

DISTFILES += \
    NOTES
