//
//  DLNACenter.m
//  Genie
//
//  Created by cs Siteview on 11-9-14.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#include "DLNACenter.h"
#import "DLNADelegation.h"
#import "DLNARenderList.h"
#import "DLNAServerList.h"
#import "DLNADeviceController.h"
#import "DLNAOptionController.h"

@implementation DLNACenter
@synthesize serverTitle = m_serverTitle;
#pragma mark memory
- (void)dealloc
{
    self.serverTitle = nil;
    [super dealloc];
}
#pragma uialertView delegate
- (void)willPresentAlertView:(UIAlertView *)alertView
{
    //for import photos
    UIActivityIndicatorView * aiv = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
    aiv.center = CGPointMake(alertView.bounds.size.width/2.0f, alertView.bounds.size.height/2.0f);
    [aiv startAnimating];
    [alertView addSubview:aiv];
    [aiv release];
}
#pragma mark view life-style
- (void)viewDidLoad
{
    [super loadView];
    NSArray * imgs = [NSArray arrayWithObjects:[UIImage imageNamed:@"browse.png"],
                      [UIImage imageNamed:@"device.png"],
                      [UIImage imageNamed:@"playing.png"],
                      [UIImage imageNamed:@"option.png"],nil];
    
    UITabBarItem *  barItem = nil;
    
    DLNAServerList * serverList = [[DLNAServerList alloc] init];
    serverList.delegate = self;
    serverList.title = NSLocalizedStringFromTable(@"tabBarLabelSource", @"Localizable", nil);
    barItem = [[UITabBarItem alloc]
               initWithTitle:NSLocalizedStringFromTable(@"tabBarLabelSource", @"Localizable", nil) image:[imgs objectAtIndex:0] tag:0];
    serverList.tabBarItem = barItem;
    [barItem release];
    UINavigationController * serverListNavi = [[UINavigationController alloc] initWithRootViewController:serverList];
    serverListNavi.navigationBar.barStyle = UIBarStyleBlack;
    [self createBackBtn:serverList.navigationItem];
    [serverList release];
    
    DLNARenderList * renderList = [[DLNARenderList alloc] init];
    renderList.delegate = self;
    renderList.title = NSLocalizedStringFromTable(@"tabBarLabelPlayer", @"Localizable", nil);
    barItem = [[UITabBarItem alloc]
               initWithTitle:NSLocalizedStringFromTable(@"tabBarLabelPlayer", @"Localizable", nil) image:[imgs objectAtIndex:1] tag:0];
    renderList.tabBarItem = barItem;
    [barItem release];
    UINavigationController * renderListNavi = [[UINavigationController alloc] initWithRootViewController:renderList];
    renderListNavi.navigationBar.barStyle = UIBarStyleBlack;
    [self createBacktoBrowsePageBtn:renderList.navigationItem];
    [renderList release];
    
    DLNADeviceController * device = [[DLNADeviceController alloc] init];
    device.delegate = self;
    device.title = NSLocalizedStringFromTable(@"tabBarLabelPlaying", @"Localizable", nil);
    barItem = [[UITabBarItem alloc]
               initWithTitle:NSLocalizedStringFromTable(@"tabBarLabelPlaying", @"Localizable", nil) image:[imgs objectAtIndex:2] tag:0];
    device.tabBarItem = barItem;
    [barItem release];
    UINavigationController * deviceNavi = [[UINavigationController alloc] initWithRootViewController:device];
    deviceNavi.navigationBar.barStyle = UIBarStyleBlack;
    [self createBacktoBrowsePageBtn:device.navigationItem];
    [device release];
    
    ///
    /*UIViewController * option = [[UIViewController alloc] init];
     option.title = @"Option";
    
    UIView * testview = [[UIView alloc] initWithFrame:[UIScreen mainScreen].applicationFrame];
    UILabel * l1 = [[UILabel alloc] initWithFrame:CGRectMake(50, 100, 100, 50)];
    l1.text = @"control point";
    UISwitch * switchCP = [[UISwitch alloc] initWithFrame:CGRectMake(200, 100, 100,50)];
    switchCP.on = YES;
    [switchCP addTarget:self action:@selector(switchCP:) forControlEvents:UIControlEventValueChanged];
    
    UILabel * l2 = [[UILabel alloc] initWithFrame:CGRectMake(50, 200, 100, 50)];
    l2.text = @"server";
    UISwitch * switchServer = [[UISwitch alloc] initWithFrame:CGRectMake(200, 200, 100,50)];
    switchServer.on = YES;
    [switchServer addTarget:self action:@selector(switchServer:) forControlEvents:UIControlEventValueChanged];
    
    UILabel * l3 = [[UILabel alloc] initWithFrame:CGRectMake(50, 300, 100, 50)];
    l3.text = @"render";
    UISwitch * switchRender = [[UISwitch alloc] initWithFrame:CGRectMake(200, 300, 100,50)];
    switchRender.on = YES;
    [switchRender addTarget:self action:@selector(switchRender:) forControlEvents:UIControlEventValueChanged];
    
    UILabel * l4 = [[UILabel alloc] initWithFrame:CGRectMake(50, 400, 100, 50)];
    l4.text = @"all";
    UISwitch * switchAll = [[UISwitch alloc] initWithFrame:CGRectMake(200, 400, 100,50)];
    switchAll.on = YES;
    [switchAll addTarget:self action:@selector(switchAll:) forControlEvents:UIControlEventValueChanged];
    
    [testview addSubview:switchCP];
    [switchCP release];
    [testview addSubview:l1];
    [l1 release];
    
    [testview addSubview:switchServer];
    [switchServer release];
    [testview addSubview:l2];
    [l2 release];
    
    [testview addSubview:switchRender];
    [switchRender release];
    [testview addSubview:l3];
    [l3 release];
    
    [testview addSubview:switchAll];
    [switchAll release];
    [testview addSubview:l4];
    [l4 release];
    
    option.view = testview;
    [testview release];*/
    
    DLNAOptionController * option = [[DLNAOptionController alloc] init];
    option.title = NSLocalizedStringFromTable(@"titleOptions", @"Localizable", nil);
    option.delegate = self;
     barItem = [[UITabBarItem alloc]
                initWithTitle:NSLocalizedStringFromTable(@"titleOptions", @"Localizable", nil) image:[imgs objectAtIndex:3] tag:0];
     option.tabBarItem = barItem;
     [barItem release];
     UINavigationController * optionNavi = [[UINavigationController alloc] initWithRootViewController:option];
     optionNavi.navigationBar.barStyle = UIBarStyleBlack;
     [self createBacktoBrowsePageBtn:option.navigationItem];
     [option release];

    [self setViewControllers:[NSArray arrayWithObjects:serverListNavi,renderListNavi,deviceNavi,optionNavi, nil]];
    [serverListNavi release];
    [renderListNavi release];
    [deviceNavi release];
    [optionNavi release];
    
    self.delegate = self;
}

