//
//  DLNADeviceController.h
//  Genie
//
//  Created by cs Siteview on 11-9-16.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DLNACore.h"
#import "DLNAGolbal.h"
@class DLNACenter;
@class MPMoviePlayerController;
@class DLNAVideoControlPage;
@class DLNAImageControlPage;
@class MyScrollView;
@interface DLNADeviceController : UIViewController<UIAlertViewDelegate> {
    DLNACenter                                  * delegate;
    
    //control page
    UIView                                      * controlPointView;
    DLNAVideoControlPage                        * videoControlPage;
    DLNAImageControlPage                        * imgControlPage;
    //
    NSMutableData                               * m_imgData;
    
    DLNAMediaObjType                            currentMediaObjType;
    deejay::DLNAObjectList                      playList;
    NSInteger                                   nowPlayIndexAtPlayList;
    //slide show
    NSTimer                                     * autoPlayBackPicTimer;
    BOOL                                        needAutoPlayBackPicture;
    NSInteger                                   slideSpeed;
    
    
    //render page
    UIView                                      * mediaPlaybackView;
    MPMoviePlayerController                     * moviePlayController;
    MyScrollView                                * imgView;
    NSTimer                                     * releaseTrackTimer;
    
    UIAlertView                                 * loadWaitingView;
    UIAlertView                                 * notSupportDig;
    
}
@property (nonatomic, assign) DLNACenter* delegate;
@property (nonatomic, assign) MPMoviePlayerController * moviePlayController;

- (BOOL) currentRenderIsSelf;
- (void) becomeCurrentPageView;
- (void) resignCurrentPageView;
- (void) showViewWithOrientation:(UIInterfaceOrientation) orientation;
- (void) transformViewTo:(UIInterfaceOrientation) orientation;
- (void) rotateRenderViewTo:(UIInterfaceOrientation) orientation;


//2012.2.9
- (void) setThumbImg:(NSData*) imgData withMediaObjType:(DLNAMediaObjType)type;

/////////////////////////////////////////////
//// as  control point
////////////////////////////////////////////
- (NSArray*) getCurrentMediaInfo;
- (void) loadControlPointViewWithMediaObjType:(DLNAMediaObjType)type;//load cp view
- (void) setPlayList:(const deejay::DLNAObjectList&) playlist withNowPlayIndex:(NSInteger) index;

- (void) receiveUPnPControlSignal:(DLNAControlSignal) signal;
- (void) receiveSetVolumeSignalWithVolume:(int) volume maxVolume:(int) maxVolume minVolume:(int) minVolume;
- (void) receiveSetMuteSignal:(BOOL) needMute;
- (void) receiveSetTotalDuration:(int) duration;
- (void) receiveTrackSignal:(int) time;


- (void) sendStopSignal;
- (void) sendPauseSignal;
- (void) sendPlaySignal;
- (void) sendMuteBtnPressSignal:(BOOL) mute;
- (void) sendChangeProgressSignal:(int) progress;
- (void) sendChangeVolumeSignal:(int) volume;
- (void) next;
- (void) previous;

//on or off slide picture are controlled by slide switch btn on imageControlPage
- (void) openAutoPlaybackPicWithSpeed:(NSInteger)speed;
- (void) closeAutoPlaybackPic;
- (void) startAutoPlayBackPicture;
- (void) finishAutoPlayBackPicture;
- (void) autoPlayBackPicture;
- (void) autoPlaybackTimeOut;

/////////////////////////////////////////////
//// as  render
////////////////////////////////////////////
- (void) loadMediaWithType:(DLNAMediaObjType)type andURL:(NSString*)url;
- (void) getImgData:(NSURLRequest*)req;
- (void) onRenderPlay;
- (void) onRenderPause;
- (void) onRenderStop;
- (void) onRenderSetMute:(BOOL)mute;
- (void) onRenderSeekTo:(long long) timeInMillis;
- (void) onRenderSetVolume:(int) volume;




@end
