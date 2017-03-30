//
//  camera.cpp
//  bone
//
//  Created by korytov on 09/05/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#include "bcamera.h"
#include "libusb-1.0/libusb.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <QDebug>
//#include <turbojpeg.h>

bCamera::bCamera(unsigned int n_afPin, unsigned int n_srPin, unsigned int n_usbPin, bool nolookup, QObject *parent) : QObject(parent) {
    camActive = false;
    lvActive = false;

    camIsNikon = false;
    camIsCanon = false;

    viewfinderBusy = false;

    setUsbPower(1);
    usbSuspended = false;
    devh = NULL;
    flushTerminator = NULL;

    initUSB();
//    lookupCam();
//    initCam();

    afPin = new gpioInt(n_afPin);
    srPin = new gpioInt(n_srPin);

    afPin->setDir(1);
    srPin->setDir(1);

    if(n_usbPin != 0) {
        usbPin = new gpioInt(n_usbPin);
        usbPin->setDir(1);
    }
    else {
        usbPin = NULL;
    }

//    enaPin->setValue(0);

    pause = true;

    camDownloadDir = "/emmc/images";

    focusValue = 0;

    qDebug() << "VFC";
    vfcam = new ViewfinderCamera();
    if(vfcam->start()) {
        vfActive = true;
        connect(vfcam, SIGNAL(frameReady(QByteArray)), this, SLOT(onViewfinderCameraFrameReady(QByteArray)));
    }

    if (!nolookup) {
        camLookupThread = new std::thread( &bCamera::runCamLookupThread, this );
        QObject::connect(this, SIGNAL(cameraFound()), this, SLOT(onCameraFound()));
        QObject::connect(this, SIGNAL(cameraLost()), this, SLOT(onCameraLost()));
    }
}


void bCamera::runCamLookupThread(void) {
    camLookupTerminator = false;

    std::cout << "starting cam lookup thread\r\n" << std::flush;
    while(!camLookupTerminator) {
        usleep(5 * 1000 * 1000);

        bool f = lookupCam();
        
        if(f && !camActive) {
            qDebug() << "camera found...\r\n";
            emit(cameraFound());
        }
        else if (!f && camActive) {
            qDebug() << "camera lost...\r\n";
            emit(cameraLost());
        }
    }
}

void bCamera::onCameraFound(){
    initCam();
    updateCamInfo();

    setConfig("viewfinder", "0", 2);
    setConfig("controlmode", "0", 2);
    setConfig("capturetarget", "1", 4);
    setConfig("recordingmedia", "0", 4);
}

void bCamera::onCameraLost(){
    freeCam();
}

bool bCamera::lookupCam(void) {
    if(usbSuspended)return(false);
    int l_ret;
    
    struct libusb_device **devs;
    libusb_device *l_dev;
    struct libusb_device_descriptor  desc;
    
    qDebug() << "searching for camera...";

    l_ret = libusb_get_device_list(NULL, &devs);
    
    int i = 0;
    while ((l_dev = devs[i++]) != NULL) {
        int r  = libusb_get_device_descriptor (l_dev, &desc);

        if (desc.idVendor == 1200) {
            if(devh == NULL)l_ret = libusb_open(l_dev, &devh);
            camIsNikon = true;
            qDebug() << "NIKON camera found\r\n";

            return(true);
            
            break;
        }
        else if (desc.idVendor == 0x04a9) {
            if(devh == NULL)l_ret = libusb_open(l_dev, &devh);
            camIsCanon = true;
            qDebug() << "CANON camera found\r\n";

            return(true);

            break;
        }
    }

//    libusb_free_device_list(devs, 1);

    if(devh != NULL) {
//        libusb_close(devh);
        devh = NULL;
    }

    qDebug() << "not found\r\n";
    return(false);
}

void bCamera::initUSB(void) {
    int l_ret;
    l_ret = libusb_init(NULL);
    
    lvRunning = false;
}