#pragma option test func
- (void) switchCP:(id)switcher
{
    UISwitch * sw = (UISwitch*)switcher;
    if (sw.on)
    {
        DLNADelegation::GetInstance()->startUPNPControlPoint();
    }
    else
    {
        DLNADelegation::GetInstance()->stopUPNPControlpoint();
    }
}

- (void) switchServer:(id)switcher
{
    UISwitch * sw = (UISwitch*)switcher;
    if (sw.on)
    {
        DLNADelegation::GetInstance()->startUPNPServer();
    }
    else
    {
        DLNADelegation::GetInstance()->stopUPNPServer();
    }
}

- (void) switchRender:(id)switcher
{
    UISwitch * sw = (UISwitch*)switcher;
    if (sw.on)
    {
        DLNADelegation::GetInstance()->startUPNPRender();
    }
    else
    {
        DLNADelegation::GetInstance()->stopUPNPRender();
    }
}

- (void) switchAll:(id)switcher
{
    UISwitch * sw = (UISwitch*)switcher;
    if (sw.on)
    {
        DLNADelegation::GetInstance()->startUPNPServer();
        DLNADelegation::GetInstance()->startUPNPRender();
        DLNADelegation::GetInstance()->startUPNPControlPoint();
    }
    else
    {
        DLNADelegation::GetInstance()->stopUPNPServer();
        DLNADelegation::GetInstance()->stopUPNPRender();
        DLNADelegation::GetInstance()->stopUPNPControlpoint();
    }
}



