//
//  camera.h
//  bone
//
//  Created by korytov on 09/05/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#ifndef __bone__camera__
#define __bone__camera__

//#include <unistd.h>
#include <thread>

//#include <QtCore>
#include <QObject>
#include <QByteArray>

#include "gphoto2/gphoto2.h"

#include "btypes.h"

class ViewfinderCamera;
class gpioInt;
class LvLoop;

class bCamera : public QObject {
    Q_OBJECT
private:
    Camera *camera;
    GPContext *context;

    ViewfinderCamera *vfcam;

    gpioInt *afPin, *srPin, *usbPin;
    
    void resetUSB();
    
    bool usbSuspended;
    bool lvFirstImage;

    int downloadLimit = 10000;
      
    std::thread *lvThread, *afControlThread, *srControlThread, *camLookupThread;
    bool srOn, afOn;
    
    bool camLookupTerminator;
    
    bool terminator;
    bool pause;
    bool isPaused;
    bool *flushTerminator; //TODO - wat?!

    struct libusb_device_handle *devh;
    
    bool camActive;
    bool camFound;

    unsigned int lvBlockerCount;
    unsigned int focusValue;
        
    const unsigned int focusClientRange = 1000;
    const unsigned int focusCameraRange = 30000;
    
    bool lvRunning, lvPaused, lvActive, vfActive;
    bool camIsNikon, camIsCanon;
    
    void lvLoop();
    
    camInfoType camInfo;
    
    std::string camDownloadDir;
    std::vector<std::string> pwd;
    
    void runCamLookupThread();
    bool lookupCam(void);

    LvLoop *lvLoopThreaded;

    unsigned char * currentFrameBuffer;
    bool viewfinderBusy;

public:
    bCamera(unsigned int n_afPin, unsigned int n_srPin, unsigned int n_usbPin = 0, bool nolookup = false, QObject *parent = 0);
    ~bCamera();
    
    void capture();
    void captureGPIO(uint shutterSpeedMsec = 0, uint holdMsec = 0);
    void initLiveView();

    void pauseLvLoop();
    void unPauseLvLoop();
    
    bool activateLiveView();
    bool deactivateLiveView();
    bool activateViewfinderCam();
    bool deactivateViewfinderCam();

    bool toggleLiveView();
    bool toggleViewfinderCam();

    void initUSB();
    void freeUSB();
    void initCam();
    void freeCam();
    
    void getCameraFile(std::string filename);
    QByteArray captureLvFrame(void);
    
    bool getCamActive() {return camActive;}

    bool isNikon() {return camIsNikon;}
    bool isCanon() {return camIsCanon;}
    
    unsigned int getFocusValue();
  
    bool setConfig(std::string key, std::string val, unsigned int inputType, CameraEventType eventType = GP_EVENT_UNKNOWN, int eventTimeout = 0, QString eventData = ""); //0 - range, 1 - int, 2 - toggle, 3 - menu|radio|text
    std::string getConfig(std::string key, unsigned int inputType, bool getChoice = false);
    
    bool setUsbPower(unsigned int status);
    unsigned int getUsbPower(void);

    void setCamParamIso(std::string value);
    void setCamParamAf(std::string value);
    void setCamParamMode(std::string value);
    void setCamParamD(std::string value);
    void setCamParamS(std::string value);
    void setCamParamExp(std::string value);
    void setCamParamFocusmetermode(std::string value);
    
    void setAutofocusarea(std::string value);
    
    void triggerAf(bool value, float timeLimit = 5);
    void triggerSr(bool value, float timeLimit = 5);
    void srControl(float timeLimit = 5);
    void afControl(float timeLimit = 5);
    
    void focusUp();
    void focusDown();
    void focusUpMuch();
    void focusDownMuch();
    void focusSetValue(unsigned int val);
    
    void nullFocus();
    
    void flushCamera(int downloadLimit = 10000, bool *camFlushTerminator = NULL);
    void downloadCameraDir(std::string dirname);
    
    void updateCamInfo();
    
    camInfoType getCamInfo();

    bool getLvActive();
    bool getVfActive();

signals:
    void frameReady(QByteArray);
    void cameraFound();
    void cameraLost();
    void logToClient(QString);

public slots:
    void onLvLoopFrameReady(QByteArray frame);
    void onViewfinderCameraFrameReady(QByteArray frame);
    void onViewfinderCameraStopped();
    void onCameraFound();
    void onCameraLost();
    void onLvLoopEnd();
    void onLvLoopStart();
    void onLvLoopTimer();
};

#endif // __bone__camera__
