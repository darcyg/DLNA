//
//  DLNARenderList.h
//  DLNAdemo
//
//  Created by cs Siteview on 11-9-13.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DLNACore.h"
@class DLNACenter;
@interface DLNARenderList : UITableViewController<UIActionSheetDelegate,UIAlertViewDelegate> {
    DLNACenter                                  * delegate;
    deejay::DeviceDescList                      * m_list;
    NPT_String                                  m_currentRenderID;
    
    //
    NPT_List<const deejay::DLNAItem*>           m_mediaBuf;
    
    //
    UITextView                                  * m_nodeviceview;
    UITableView                                 * m_tableView;
}
@property (nonatomic, assign) DLNACenter* delegate;
@property (nonatomic, retain) UITableView * tableView;
- (void) becomeCurrentPageView;

- (void) reloadData:(const deejay::DeviceDescList&)data withCurrentRenderID:(const NPT_String&)renderId;
- (void) shouldSelectRenderForMedia:(const deejay::DLNAItem*)item;
- (const deejay::DLNAItem*) getMediaBuf;

@end