#pragma mark share tool api
- (DLNAMediaObjType) getMediaObjType:(const deejay::DLNAObject*) media
{
    return DLNADelegation::GetInstance()->getMediaObjType(media);
}

- (BOOL) currentRenderIsSelf
{
    return DLNADelegation::GetInstance()->currentRenderIsSelf();
}

#pragma mark Foundation
- (void) layoutDLNACenterOnController:(UIViewController*)viewController
{
    [self startDLNAService];
    [viewController presentModalViewController:self animated:YES];
}

- (void) dismissDLNACenter
{
    DLNADeviceController * deviceController = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [deviceController performSelector:@selector(finishTimerForReleaseTrackInfo)];
    [deviceController performSelector:@selector(finishTimerForGetTrack)];
    [deviceController performSelector:@selector(finishAutoPlayBackPicture)];
    [deviceController performSelector:@selector(STOP)];
    [self stopDLNAService];
    [self dismissModalViewControllerAnimated:YES];
}

- (void) startDLNAService
{
    DLNADelegation::GetInstance()->setDelegate(self);
    DLNADelegation::GetInstance()->importPhotos();
    
    //
    DLNADelegation::GetInstance()->startCore();
    NSArray * arr = [self readOptionConfig];
    if (!arr || ![(NSString*)[arr objectAtIndex:DLNAOptionSettingRenderSwitch] isEqualToString:@"0"])
    {
        DLNADelegation::GetInstance()->startUPNPRender();
    }
    //
    DLNADelegation::GetInstance()->startUPNPControlPoint();
}

- (void) stopDLNAService
{
    DLNADelegation::GetInstance()->stopUPNPRender();
    //DLNADelegation::GetInstance()->stopUPNPControlpoint();
    DLNADelegation::GetInstance()->clearDelegate();
}

//option
- (void) refreshMediaSource
{
    DLNADelegation::GetInstance()->refreshMediaSource();
}
- (void) restartDLNAService
{
    DLNADelegation::GetInstance()->restartUPNP();
}
- (void) openDLNAServer
{
    DLNADelegation::GetInstance()->startUPNPServer();
}
- (void) closeDLNAServer
{
    DLNADelegation::GetInstance()->stopUPNPServer();
}

- (void) openDLNARender
{
    DLNADelegation::GetInstance()->startUPNPRender();
}
- (void) closeDLNARender
{
    DLNADelegation::GetInstance()->stopUPNPRender();
}

- (void) writeOptionConfigWithServerOn:(BOOL)serverOn renderOn:(BOOL)renderOn
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString * homeDic = [paths objectAtIndex:0];
    NSString * filePath = [NSString stringWithFormat:@"%@/DLNAOptionSetting.info",homeDic];
    NSFileManager * fileManager = [NSFileManager defaultManager];
    if ( ![fileManager fileExistsAtPath:filePath] )
    {
        [fileManager createFileAtPath:filePath contents:nil attributes:nil];
    }
    NSMutableArray * settings = [[NSMutableArray alloc] init];
    if (serverOn)
    {
        [settings addObject:@"1"];
    }
    else
    {
        [settings addObject:@"0"];
    }
    if (renderOn)
    {
        [settings addObject:@"1"];
    }
    else
    {
        [settings addObject:@"0"];
    }
    [settings writeToFile:filePath atomically:NO];
    [settings release];
} 
- (NSMutableArray*) readOptionConfig
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString * homeDic = [paths objectAtIndex:0];
    NSString * filePath = [NSString stringWithFormat:@"%@/DLNAOptionSetting.info",homeDic];
    NSFileManager * fileManager = [NSFileManager defaultManager];
    if ( ![fileManager fileExistsAtPath:filePath] )
    {
        return nil;
    }
    NSMutableArray * tmp = [NSMutableArray arrayWithContentsOfFile:filePath];
    return tmp;
}

