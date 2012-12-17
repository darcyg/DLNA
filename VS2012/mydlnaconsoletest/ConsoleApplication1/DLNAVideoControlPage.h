//
//  DLNAVideoControlPage.h
//  Genie
//
//  Created by cs Siteview on 11-10-9.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DLNAGolbal.h"

enum{
    PLAY_BTN = 1003,
    PAUSE_BTN,
};
typedef  int  PausePlayBtn_type;
@class  DLNADeviceController;

@interface DLNAVideoControlPage : UIView {
    DLNADeviceController            * delegate;
    
    UIImageView                     * bg_view;
    UIImageView                     * controlBtnBg;
    
    UISlider                        * progressView;
    UISlider                        * volumeView;
    
    UILabel                         * leftProgress;
    UILabel                         * rightProgress;
    UILabel                         * mediaObjName;
    
    PausePlayBtn_type               pausePlayBtnType;
    NSTimer                         * getTrackTimer;
    
    //2012.2.9
    UIImageView                             * thumbImgView;
    UIInterfaceOrientation                  m_orientation;
    
}
@property (nonatomic, assign) DLNADeviceController *delegate;

- (void) transformTo:(UIInterfaceOrientation)orientation withType:(DLNAMediaObjType) type;
- (void) layoutPageViewWithInterfaceOrientation:(UIInterfaceOrientation) orientation;
- (void) layoutControlBtnsWithOrientation:(UIInterfaceOrientation) orientation;
- (void) setBg_imgWithType:(DLNAMediaObjType) type;
- (void) setMediaObjInfo:(NSArray*) info;

- (void) recevieUPnPControlSignal:(DLNAControlSignal) signal;
- (void) receiveSetVolumeSignalWithVolume:(int) volume maxVolume:(int) maxVolume minVolume:(int) minVolume;
- (void) receiveSetMuteSignal:(BOOL) needMute;
- (void) receiveSetTotalDuration:(int) duration;
- (void) receiveTrackSignal:(int) time;

- (void) showTrackTime:(int) timeStr inLabel:(UILabel*) label;
- (NSString*) formatterTime:(int) millisecond;

//2012.2.9
- (void) setThumbImg:(NSData*) imgData;
- (void) scaleThumbImgViewIntoSquareWithLength:(float)length;
- (void) showThumbImg;
@end