void bCamera::freeUSB(void) {
    libusb_exit(NULL);
    std::cout << "USB closed\r\n" << std::flush;
    
    camActive = false;
}

void bCamera::initCam(void){
    std::cout << "USB reset & cam init\r\n" << std::flush;

    libusb_reset_device(devh);
    
    gp_camera_new(&camera);
    context = gp_context_new();

    gp_camera_init(camera, context);

    lvLoopThreaded = new LvLoop();

    QThread *thread = new QThread();
//    QObject::connect(lvLoopThreaded, SIGNAL(frameReady(QByteArray)), this, SLOT(onLvLoopFrameReady(QByteArray)));
    QObject::connect(lvLoopThreaded, SIGNAL(lvLoopTimer()), this, SLOT(onLvLoopTimer()), Qt::QueuedConnection);
    QObject::connect(lvLoopThreaded, SIGNAL(lvLoopEnd()), this, SLOT(onLvLoopEnd()), Qt::DirectConnection);
    QObject::connect(lvLoopThreaded, SIGNAL(lvLoopStart()), this, SLOT(onLvLoopStart()), Qt::DirectConnection);
    lvLoopThreaded->moveToThread(thread);
    thread->start();

    camActive = true;
//    nullFocus();
}

void bCamera::onLvLoopTimer() {
    if(viewfinderBusy)return;

    int retval;

    CameraFile *cf;
    retval = gp_file_new (&cf);

    retval = gp_camera_capture_preview(camera, cf, context);

    char *cfData;
    unsigned long int size;
    retval = gp_file_get_data_and_size (cf, &cfData, &size);

//-- resize cfData ---


//    tjhandle _jpegDecompressor = tjInitDecompress();

//    int jpegSubsamp, width, height;
//    tjDecompressHeader2(_jpegDecompressor, cfData, size, &width, &height, &jpegSubsamp);

//    width= 960;
//    height = 640;

//    width= 640;
//    height = 426;

//    free(currentFrameBuffer);
//    currentFrameBuffer = (unsigned char*)malloc(width*height*tjPixelSize[TJPF_RGB]);
//    int res = tjDecompress2(_jpegDecompressor, cfData, size, currentFrameBuffer, width,  0 /*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);


//-- /resize ---

    QByteArray a(cfData, size);
    retval = gp_file_unref (cf);
    a.append("--cutjpeg--");

    onLvLoopFrameReady(a);

//    qDebug() << "captured LvLoop frame";
}

void bCamera::onLvLoopFrameReady(QByteArray frame){
//    qDebug() << "reemintting frameRady(...)";
    emit frameReady(frame);
}

void bCamera::onViewfinderCameraFrameReady(QByteArray frame){
//    qDebug() << "reemintting frameRady(...)";
    emit frameReady(frame);
}

void bCamera::freeCam(void){
    camIsNikon = false;
    camIsCanon = false;

    if(!camActive)return;
    
    deactivateLiveView();
    
    gp_camera_exit(camera, context);
    std::cout << "camera closed\r\n" << std::flush;
//    libusb_reset_device(devh);
    camActive = false;
}

bool bCamera::setUsbPower(unsigned int status) {
    if(status == 0) {

        qDebug() << "USB power off";
        usbSuspended = true;
        freeCam();
//        freeUSB();

        usleep(1 * 1000 * 1000);
        system("/usr/bin/devmem2 0x47401c60 b 0x00");
    }
    else {
        qDebug() << "USB power on";
        system("/usr/bin/devmem2 0x47401c60 b 0x01");
        usleep(1 * 1000 * 1000);

        initUSB();
        usbSuspended = false;
    }

    qDebug() << "USB power done";
    return(true);
}

unsigned int bCamera::getUsbPower(void) {
    if(!camActive)return(0);
    return(1);
}