#pragma mark tabBarController delegate
- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController
{
    DLNADeviceController * deviceController = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    if (![viewController isEqual:[self getDLNAPage:DLNAPageDevice]])
    {
        [deviceController resignCurrentPageView];
    }
    
    /*DLNAOptionController * optioncontroller = (DLNAOptionController*)[[self getDLNAPage:DLNAPageOption] topViewController];
    if ([viewController isEqual:[self getDLNAPage:DLNAPageOption]])
    {
        [optioncontroller becomeCurrentPageView];
    }*/
    [[(UINavigationController*)viewController topViewController] performSelector:@selector(becomeCurrentPageView)];
}

#pragma mark callback
- (void) notifyMediaRenderListChanged
{
    DLNADelegation::GetInstance()->traverseRenderList();
    //const deejay::DeviceDescList& dmrList = DLNADelegation::GetInstance()->getMediaRenderList();
    //[self reloadMeidaRenderList:dmrList withCurrentRenderID:DLNADelegation::GetInstance()->getCurrentRenderUUID().toString()];
}

- (void) notifyMediaServerListChanged
{
    const deejay::DeviceDescList& dmsList = DLNADelegation::GetInstance()->getMediaServerList();
    if (!dmsList.find(DLNADelegation::GetInstance()->getCurrentServerUUID()))
    {
        [self serverPagePopToRoot];
    }
    [self reloadMediaServerList:dmsList];
}

- (void) notifyMediaServerStatusChanged
{
    
}

- (void) notifyMediaRenderStatusChanged
{
    [self updateControlUI];
}

- (void) notifyTrackOpFinish
{
    int track = DLNADelegation::GetInstance()->onQueryMediaPositionFinished();
    [self refreshTrack:track];
}

///render
- (void) OnMediaRenderOpen:(NSMutableArray*) info
{
    NSString* url = [info objectAtIndex:0];
    NSString* typeStr = [info objectAtIndex:1];
    //NSString* metaData = [info objectAtIndex:2];
    DLNAMediaObjType type = DLNAMediaObjTypeVideo;
    if ([typeStr rangeOfString:@"image/"].length > 0)
    {
        type = DLNAMediaObjTypeImage;
    }
    else if ([typeStr rangeOfString:@"audio/"].length > 0)
    {
        type = DLNAMediaObjTypeAudio;
    }
    else if ([typeStr rangeOfString:@"video/"].length > 0)
    {
        type = DLNAMediaObjTypeVideo;
    }
    [self loadMediaWithType:type andURL:url];
}
- (void) OnMediaRenderPlay
{
    [self onRenderPlay];
}
- (void) OnMediaRenderPause
{
    [self onRenderPause];
}
- (void) OnMediaRenderStop
{
    [self onRenderStop];
}
- (void) OnMediaRenderSeekTo:(NSNumber*)timeInMillis
{
    [self onRenderSeekTo:[timeInMillis longLongValue]];
}
- (void) OnMediaRenderSetMute:(NSNumber*)mute
{
    [self onRenderSetMute:[mute boolValue]];
}
- (void) OnMediaRenderSetVolume:(NSNumber*)volume
{
    [self onRenderSetVolume:[volume intValue]];
}
#pragma mark UI

- (void) createBackBtn:(UINavigationItem*) navItem
{
    UIBarButtonItem * backItem = [[UIBarButtonItem alloc]   initWithTitle:NSLocalizedStringFromTable(@"back", @"Localizable", nil) 
                                                                  style:UIBarButtonItemStyleBordered
                                                                 target:self
                                                                 action:@selector(dismissDLNACenter)];
    navItem.leftBarButtonItem = backItem;
    [backItem release];
}

- (void) createBacktoBrowsePageBtn:(UINavigationItem*) navItem
{
    UIBarButtonItem * backItem = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedStringFromTable(@"back", @"Localizable", nil) 
                                                                  style:UIBarButtonItemStyleBordered
                                                                 target:self
                                                                 action:@selector(gotoBrowsePage)];
    navItem.leftBarButtonItem = backItem;
    [backItem release];
}

- (void) gotoBrowsePage
{
    DLNADeviceController * deviceController = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [deviceController resignCurrentPageView];
    [self selectDLNAPage:DLNAPageServer];
}

- (void) selectDLNAPage:(DLNAPage)page
{
    self.selectedIndex = page;
}

- (UINavigationController*) getDLNAPage:(DLNAPage)page
{
    return (UINavigationController*)[self.viewControllers objectAtIndex:page];
}

