//
//  btypes.h
//  bone
//
//  Created by korytov on 06/05/14.
//  Copyright (c) 2014 korytov. All rights reserved.
//

#define MAX_SPD 300.0
#define KVAL_HOLD 200
#define KVAL_ACC 200
#define KVAL_DEC 200
#define KVAL_RUN 200
#define INT_SPD 0x18C6
#define ST_SLP 0x16
#define SLP_ACC 0x62
#define SLP_DEC 0x62
#define M_STP 128
#define MOT_ACC 100.0

#ifndef bone_btypes_h
#define bone_btypes_h

#include <vector>
#include <map>

struct l6470Status {
    int dir;
    int position;
    int isMoving;
    bool nowWatched;
    int powerStatus;
    int stalled;
    bool expired;
};

struct l6470Setup {
    uint8_t m_stp = M_STP;
    float max_spd = MAX_SPD;
    float mot_acc = MOT_ACC;
    uint32_t kval_hold = KVAL_HOLD;
    uint32_t kval_run = KVAL_RUN;
    uint32_t int_spd = INT_SPD;
    uint32_t st_slp = ST_SLP;
    uint32_t slp_acc = SLP_ACC;
};

struct fixedPoint {
    int pan;
    int tilt;
    int slider;
    int focus;
    int mfocus;
    int zoom;
    
    int timelapseMember;
    
    std::string name;
    std::string id;
};

typedef std::map<std::string, fixedPoint> fixedPoints;
typedef std::vector<fixedPoint> fixedPointsVector;

struct tlParams {
    unsigned int tlDelay;
    unsigned int tlFrames;
    short tlDirection;
    unsigned int tlProgress;
    unsigned int tlShutterdelay;
    short tlMirrorup;
    unsigned int tlStabdelay;
};

struct tHash {
    std::string dev;
    std::string key;
    std::string value;
    std::vector<std::string> params;
};

struct tinyStatus {
    tHash positionPan;
    tHash positionTilt;
    tHash positionSlider;
};

struct heavyStatus {
    tHash limitPanLow;
    tHash limitPanHigh;
    tHash limitTiltLow;
    tHash limitTiltHigh;
    tHash limitSliderLow;
    tHash limitSliderHigh;
    
    tHash rangePan;
    tHash rangeTilt;
    tHash rangeSlider;
};

struct camInfoType {
    std::string iso;
    std::string mode;
    std::string af;
    std::string d;
    std::string s;
    std::string exp;
    std::string autofocusarea;
    std::string lightmeter;
    std::string focusmetermode;
};

struct limits {
  int minSlider;
  int maxSlider;
  int minPan;
  int maxPan;
  int minTilt;
  int maxTilt;
  int minZoom;
  int maxZoom;
};

#endif