void bCamera::nullFocus() {
//    triggerAf(true);
//    triggerAf(false);
    
    int c = 0;

    setConfig("viewfinder", "1", 2);
    
    
//    activateLiveView();
//    usleep(2 * 1000 * 1000);
    
    setConfig("manualfocusdrive", "-32767", 0);
    focusValue = 0;
    
//    deactivateLiveView();

    

    setConfig("viewfinder", "0", 2);
}

bCamera::~bCamera() {
    camLookupTerminator = true;
    camLookupThread->join();
    
    freeCam();
    freeUSB();
}

void bCamera::focusUp() {
    if(setConfig("manualfocusdrive", "30", 0)) {
        focusValue += 30;
    }
}

void bCamera::focusDown() {
    if(setConfig("manualfocusdrive", "-30", 0)) {
        focusValue -= 30;
    }
}

void bCamera::focusUpMuch() {
    if(setConfig("manualfocusdrive", "300", 0)) {
        focusValue += 300;
    }
}

void bCamera::focusDownMuch() {
    if(setConfig("manualfocusdrive", "-300", 0)) {
        focusValue -= 300;
    }
}

void bCamera::focusSetValue(unsigned int val) {
    int delta = val - focusValue;
    if(setConfig("manualfocusdrive", std::to_string(delta), 0)) {
        focusValue = val;
    }
}

unsigned int bCamera::getFocusValue() {
//    unsigned int fv = focusValue * ((double)focusClientRange/(double)focusCameraRange);
    unsigned int fv = focusValue;
//    std::cout << "current focus value: " << fv << "\r\n" << std::flush;
    return fv;
}

void bCamera::initLiveView() {

}


void bCamera::setAutofocusarea(std::string value){
    if(camActive)lvLoopThreaded->incBlockers();
    if(setConfig("autofocusarea", value, 4)) {
        camInfo.autofocusarea = value;
    }
    if(camActive)lvLoopThreaded->decBlockers();
}

bool bCamera::setConfig(std::string key, std::string val, unsigned int inputType, CameraEventType eventType, int eventTimeout, QString eventData) {
    if(!camActive)return(false);
    
    std::cout << "setting cam: " << key << "=" << val << "\r\n" << std::flush;

//    lvLoopThreaded->stop();
//    if(lvRunning)pauseLvLoop();

    int ret;
    CameraWidget *widget = NULL, *child = NULL;

    bool s = false;
//    usleep(0.2 * 1000 * 1000);
    do {
        int busy_wait_retries = 0;
        do {
            ret = gp_camera_get_config (camera, &widget, context);
            if(ret == GP_OK)continue;
            std::cout << "cam busy, waiting\r\n" << std::flush;;
            usleep(0.1 * 1000 * 1000);
            busy_wait_retries++;
        } while ((ret == GP_ERROR_CAMERA_BUSY) && (busy_wait_retries < 50));

        if(ret != GP_OK) {
            std::cout << "error: gp_camera_get_config: \r\n" << ret << std::flush;
            continue;
        }
        
        ret = gp_widget_get_child_by_name (widget, key.c_str(), &child);
        if(ret != GP_OK) {
            std::cout << "error: gp_widget_get_child_by_name\r\n" << std::flush;
            continue;
        }
        
        if (inputType == 2){
            int v_int = std::stoi(val);
            ret = gp_widget_set_value (child, &v_int);
        }
        else if (inputType == 3) {
            char v_char[20];
            memcpy(v_char, val.c_str(), 20);
            ret = gp_widget_set_value (child, &v_char);
        }
        else if (inputType == 4) {
            int cnt, i;
            
            cnt = gp_widget_count_choices (child);
            
            if( std::stoi(val) > cnt )continue;
            
            const char *choice;
            ret = gp_widget_get_choice (child, std::stoi(val), &choice);
            gp_widget_set_value (child, choice);
        }
        else {
            float v_float = std::stof(val);
            ret = gp_widget_set_value (child, &v_float);
        }
        if(ret != GP_OK) {
            std::cout << "error: gp_widget_set_value\r\n" << std::flush;
            continue;
        }
        
        
        ret = gp_camera_set_config(camera, widget, context);
        
        if(ret != GP_OK) {
            std::cout << "error: gp_camera_set_config\r\n" << std::flush;
            continue;
        }
        
        s = true;
        
    } while(false);

    if( eventTimeout != 0 ) {
        CameraEventType	evtype;
        void *data;
        int retval;

        for(int i=0; i<100; i++) {
            data = NULL;
            retval = gp_camera_wait_for_event(camera, eventTimeout, &evtype, &data, context);
            std::cout << "event caught: " << evtype << ", data: " << "hidden" << "\r\n" <<  std::flush;
            if(data) {
                std::cout << "data: " << (char*)data << "\r\n" <<  std::flush;
            }

            QString edata((char*)data);

            if(evtype==GP_EVENT_TIMEOUT) {
                std::cout << "event timeout\r\n" << std::flush;
                break;
            }
            if(evtype==eventType && edata==eventData) {
                std::cout << "event match\r\n" << std::flush;
                break;
            }
        }
    }
    
    ret = gp_widget_free(widget);
    
//    if(lvRunning)unPauseLvLoop();

//    lvLoopThreaded->start();
    
    return(s);
}