- (void) reloadMeidaRenderList:(const deejay::DeviceDescList&)data withCurrentRenderID:(const NPT_String&)renderId
{
    DLNARenderList * renderListController = (DLNARenderList*)[[self getDLNAPage:DLNAPageRender] topViewController];
    [renderListController reloadData:data withCurrentRenderID:renderId];
}

- (void) reloadMediaServerList:(const deejay::DeviceDescList&)mediaServerList
{
    UINavigationController * serverNavi = [self getDLNAPage:DLNAPageServer];
    DLNAServerList * serverController = [serverNavi.viewControllers objectAtIndex:0];
    [serverController reloadRootData:mediaServerList];
}

- (void) addMediaObjectDir:(const NPT_String&)dirTitle withList:(const deejay::DLNAObjectList&)mediaObjectList
{
    UINavigationController * serverNavi = [self getDLNAPage:DLNAPageServer];
    DLNAServerList *aController = [[DLNAServerList alloc] init];
    aController.headTitle = self.serverTitle;
    aController.delegate = self;
    aController.title = [NSString stringWithUTF8String:dirTitle.GetChars()];
    [aController reloadLevelData:mediaObjectList];
    [serverNavi pushViewController:aController animated:YES];
    [aController release];
}

- (void) addMediaObjectDir:(DLNAServerList*)controller
{
    UINavigationController * serverNavi = [self getDLNAPage:DLNAPageServer];
    [serverNavi pushViewController:controller animated:YES];
}

- (void) reloadCurrentMediaObjectDir:(const deejay::DLNAObjectList&)mediaObjectList
{
    DLNAServerList * currentMediaObjectDir = (DLNAServerList*)[[self getDLNAPage:DLNAPageServer] topViewController];
    [currentMediaObjectDir reloadLevelData:mediaObjectList];
}

- (void) serverPagePopToRoot
{
    UINavigationController * serverNavi = [self getDLNAPage:DLNAPageServer];
    [serverNavi popToRootViewControllerAnimated:YES];
}

- (void) setPlaylist
{
    DLNAServerList * currentMediaObjectDir = (DLNAServerList*)[[self getDLNAPage:DLNAPageServer] topViewController];
    deejay::DLNAObjectList* objectList = [currentMediaObjectDir getMediaObjList];
    NSInteger index = [currentMediaObjectDir selectedRow];
    DLNADeviceController * deviceController = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [deviceController setPlayList:*objectList withNowPlayIndex:index];
}

- (void) shouldSelectRenderForMedia:(const deejay::DLNAItem*)item
{
    //[self selectDLNAPage:DLNAPageRender];2012.2.4
    DLNARenderList * renderListController = (DLNARenderList*)[[self getDLNAPage:DLNAPageRender] topViewController];
    [renderListController shouldSelectRenderForMedia:item];
}


- (deejay::DeviceDescList) snapshotMediaRendererList
{
    return DLNADelegation::GetInstance()->getMediaRenderList();
}
- (deejay::DeviceDescList) snapshotMediaServerList
{
    return DLNADelegation::GetInstance()->getMediaServerList();
}

//..................
//1. control point
- (void) showControlPointPageWithMediaType:(DLNAMediaObjType)type
{
    [self selectDLNAPage:DLNAPageDevice];
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage loadControlPointViewWithMediaObjType:type];
}

//thumb img 2012.2.9
- (void) setThumbImg:(NSData*)imgData withType:(DLNAMediaObjType)type
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage setThumbImg:imgData withMediaObjType:type];
}

