//
//  DLNAShareApi.h
//  Genie
//
//  Created by cs Siteview on 11-9-15.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//


#ifndef DLNASHAREAPI_H
#define DLNASHAREAPI_H
@class UIViewController;

#ifdef __cplusplus
extern "C" {
#endif

void beginDLNAService(UIViewController* controller);
void startUPNPService();
void stopUPNPService();
void saveDLNAConfigInfo();
bool needstartUPNPService();

#ifdef __cplusplus
}
#endif
    
#endif