void bCamera::onLvLoopEnd() {
//    usleep(0.1 * 1000 * 1000);

    int retval = gp_camera_capture_preview_done(camera, context);

    viewfinderBusy = true;

    setConfig("capturetarget", "1", 4);

}

void bCamera::onLvLoopStart() {
    lvFirstImage = true;

    /*
    CameraEventType	evtype;
    void *data;
    int retval;

    for(int i=0; i<1000; i++) {
        data = NULL;
        retval = gp_camera_wait_for_event(camera, 500, &evtype, &data, context);

        if(evtype==GP_EVENT_FILE_ADDED) {
            std::cout << "file added: " << (char*)data << "\r\n" <<  std::flush;
        }
        if(evtype==GP_EVENT_TIMEOUT)break;
    }
*/
    viewfinderBusy = false;
}

std::string bCamera::getConfig(std::string key, unsigned int inputType, bool getChoice) {
    if(!camActive)return("");
    
    int v_int;
    char v_char[20];
    float v_float;
    
    std::string retVal;
    char *wval;
    int ival;
    const char ** choice;
    
//    lvLoopThreaded->stop();

    int ret;
    CameraWidget *widget = NULL, *child = NULL;
    
    ret = gp_camera_get_config (camera, &widget, context);
    if(ret != GP_OK)return("");
    std::cout << ret << "?" << GP_OK << "\r\n" << std::flush;
    
    ret = gp_widget_get_child_by_name (widget, key.c_str(), &child);
    if(ret != GP_OK)return("");
    std::cout << ret << "?" << GP_OK << "\r\n" << std::flush;
    
    /*
    if (inputType == 2) {
        ret = gp_widget_get_value (child, &wval);
        retVal = v_int;
    }
    if (inputType == 3) {
        ret = gp_widget_get_value (child, &wval);
        retVal = v_char;
    }
    else {
        ret = gp_widget_get_value (child, &wval);
        retVal = v_float;
    }
     */
    
    if(inputType==5) {
        ret = gp_widget_get_value (child, &v_float);
    }
    else {
        ret = gp_widget_get_value (child, &wval);
    }
    
    if(inputType==4) { //radio
        int cnt = gp_widget_count_choices(child);
        for ( int i=0; i<cnt; i++) {
            const char *choice;
            ret = gp_widget_get_choice (child, i, &choice);
            
            if (!strcmp (choice, wval)) {
                sprintf(wval, "%d", i);
                break;
            }
        }
    }
    
    if(inputType==5) {
        retVal = std::to_string((int)ceil(v_float));
    }
    else {
        retVal = std::string(wval);
    }

       if(ret != GP_OK)return("");

    
    std::cout << "got: " << retVal << "\r\n" << std::flush;
    
    gp_widget_free(widget);
//    if(lvRunning)unPauseLvLoop();
//    lvLoopThreaded->start();

    return retVal;
}