- (void) updateControlUI
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    deejay::ServiceDesc* m_avt = DLNADelegation::GetInstance()->findServiceByType("urn:schemas-upnp-org:service:AVTransport:*");
	deejay::ServiceDesc* m_rcs = DLNADelegation::GetInstance()->findServiceByType("urn:schemas-upnp-org:service:RenderingControl:*");
    
    if (m_rcs) 
    {
		const char *varNames[] = {"Volume","Mute",};
        NPT_List<NPT_String> nameList;
        NPT_List<NPT_String> valueList;
        for (size_t i = 0; i < sizeof(varNames) / sizeof(varNames[0]); i++) 
        {
            nameList.Add(varNames[i]);
        }
        
		if (NPT_SUCCEEDED(DLNADelegation::GetInstance()->queryStateVariables(m_rcs->serviceId(), nameList, valueList))) 
        {
			const NPT_String& varVolume = *valueList.GetItem(0);
			int volume;
            int maxVolume = 100;
            deejay::StateVariableDesc *varVolumeDesc = m_rcs->findStateVariable("Volume");
            if (varVolumeDesc && varVolumeDesc->hasAllowedValueRangeMaximum()) 
            {
                NPT_SUCCEEDED(varVolumeDesc->allowedValueRangeMaximum()->ToInteger(maxVolume));
            }
			if (NPT_SUCCEEDED(NPT_ParseInteger(varVolume, volume))) 
            {
				[devicePage receiveSetVolumeSignalWithVolume:volume maxVolume:maxVolume minVolume:0];
			}
            
			const NPT_String& varMute = *valueList.GetItem(1);
			if (varMute.Compare("1") == 0 || varMute.Compare("true", true) == 0) 
            {
				[devicePage receiveSetMuteSignal:YES];
			} 
            else 
            {
				[devicePage receiveSetMuteSignal:NO];
			}
		}
	}
    
	if (m_avt) 
    {
		const char *varNames[] = {"TransportState","CurrentTrackDuration","TransportStatus"};
        
		NPT_List<NPT_String> nameList;
		NPT_List<NPT_String> valueList;
		for (size_t i = 0; i < sizeof(varNames) / sizeof(varNames[0]); i++) 
        {
			nameList.Add(varNames[i]);
		}
        
		if (NPT_SUCCEEDED(DLNADelegation::GetInstance()->queryStateVariables(m_avt->serviceId(), nameList, valueList))) 
        {
			const NPT_String& varTransportState = *valueList.GetItem(0);
            DLNAControlSignal controlSignal = DLNAControlSignalNone;
			if (varTransportState.Compare("STOPPED", true) == 0) 
            {
				controlSignal = DLNAControlSignalStop;
			} 
            else if (varTransportState.Compare("PLAYING", true) == 0) 
            {
				controlSignal = DLNAControlSignalPlay;
			} 
            else if (varTransportState.Compare("PAUSED_PLAYBACK", true) == 0) 
            {
				controlSignal = DLNAControlSignalPause;
			} 
            else if (varTransportState.Compare("NO_MEDIA_PRESENT", true) == 0) 
            {
				
			} 
            else 
            {
				
			}
            [devicePage receiveUPnPControlSignal:controlSignal];
            
			const NPT_String& varCurrentTrackDuration = *valueList.GetItem(1);
			NPT_UInt64 duration;
			if (NPT_SUCCEEDED(deejay::DLNACore::parseTrackDurationString(varCurrentTrackDuration, duration))) 
            {
                [devicePage receiveSetTotalDuration:static_cast<int>(duration)];
			}
            
            const NPT_String& varTransportStatus = *valueList.GetItem(2); 
            if (varTransportStatus.Compare("OK") == 0)
            {
                NSLog(@"render report playback error");
            }
		}
	}
}

- (void) refreshTrack:(NSInteger)milliseconds
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage receiveTrackSignal:milliseconds];
}

- (void) autoPlaybackTimeOut
{
    [self selectDLNAPage:DLNAPageDevice];
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage autoPlaybackTimeOut];
}

//2.render
- (void) loadMediaWithType:(DLNAMediaObjType)type andURL:(NSString*)url
{
    [self selectDLNAPage:DLNAPageDevice];
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage loadMediaWithType:type andURL:url];
}
- (void) onRenderPlay
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage onRenderPlay];
}
- (void) onRenderPause
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage onRenderPause];
}
- (void) onRenderStop
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage onRenderStop];
}
- (void) onRenderSetMute:(BOOL)mute
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage onRenderSetMute:mute];
}
- (void) onRenderSeekTo:(long long) timeInMillis
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage onRenderSeekTo:timeInMillis];
}
- (void) onRenderSetVolume:(int) volume
{
    DLNADeviceController * devicePage = (DLNADeviceController*)[[self getDLNAPage:DLNAPageDevice] topViewController];
    [devicePage onRenderSetVolume:volume];
}


