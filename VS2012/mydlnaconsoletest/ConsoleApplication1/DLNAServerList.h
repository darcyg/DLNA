//
//  DLNAServerList.h
//  DLNAdemo
//
//  Created by cs Siteview on 11-9-13.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DLNACore.h"
#import "DLNAGolbal.h"

@class DLNACenter;
class DMSImageItem;
@interface DLNAServerList : UIViewController<UITableViewDelegate,UITableViewDataSource,UIAlertViewDelegate>{
    DLNACenter                  * delegate;
    deejay::DeviceDescList      * m_dmsList;
	deejay::DLNAObjectList      * m_objList;
    BOOL                        m_rootViewMode;
    
    NPT_List<DMSImageItem*>     m_objImageList;
	deejay::URLLoader           m_loader;
	deejay::ReadWriteLock       m_queueLock;
	NPT_List<int>               m_updateQueue;
    
    NSInteger                   m_selectedRow;
    //
    UITextView                  * m_nodeviceview;
    
    NSString                    * m_ServerString;
    
    //2012.2.13
    UITableView                 * m_tableView;
    UIView                      * m_headView;
}
@property (nonatomic, retain) UITableView * tableView;//2012.2.13
@property (nonatomic, retain) UIView * headerView;//2012.2.13
@property (nonatomic, assign) DLNACenter* delegate;
@property (nonatomic,readonly) NSInteger selectedRow;
@property (nonatomic, retain) NSString *headTitle;
- (void) becomeCurrentPageView;

- (int) getMediaSize:(const deejay::DLNAObject*) media;
- (deejay::DLNAObjectList*) getMediaObjList;
- (BOOL) isServerList;

- (void)reloadRootData:(const deejay::DeviceDescList&)ls;
- (void)reloadLevelData:(const deejay::DLNAObjectList&)ls;

- (void)reloadLevelData:(const deejay::DLNAObjectList&)ls withTotalObjNumber:(int) totalNumber;//2012.2.8
- (void)addLevelData:(const deejay::DLNAObjectList&)ls withTotalObjNumber:(int) totalNumber;//2012.2.7

//当前controller销毁时，若progressive Browser Op还没有finish，则应该主动abort
- (void) cleanProgressiveBrowseOp;

- (void) requestUpdateItemFromWorkerThread:(int)index;
- (void) updateItemIconsOnMainThread;
- (void) elevateImagesForOnscreenRows;

- (void) settableHerderViewWithOrientation:(UIInterfaceOrientation)orientation;//2012.2.8
- (void) setDirNumberInfo:(NSString*)info;//以类似12/100的格式显示出当前目录中对象信息： 已经刷出来的对象个数/总共的对象个数

- (void) layoutViewWithOrientation:(UIInterfaceOrientation) orientation;
@end



//////////////////////
class DMSImageItem : public deejay::URLLoaderTask::FinishCallback
{
public:
	DMSImageItem(DLNAServerList *owner, int index);
	~DMSImageItem();
	virtual void onURLLoaderTaskFinished(deejay::URLLoaderTask *task);
	void manage(deejay::URLLoaderTask *task);
	
	UIImage *m_image;
	deejay::URLLoaderTask *m_task;
	CGImageRef m_cg;
	deejay::ReadWriteLock m_stateLock;
	bool m_finished;
	DLNAServerList *m_owner;
	int m_index;
	bool m_loading;
};