void bCamera::pauseLvLoop(){
    std::cout << "pausing LV loop\r\n" << std::flush;
    if(pause) {
        std::cout << "already paused\r\n" << std::flush;
        return;
    }
    
    isPaused = false;
    
    pause = true;
    while(!isPaused){};
    std::cout << "paused LV loop\r\n" << std::flush;
}

void bCamera::unPauseLvLoop(){
    std::cout << "unpausing LV loop\r\n" << std::flush;

    pause = false;
}

QByteArray bCamera::captureLvFrame(void) {
    int retval;

    CameraFile *cf;
    retval = gp_file_new (&cf);

    retval = gp_camera_capture_preview(camera, cf, context);

    char *cfData;
    unsigned long int size;
    retval = gp_file_get_data_and_size (cf, &cfData, &size);

    QByteArray a(cfData, size);
    retval = gp_file_unref (cf);
    a.append("--cutjpeg--");
    return(a);
}

void bCamera::lvLoop() {
    pause = false;
    
    int retval;    
    isPaused = false;

    while(1){
        if(terminator==true)break;
        
        if(pause==false) {
            isPaused = false;

            QByteArray a = captureLvFrame();
            emit frameReady(a);
       }
        else {
            isPaused = true;
        }
        
        qDebug() << "frame";
        usleep(0.04*1000*1000);
    }
    
    pause = true;
}

bool bCamera::activateLiveView() {
    if(!camActive)return(false);

    lvLoopThreaded->enable();

    lvActive = true;
    
    return(true);
}

bool bCamera::deactivateLiveView() {
    if(!camActive)return(false);

    lvLoopThreaded->disable();

    lvActive = false;
    
    return(true);
}

bool bCamera::getLvActive() {
    return lvActive;
}

bool bCamera::getVfActive() {
    return vfActive;
}

void bCamera::captureGPIO(uint shutterSpeedMsec, uint holdMsec) {
    if(camActive)lvLoopThreaded->incBlockers();

    afPin->setValue(1);
    usleep(1000);
    srPin->setValue(1);
    usleep(1000);

    usleep(holdMsec * 1000);

    srPin->setValue(0);
    afPin->setValue(0);
    
    if(camActive)lvLoopThreaded->decBlockers();

    usleep(shutterSpeedMsec * 1000);
}

void bCamera::afControl(float timeLimit) {
    int i = timeLimit / 0.1;
    while(i--){
        usleep(0.1 * 1000 * 1000);
        if(afOn==false)return;
    }
    if(afOn==true)triggerAf(false);
}

void bCamera::srControl(float timeLimit) {
    int i = timeLimit / 0.1;
    while(i--){
        usleep(0.1 * 1000 * 1000);
        if(srOn==false)return;

    }
    if(srOn==true)triggerSr(false);
}

void bCamera::triggerAf(bool value, float timeLimit) {
    std::cout << "trigger AF\r\n" << std::flush;
    if(afOn==value)return;
    if(value == true){
        if(camActive)lvLoopThreaded->incBlockers();
        afPin->setValue(1);
        afOn = true;
        afControlThread = new std::thread( &bCamera::afControl, this, timeLimit );
        afControlThread->detach();
        return;
    }
    afPin->setValue(0);
    if(camActive)lvLoopThreaded->decBlockers();
    afOn = false;
}

/*
void bCamera::triggerSr(bool value, float timeLimit) {
    std::cout << "trigger SR - test\r\n" << std::flush;

    if(value == true){
        viewfinderBusy = true;
        setConfig("eosremoterelease", "Press Full", 3);
        return;
    }
    viewfinderBusy = false;
    setConfig("eosremoterelease", "Release Full", 3);
}
*/