#pragma mark UPNP
- (void) openMediaServer:(deejay::DeviceDesc&)device
{
    DLNADelegation::GetInstance()->setActiveServer(device);
    [self browseServerOrDir:device.friendlyName() withDirID:"0"];
}

- (void) openDirectoryOrMeidaObj:(const deejay::DLNAObject&)obj
{
    const deejay::DLNAContainer *container = obj.asContainer();
	if (container) 
    {
		[self browseServerOrDir:obj.title() withDirID:container->objectId()];
	}
    else
    {
        [self setPlaylist];
        [self openMediaObj:obj.asItem()];
    }
}

- (void) browseServerOrDir:(const NPT_String&)dirTitle withDirID:(const NPT_String&) containerId
{
    DLNADelegation::GetInstance()->browse(dirTitle,containerId);
}

- (void) openMediaObj:(const deejay::DLNAItem*) mediaItem
{
    DLNADelegation::GetInstance()->openMediaObj(mediaItem);
}

- (void) autoPlaybackMediaObj:(const deejay::DLNAItem*) mediaItem timeOut:(int)seconds
{
    DLNADelegation::GetInstance()->openMediaObj(mediaItem,seconds);
}

- (void) refreshAllDevices
{
    [self refreshDevices:deejay::DLNACore::FlushMode_All];
}
- (void) refreshRenderOnly
{
    [self refreshDevices:deejay::DLNACore::FlushMode_MediaRendererOnly];
}
- (void) refreshDevices:(deejay::DLNACore::FlushMode) flushMode
{
    DLNADelegation::GetInstance()->refreshDevices(flushMode);
}

- (void) refreshCurrentDirectory:(DLNAServerList*)aController
{
    UINavigationController * serverNavi = [self getDLNAPage:DLNAPageServer];
    NSInteger count = [serverNavi.viewControllers count];
    DLNAServerList *previousDir = (DLNAServerList*)[serverNavi.viewControllers objectAtIndex:count-2];
    if ([previousDir isServerList])
    {
        DLNADelegation::GetInstance()->refreshDirectory("0",aController);
    }
    else
    {
        deejay::DLNAObjectList* objectList = [previousDir getMediaObjList];
        NSInteger index = [previousDir selectedRow];
        const NPT_String & currentDirID = (objectList->itemAt(index))->objectId();
        DLNADelegation::GetInstance()->refreshDirectory(currentDirID,aController);
    }
    
}

- (void) cleanProgressiveBrowseOp:(DLNAServerList*)aController
{
    DLNADelegation::GetInstance()->abortProgressiveBrowser(aController);
}
//////////////////
- (void) selectRender:(deejay::DeviceDesc&)device
{
    DLNADelegation::GetInstance()->setActiveRender(device);
}

- (void) renderPageFirstSelectRender:(deejay::DeviceDesc&)device withMediaObj:(const deejay::DLNAItem*)mediaItem
{
    DLNADelegation::GetInstance()->setActiveRender(device);
    DLNADelegation::GetInstance()->openMediaObj(mediaItem);
    
}

/////////////////
- (void) getTrackInfo
{
    DLNADelegation::GetInstance()->queryTrackInfo();
}

- (void) stop
{
    DLNADelegation::GetInstance()->stop();
}

- (void) pause
{
    DLNADelegation::GetInstance()->pause();
}

- (void) play
{
    DLNADelegation::GetInstance()->play();
}


- (void) setVolume:(int)volume
{
    DLNADelegation::GetInstance()->setVolume(volume);
}

- (void) setProgress:(int)progress
{
    DLNADelegation::GetInstance()->setProgress(progress);
}

- (void) setMute:(BOOL)mute
{
    DLNADelegation::GetInstance()->setMute(mute);
}


////
- (void) renderReportPlayBackState:(DLNAMediaPlayBackState) state
{
    DLNADelegation::GetInstance()->renderReportPlayBackState(state);
}
- (void) renderReportProgress:(long long)playbackSeconds withDuration:(long long) durationSeconds
{
    DLNADelegation::GetInstance()->renderReportProgress(playbackSeconds,durationSeconds);
}
@end



#pragma mark Rotation

@implementation UITabBarController(Rotation) 

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    return YES;
}

@end













