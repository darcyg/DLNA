//
//  DLNAImageControlPage.h
//  Genie
//
//  Created by cs Siteview on 11-10-11.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
@class  DLNADeviceController;

@interface DLNAImageControlPage : UIView<UITableViewDataSource,UITableViewDelegate> {
    DLNADeviceController                    * delegate;
    
    UIImageView                             * bg_view;
    UIImageView                             * controlBtnBg;
    UILabel                                 * mediaObjName;
    //2011.11.01
    BOOL                                    isAutoPlaybackOpened;
    //2011.11.02
    UIImageView                             *slideShowModeBg;//2011.11.14
    NSInteger                               slideSpeed;
    
    //2012.2.9
    UIImageView                             * thumbImgView;
    UIInterfaceOrientation                  m_orientation;
    
}
@property (nonatomic, assign) DLNADeviceController* delegate;

- (void) transformTo:(UIInterfaceOrientation)orientation;
- (void) layoutPageViewWithInterfaceOrientation:(UIInterfaceOrientation) orientation;
- (void) layoutControlBtnsWithOrientation:(UIInterfaceOrientation) orientation;
- (void) setMediaObjInfo:(NSArray*) info;
- (void) btnPressForOpenSlideShow;
- (void) btnPressForCloseSlideShow;

//2012.2.9
- (void) setThumbImg:(NSData*) imgData;
- (void) scaleThumbImgViewIntoSquareWithLength:(float)length;
- (void) showThumbImg;
@end
