//
//  DLNACenter.h
//  Genie
//
//  Created by cs Siteview on 11-9-14.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DLNACore.h"
#import "DLNAGolbal.h"

@class DLNAServerList;
@interface DLNACenter : UITabBarController<UITabBarControllerDelegate,UIAlertViewDelegate> {
        NSString                    * m_serverTitle;
}
@property (nonatomic,retain) NSString *serverTitle;//record now selected server title
//share tool api
- (DLNAMediaObjType) getMediaObjType:(const deejay::DLNAObject*) media;
- (BOOL) currentRenderIsSelf;
//fundation
- (void) layoutDLNACenterOnController:(UIViewController*)viewController;
- (void) dismissDLNACenter;
- (void) startDLNAService;
- (void) stopDLNAService;

//option
- (void) refreshMediaSource;
- (void) restartDLNAService;
- (void) openDLNAServer;
- (void) closeDLNAServer;
- (void) openDLNARender;
- (void) closeDLNARender;
- (void) writeOptionConfigWithServerOn:(BOOL)serverOn renderOn:(BOOL)renderOn;
- (NSMutableArray*) readOptionConfig;


//UI

- (void) createBackBtn:(UINavigationItem*) navItem;
- (void) createBacktoBrowsePageBtn:(UINavigationItem*) navItem;
- (void) gotoBrowsePage;
- (void) selectDLNAPage:(DLNAPage)page;
- (UINavigationController*) getDLNAPage:(DLNAPage)page;
- (void) reloadMeidaRenderList:(const deejay::DeviceDescList&)data withCurrentRenderID:(const NPT_String&)renderId;
- (void) reloadMediaServerList:(const deejay::DeviceDescList&)mediaServerList;
- (void) addMediaObjectDir:(const NPT_String&)dirTitle withList:(const deejay::DLNAObjectList&)mediaObjectList;
- (void) addMediaObjectDir:(DLNAServerList*)controller;//2012.2.6  progress browse
- (void) reloadCurrentMediaObjectDir:(const deejay::DLNAObjectList&)mediaObjectList;
- (void) serverPagePopToRoot;

- (void) setPlaylist;
- (void) shouldSelectRenderForMedia:(const deejay::DLNAItem*)item;

- (deejay::DeviceDescList) snapshotMediaRendererList;
- (deejay::DeviceDescList) snapshotMediaServerList;


//thumb img 2012.2.9
- (void) setThumbImg:(NSData*)imgData withType:(DLNAMediaObjType)type;

//.............................device
//1. control point
- (void) showControlPointPageWithMediaType:(DLNAMediaObjType)type;
- (void) updateControlUI;
- (void) refreshTrack:(NSInteger)milliseconds;
- (void) autoPlaybackTimeOut;

//2.render
- (void) loadMediaWithType:(DLNAMediaObjType)type andURL:(NSString*)url;
- (void) onRenderPlay;
- (void) onRenderPause;
- (void) onRenderStop;
- (void) onRenderSetMute:(BOOL)mute;
- (void) onRenderSeekTo:(long long) timeInMillis;
- (void) onRenderSetVolume:(int) volume;


//UPNP
//.........
- (void) openMediaServer:(deejay::DeviceDesc&)device;
- (void) openDirectoryOrMeidaObj:(const deejay::DLNAObject&)obj;
- (void) browseServerOrDir:(const NPT_String&)dirTitle withDirID:(const NPT_String&) containerId;
- (void) openMediaObj:(const deejay::DLNAItem*) mediaItem;
- (void) autoPlaybackMediaObj:(const deejay::DLNAItem*) mediaItem timeOut:(int)seconds;
- (void) refreshAllDevices;
- (void) refreshRenderOnly;
- (void) refreshDevices:(deejay::DLNACore::FlushMode) flushMode;
- (void) refreshCurrentDirectory:(DLNAServerList*)aController;


- (void) cleanProgressiveBrowseOp:(DLNAServerList*)aController;//2012.2.7
//.........
- (void) selectRender:(deejay::DeviceDesc&)device;
- (void) renderPageFirstSelectRender:(deejay::DeviceDesc&)device withMediaObj:(const deejay::DLNAItem*)mediaItem;


//........control point
- (void) getTrackInfo;
- (void) stop;
- (void) pause;
- (void) play;
- (void) setVolume:(int)volume;
- (void) setProgress:(int)progress;
- (void) setMute:(BOOL)mute;

//.........render
- (void) renderReportPlayBackState:(DLNAMediaPlayBackState) state;
- (void) renderReportProgress:(long long)playbackSeconds withDuration:(long long) durationSeconds;

@end


@interface UITabBarController(Rotation) 

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation;

@end