void bCamera::triggerSr(bool value, float timeLimit) {
    std::cout << "trigger SR\r\n" << std::flush;
    if(srOn==value)return;
    if(value == true){
        if(camActive)lvLoopThreaded->incBlockers();
        srPin->setValue(1);
        srOn = true;
        srControlThread = new std::thread( &bCamera::srControl, this,  timeLimit);
        srControlThread->detach();
        return;
    }
    srPin->setValue(0);
    if(camActive)lvLoopThreaded->decBlockers();
    srOn = false;
}


void bCamera::capture() {
    CameraEventType	evtype;
	void		*data;
	int		retval;
    CameraFilePath * 	path;
    
    path = new CameraFilePath;


    retval =  gp_camera_capture	(camera, GP_CAPTURE_IMAGE, path, context );
    
//    retval = gp_camera_trigger_capture (camera, context);
    
    if ((retval != GP_OK) && (retval != GP_ERROR) && (retval != GP_ERROR_CAMERA_BUSY)) {
        fprintf(stderr,"triggering capture had error %d\n", retval);
    }
    
    for(int i=0; i<100; i++) {
        retval = gp_camera_wait_for_event(camera, 10000, &evtype, &data, context);
        std::cout << evtype << "\n" <<  std::flush;
        if(evtype==GP_EVENT_CAPTURE_COMPLETE)break;
    }

    
    if (retval != GP_OK) {
		fprintf (stderr, "return from waitevent in trigger sample with %d\n", retval);
	}
}


void bCamera::getCameraFile(std::string filename) {
    if(!camActive)return;
    
    CameraList *list;
    gp_list_new (&list);
    const char *name;
    int fd, retval;
    CameraFile *file;
    CameraFilePath  *path;
    
    gp_camera_folder_list_files (camera, "/store_00020001/DCIM/231NC_D4", list, context);
    
    
    int count = gp_list_count (list);
    std::cout << "count: " << count << "\r\n" << std::flush;

    int i;
    for (i = 0; i < count; i++) {
        gp_list_get_name (list, i, &name);
        std::cout << "name: " << name << "\r\n" << std::flush;
        
        timespec tsb, tsa;
        
        fd = open("/home/korytov/test.jpg", O_CREAT | O_WRONLY, 0644);
        retval = gp_file_new_from_fd(&file, fd);
        std::cout << "getting file" << "\r\n" << std::flush;
        clock_gettime(CLOCK_REALTIME, &tsb);
        retval = gp_camera_file_get(camera, "/store_00020001/DCIM/231NC_D4", name,
                                    GP_FILE_TYPE_NORMAL, file, context);
        clock_gettime(CLOCK_REALTIME, &tsa);
        
        
        unsigned long tbefore = tsb.tv_sec * 1000000000 + tsb.tv_nsec;;
        unsigned long tafter = tsa.tv_sec * 1000000000 + tsa.tv_nsec;
        double elapsed = 1.0*(tafter - tbefore) / 1000000000;
        std::cout << "...done in " << elapsed << "\r\n" << std::flush;
        gp_file_free(file);

    }
}

void bCamera::setCamParamFocusmetermode(std::string value) {
    setConfig("focusmetermode", value, 3);
    camInfo.focusmetermode = value;
}

void bCamera::setCamParamIso(std::string value) {
    setConfig("iso", value, 3);
    camInfo.iso = value;
}

void bCamera::setCamParamExp(std::string value) {
    setConfig("exposurecompensation", value, 3);
    camInfo.exp = value;
}

void bCamera::setCamParamAf(std::string value) {
//    setConfig("af", value, 3);
//    camInfo.af = value;
}

void bCamera::setCamParamMode(std::string value) {
    setConfig("expprogram", value, 3);
    camInfo.mode = value;
}

void bCamera::setCamParamD(std::string value) {
    setConfig("f-number", value, 3);
    camInfo.d = value;
}

