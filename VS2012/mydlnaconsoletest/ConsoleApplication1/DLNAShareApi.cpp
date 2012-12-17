//
//  DLNAShareApi.c
//  Genie
//
//  Created by cs Siteview on 11-9-15.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "DLNAShareApi.h"
#import "DLNACenter.h"
#import "DLNADelegation.h"
#import "DLNAGolbal.h"
#import <UIKit/UIKit.h>

static bool dlnaServerOpenFlag = false;

void beginDLNAService(UIViewController* controller)
{
    DLNACenter * dlnaCenter = [[DLNACenter alloc] init];
    [dlnaCenter layoutDLNACenterOnController:controller];
    [dlnaCenter release];
    if (!dlnaServerOpenFlag)
    {
        dlnaServerOpenFlag = true;
        startUPNPService();
    }
}


void startUPNPService()
{
    if (needstartUPNPService())
    {
        DLNADelegation::GetInstance()->startUPNPServer();
    }
}


void stopUPNPService()
{
    if (DLNADelegation::isNull())
    {
        return;
    }
    DLNADelegation::GetInstance()->stopUPNPServer();
    DLNADelegation::ReleaseInstance();
}

void saveDLNAConfigInfo()
{
    if (DLNADelegation::isNull())
    {
        return;
    }
    DLNADelegation::GetInstance()->writeConfig();
    DLNADelegation::GetInstance()->WriteDLNAUserConfig();
}

bool needstartUPNPService()
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString * homeDic = [paths objectAtIndex:0];
    NSString * filePath = [NSString stringWithFormat:@"%@/DLNAOptionSetting.info",homeDic];
    NSFileManager * fileManager = [NSFileManager defaultManager];
    if ( ![fileManager fileExistsAtPath:filePath] )
    {
        return true;
    }
    NSMutableArray * tmp = [NSMutableArray arrayWithContentsOfFile:filePath];
    if (!tmp)
    {
        return true;
    }
    NSString * str = (NSString*)[tmp objectAtIndex:DLNAOptionSettingServerSwitch];
    if ([str isEqualToString:@"0"])
    {
        return false;
    }
    else
    {
        return true;
    }
}
