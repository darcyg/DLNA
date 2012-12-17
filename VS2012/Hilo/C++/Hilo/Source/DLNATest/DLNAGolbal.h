//
//  DLNAGolbal.h
//  Genie
//
//  Created by cs Siteview on 11-11-24.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//
#ifndef _DLNAGLOBAL_H_
#define _DLNAGLOBAL_H_

enum {
    DLNAMediaObjTypeVideo,
    DLNAMediaObjTypeAudio,
    DLNAMediaObjTypeImage,
};

typedef int DLNAMediaObjType;


enum {
	DLNAPageServer,
	DLNAPageRender,
	DLNAPageDevice,
	DLNAPageOption,
};

typedef int DLNAPage;


enum{
    UnlockModeAuto,
    UnLockModeManual,
};

typedef int UnLockMode;

enum
{
    DLNAControlSignalNone,
    DLNAControlSignalPlay,
    DLNAControlSignalPause,
    DLNAControlSignalStop,
};
typedef int DLNAControlSignal;

enum{
    DLNAMediaPlayBackStateStop,
    DLNAMediaPlayBackStatePlaying,
    DLNAMediaPlayBackStatePause,
    DLNAMediaPlayBackStateErr,
    DLNAMediaPlayBackStateNone,
};

typedef int DLNAMediaPlayBackState;


enum 
{
    DLNAOptionSettingServerSwitch,
    DLNAOptionSettingRenderSwitch,
};
#endif