void bCamera::setCamParamS(std::string value) {
    setConfig("shutterspeed2", value, 3);
    camInfo.s = value;
}

void bCamera::updateCamInfo(){
    if(!camActive)return;

    std::cout << "getting cam info\r\n" << std::flush;
    
    camInfo.iso = getConfig("iso", 3);
    camInfo.mode = getConfig("expprogram", 3);
    camInfo.d = getConfig("f-number", 3);
    camInfo.s = getConfig("shutterspeed2", 3);
    camInfo.exp = getConfig("exposurecompensation", 3);
    camInfo.autofocusarea = getConfig("autofocusarea", 4);
    camInfo.lightmeter = getConfig("lightmeter", 5);
    camInfo.focusmetermode = getConfig("focusmetermode", 3);
}

camInfoType bCamera::getCamInfo() {
    return camInfo;
}

void bCamera::flushCamera(int downloadNumber, bool *camFlushTerminator) {
    if(!camActive)return;
    lvLoopThreaded->incBlockers();

    flushTerminator = NULL;
    if(camFlushTerminator != NULL)flushTerminator = camFlushTerminator;
    
    gp_camera_unref(camera);
    gp_camera_new(&camera);
    gp_camera_init(camera, context);
    
    downloadLimit = downloadNumber;
    downloadCameraDir("");
    
    lvLoopThreaded->decBlockers();
}


void bCamera::downloadCameraDir(std::string dirname) {
    if(!camActive)return;
    
    CameraList *dlist, *flist;
    gp_list_new (&flist);
    gp_list_new (&dlist);
    const char *dname, *fname;
    int fd, retval;
    CameraFile *file;
    CameraFilePath  *path;
    
    std::string pwdStr;
    
    
    pwd.push_back(dirname);
    for (int i = 0; i < pwd.size(); i++) {
        pwdStr += pwd[i];
        pwdStr += "/";
    }
    
    std::cout << "now at: " << pwdStr << "\r\n" << std::flush;
    std::cout << "files at: " << pwdStr << "\r\n" << std::flush;
    gp_camera_folder_list_files (camera, pwdStr.c_str(), flist, context);
    
    int fcount = gp_list_count (flist);
    std::cout << "fcount: " << fcount << "\r\n" << std::flush;

    QString mess;
    for (int j = fcount-1; j >= 0; j--) {
        if(flushTerminator==true || downloadLimit==0)continue;
            
        gp_list_get_name (flist, j, &fname);

        mess = QString("fname: ") + QString(fname) + QString("\r\n");
        std::cout << mess.toStdString() << std::flush;
        emit(logToClient(mess));
        
        std::string targetFn = camDownloadDir + "/" + std::string(fname);

        fd = open(targetFn.c_str(), O_CREAT | O_WRONLY, 0644);

        retval = gp_file_new_from_fd(&file, fd);
        
        mess = "getting file\r\n";
        std::cout << mess.toStdString() << std::flush;
        emit(logToClient(mess));

        retval = gp_camera_file_get(camera, pwdStr.c_str(), fname,
                                    GP_FILE_TYPE_NORMAL, file, context);

        mess = "got file\r\n";
        std::cout << mess.toStdString() << std::flush;
        emit(logToClient(mess));

        downloadLimit--;
        close(fd);
        gp_file_free(file);
        if(retval == GP_OK)retval = gp_camera_file_delete(camera, pwdStr.c_str(), fname, context);
    }
    gp_list_free(flist);
    
    gp_camera_folder_list_folders (camera, pwdStr.c_str(), dlist, context);
    
    int count = gp_list_count (dlist);
    std::cout << "dcount here: " << count << "\r\n" << std::flush;
    for (int i = count-1; i >=0; i--) {
        gp_list_get_name (dlist, i, &dname);
        std::cout << "going to: " << dname << "\r\n" << std::flush;
        
        downloadCameraDir(std::string(dname));
        
    }
    gp_list_free(dlist);
    pwd.pop_back();
    
}



