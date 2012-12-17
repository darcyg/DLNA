//
//  DLNAOptionController.h
//  GenieHD
//
//  Created by cs Siteview on 11-12-13.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
@class DLNACenter;

@interface DLNAOptionController : UITableViewController {
    DLNACenter                      * delegate;
    UISwitch *                      m_fileShareSwitch;
    UISwitch *                      m_allowRenderSwitch;
}
@property (nonatomic, assign) DLNACenter* delegate;
- (void) setValueForSwitch;
- (void) becomeCurrentPageView;
@end
