 //
//  DLNAServerList.m
//  DLNAdemo
//
//  Created by cs Siteview on 11-9-13.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "DLNAServerList.h"
#import "DLNACenter.h"

#define SELECTED_ROW_NONE     -1
#define SERVER_TITLE_LABLE_TAG   101
#define DIR_NUMBER_INFO          102
#define TABEL_HEADER_VIEW_HEIGHT 30

#define REC_IS_PORTRAIT  CGRectMake(0.0f, 0.0f, 768.0f, 1024.0f-TABEL_HEADER_VIEW_HEIGHT-44*2)
#define REC_IS_LANDSCAPE CGRectMake(0.0f, 0.0f, 1024.0f, 768.0f-TABEL_HEADER_VIEW_HEIGHT-44*2)
@implementation DLNAServerList
@synthesize delegate;
@synthesize selectedRow = m_selectedRow;
@synthesize headTitle = m_ServerString;
@synthesize tableView = m_tableView;
@synthesize headerView = m_headView;

- (id) init
{
    self = [super init];
    if (self) {
        // Custom initialization
        m_dmsList = new deejay::DeviceDescList();
        m_objList = new deejay::DLNAObjectList();
        m_selectedRow = SELECTED_ROW_NONE;
        
        //
        self.headerView = nil;
        m_nodeviceview = nil;
        m_rootViewMode = NO;
    }
    return self;
}

- (void)dealloc
{
    if (!m_rootViewMode)
    {
        [self cleanProgressiveBrowseOp];
    }
    delete m_objList;
    delete m_dmsList;
    [m_nodeviceview release];
    self.headTitle = nil;
    self.tableView = nil;
    self.headerView = nil;
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
    m_loader.shutdown();
	m_loader.waitForDone();
    for (NPT_Ordinal i = 0; i < m_objImageList.GetItemCount(); i++) {
        DMSImageItem *imageItem = *m_objImageList.GetItem(i);
        [imageItem->m_image release];
        imageItem->m_image = nil;
        imageItem->m_loading = false;
        imageItem->m_finished = false;
    }
    m_loader.start();
    [self.tableView reloadData];
}

#pragma mark - View lifecycle
- (void) loadView
{
    [super loadView];
    UIView * bg = [[UIView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.view = bg;
    [bg release];
    UITableView * tView = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStylePlain];
    tView.dataSource = self;
    tView.delegate = self;
    self.tableView = tView;
    [tView release];
    [self.view addSubview:self.tableView];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    UIBarButtonItem * barItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                                                                              target:self
                                                                              action:@selector(refresh)];
    self.navigationItem.rightBarButtonItem = barItem;
    [barItem release];
    if ([[[self.navigationController viewControllers] objectAtIndex:0] isEqual:self])
    {
        m_rootViewMode = YES;
        *m_dmsList = [delegate snapshotMediaServerList];
        
        //
        m_nodeviceview = [[UITextView alloc] init];
//        m_nodeviceview.text = @"No shared device was found. To use this feature, you need to enable the file sharing feature for devices in the network, or connect a USB storage drive to your router if the router supports USB storage connection. ";
         m_nodeviceview.text=NSLocalizedStringFromTable(@"No device find", @"Localizable", nil);
        
        m_nodeviceview.textColor = [UIColor redColor];
        m_nodeviceview.font = [UIFont boldSystemFontOfSize:20];
        m_nodeviceview.editable = NO;
        //m_nodeviceview.numberOfLines = 0;
        m_nodeviceview.hidden = YES;
        [self.view addSubview:m_nodeviceview];
    }
    else
    {
        UIView * headerView = [[UIView alloc] init];
        headerView.backgroundColor = [UIColor colorWithRed:229.0/255 green:229.0/255 blue:251.0/255 alpha:1.0];
        headerView.frame = CGRectMake(0, 0, 0, TABEL_HEADER_VIEW_HEIGHT);
        
        UILabel * lab1 = [[UILabel alloc] init];
        lab1.tag = SERVER_TITLE_LABLE_TAG;
        lab1.textAlignment = UITextAlignmentLeft;
        lab1.text = [NSString stringWithFormat:@"   %@",self.headTitle];
        lab1.font = [UIFont boldSystemFontOfSize:14];
        lab1.backgroundColor = [UIColor clearColor];
        lab1.numberOfLines = 1;
        [headerView addSubview:lab1];
        [lab1 release];
        
        UILabel * lab2 = [[UILabel alloc] init];
        lab2.tag = DIR_NUMBER_INFO;
        lab2.textAlignment = UITextAlignmentRight;
        lab2.backgroundColor = [UIColor clearColor];
        lab2.font = [UIFont systemFontOfSize:14];
        [headerView addSubview:lab2];
        [lab2 release];
        
        self.headerView = headerView;
        [self.view addSubview:self.headerView];
        [headerView release];
    }
}


- (void)settableHerderViewWithOrientation:(UIInterfaceOrientation)orientation
{
    CGSize size;
    if (UIInterfaceOrientationIsPortrait(orientation))
    {
        size = CGSizeMake(768, TABEL_HEADER_VIEW_HEIGHT);
    }
    if (UIInterfaceOrientationIsLandscape(orientation))
    {
        size = CGSizeMake(1024, TABEL_HEADER_VIEW_HEIGHT);
    }
    UILabel * lab1 = (UILabel*)[self.headerView viewWithTag:SERVER_TITLE_LABLE_TAG];
    UILabel * lab2 = (UILabel*)[self.headerView viewWithTag:DIR_NUMBER_INFO];
    lab1.frame = CGRectMake(0, 0, size.width*3/4, size.height);
    lab2.frame = CGRectMake(0, 0, size.width/4, size.height);
    lab1.center = CGPointMake(size.width*3/8, size.height/2);
    lab2.center = CGPointMake(size.width*7/8, size.height/2);
}

- (void) setDirNumberInfo:(NSString*)info//以12/100的格式显示出当前目录中对象信息： 已经刷出来的对象个数/总共的对象个数
{
    /*UILabel * lab = (UILabel*)[self.tableView.tableHeaderView viewWithTag:DIR_NUMBER_INFO];
    lab.text = [NSString stringWithFormat:@"%@   ",info];*/
}

- (void) layoutViewWithOrientation:(UIInterfaceOrientation) orientation
{
    CGRect rec;
    if (UIInterfaceOrientationIsPortrait(orientation))
    {
        rec = REC_IS_PORTRAIT;
        if (m_rootViewMode)
        {
            self.tableView.frame = rec;
            m_nodeviceview.frame = rec;
        }
        else
        {
            self.headerView.frame = CGRectMake(0.0f, 0.0f, rec.size.width, TABEL_HEADER_VIEW_HEIGHT);
            self.tableView.frame = CGRectMake(0.0f, TABEL_HEADER_VIEW_HEIGHT, rec.size.width, rec.size.height-TABEL_HEADER_VIEW_HEIGHT);
            [self settableHerderViewWithOrientation:orientation];
        }
    }
    if (UIInterfaceOrientationIsLandscape(orientation))
    {
        rec = REC_IS_LANDSCAPE;
        if (m_rootViewMode)
        {
            self.tableView.frame = rec;
            m_nodeviceview.frame = rec;
        }
        else
        {
            self.headerView.frame = CGRectMake(0.0f, 0.0f, rec.size.width, TABEL_HEADER_VIEW_HEIGHT);
            self.tableView.frame = CGRectMake(0.0f, TABEL_HEADER_VIEW_HEIGHT, rec.size.width, rec.size.height-TABEL_HEADER_VIEW_HEIGHT);
            [self settableHerderViewWithOrientation:orientation];
        }
    }

}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    m_loader.start();
    [self layoutViewWithOrientation:self.interfaceOrientation];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [self.tableView reloadData];
}

- (void)viewWillDisappear:(BOOL)animated
{
    m_loader.shutdown();
	m_loader.waitForDone();
    for (NPT_Ordinal i = 0; i < m_objImageList.GetItemCount(); i++) {
        DMSImageItem *imageItem = *m_objImageList.GetItem(i);
        if (imageItem->m_loading) {
            imageItem->m_loading = false;
        }
    }
    [super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return YES;
}
- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    [self layoutViewWithOrientation:toInterfaceOrientation];
}

#pragma mark - Table view data source

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    int ret = 0;
    if (m_rootViewMode)
    {
        ret = m_dmsList->count();
        if (ret == 0)
        {
            [NSTimer scheduledTimerWithTimeInterval:2.5 target:self selector:@selector(noDevice) userInfo:nil repeats:NO];
        }
        else
        {
            m_nodeviceview.hidden = YES;
            self.tableView.hidden = NO;
        }
    }
    else
    {
        ret = m_objList->count();
    }
    return ret;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:CellIdentifier] autorelease];
    }
    if (m_rootViewMode) 
    {
        cell.imageView.image = [UIImage imageNamed:@"defaultserver.png"];
        
		deejay::DeviceDesc *deviceDesc = m_dmsList->itemAt(indexPath.row);
		cell.textLabel.text = [NSString stringWithUTF8String:deviceDesc->friendlyName()];
        cell.textLabel.font = [UIFont systemFontOfSize:18];
        cell.detailTextLabel.text = @"";
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		const deejay::IconDesc *iconDesc = deviceDesc->iconAt(0);
		if (iconDesc) 
        {
			NSData *imageData = [NSData dataWithBytes:iconDesc->iconData().GetData() length:iconDesc->iconData().GetDataSize()];
			cell.imageView.image = [UIImage imageWithData:imageData];
		}
	}
    else
    {
        deejay::DLNAObject *obj = m_objList->itemAt(indexPath.row);
        cell.textLabel.text = [NSString stringWithUTF8String:obj->title()];
        cell.textLabel.font = [UIFont systemFontOfSize:18];
        //img
        BOOL isContainer = (NULL != obj->asContainer()) ? YES : NO;
        if (!isContainer)
        {
            long long mediaSize = [self getMediaSize:obj];
            if (mediaSize > 0)
            {
                UILabel * lab = [[UILabel alloc] init];
                lab.textAlignment = UITextAlignmentRight;
                lab.frame = CGRectMake(0, 0, 80, 38);
                lab.textColor = cell.detailTextLabel.textColor;
                double MB = 0.0f;
                double KB = 0.0f;
                if ((MB = mediaSize/(1024*1024*1.0f)) >= 1.0f)
                {
                    lab.text = [NSString stringWithFormat:@"%.2f MB",MB];
                }
                else if ((KB = mediaSize/1024*1.0f) >= 1.0f)
                {
                    lab.text = [NSString stringWithFormat:@"%.2f KB",KB];
                }
                else
                {
                    lab.text = [NSString stringWithFormat:@"%d Bytes",mediaSize];
                }
                lab.font = [UIFont systemFontOfSize:12];
                cell.accessoryView = lab;
                [lab release];
                lab = nil;
            }
            else
            {
                cell.accessoryView = nil;
            }
        }
        
        DMSImageItem *imageItem = *m_objImageList.GetItem(indexPath.row);
		if (imageItem->m_image != nil) 
        {
			cell.imageView.image = imageItem->m_image;
		}
        else
        {
            if (isContainer)
            {
                cell.imageView.image = [UIImage imageNamed:@"folder.png"];
            }
            else
            {
                DLNAMediaObjType type = [delegate getMediaObjType:obj];
                switch (type)
                {
                    case DLNAMediaObjTypeVideo:
                        cell.imageView.image = [UIImage imageNamed:@"videoObj.png"];
                        break;
                        
                    case DLNAMediaObjTypeAudio:
                        cell.imageView.image = [UIImage imageNamed:@"audioObj.png"];
                        break;
                        
                    case DLNAMediaObjTypeImage:
                        cell.imageView.image = [UIImage imageNamed:@"imageObj.png"];
                        break;
                        
                    default:
                        cell.imageView.image = [UIImage imageNamed:@"videoObj.png"];
                        break;
                }
            }
            
            if (!imageItem->m_loading) 
            {
				imageItem->m_loading = true;
				NPT_String iconUrl;
				if (obj->findThumbnailURL(38, 38, NULL, iconUrl)) 
                {
					deejay::URLLoaderTask *task;
					if (NPT_SUCCEEDED(m_loader.enqueue(iconUrl, imageItem, &task))) 
                    {
						imageItem->manage(task);
					}
                }
            }
        }
        cell.accessoryType = isContainer ? UITableViewCellAccessoryDisclosureIndicator : UITableViewCellAccessoryNone;
    }
    
    return cell;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    m_selectedRow = indexPath.row;
    if (m_rootViewMode)
    {
        delegate.serverTitle = [tableView cellForRowAtIndexPath:indexPath].textLabel.text;
        [delegate openMediaServer:*(m_dmsList->itemAt(indexPath.row))];
    }
    else
    {
        [delegate openDirectoryOrMeidaObj:*(m_objList->itemAt(indexPath.row))];
    }
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate
{
    if (!decelerate)
	{
        [self elevateImagesForOnscreenRows];
    }
}

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView
{
    [self elevateImagesForOnscreenRows];
}

#pragma mark dmsImg func
- (void) requestUpdateItemFromWorkerThread:(int)index
{
	deejay::WriteLocker locker(m_queueLock);
	NPT_List<int>::Iterator it = m_updateQueue.Find(NPT_ObjectComparator<int>(index));
	if (!it) 
    {
		m_updateQueue.Add(index);
	}
	if (m_updateQueue.GetItemCount() == 1) {
		[self performSelectorOnMainThread:@selector(updateItemIconsOnMainThread) withObject:nil waitUntilDone:NO];
	}
}

- (void) updateItemIconsOnMainThread
{
	NSMutableArray *dirtyRows = [[[NSMutableArray alloc] init] autorelease];
	deejay::WriteLocker locker(m_queueLock);
	for (NPT_Ordinal i = 0; i < m_updateQueue.GetItemCount(); i++) 
    {
		int index = *m_updateQueue.GetItem(i);
		NPT_List<DMSImageItem*>::Iterator it = m_objImageList.GetItem(index);
		if (it) 
        {
			DMSImageItem *imageItem = *it;
			imageItem->m_loading = false;
			CGImage *cg = NULL;
			{
				deejay::WriteLocker locker2(imageItem->m_stateLock);
				cg = imageItem->m_cg;
				imageItem->m_cg = NULL;
			}
			if (cg) 
            {
				imageItem->m_image = [[UIImage imageWithCGImage:cg] retain];
				CFRelease(cg);
				[dirtyRows addObject:[NSIndexPath indexPathForRow:imageItem->m_index inSection:0]];
			}
		}
	}
	m_updateQueue.Clear();
	if (dirtyRows.count > 0) 
    {
		[self.tableView reloadRowsAtIndexPaths:dirtyRows withRowAnimation:UITableViewRowAnimationNone];
	}
}


- (void) elevateImagesForOnscreenRows
{
	NSArray *visiblePaths = [self.tableView indexPathsForVisibleRows];
	for (NSIndexPath *indexPath in visiblePaths)
    {
		NPT_List<DMSImageItem*>::Iterator it = m_objImageList.GetItem(indexPath.row);
		if (it) 
        {
			DMSImageItem *imageItem = *it;
			deejay::URLLoaderTask *task = NULL;
			{
				deejay::ReadLocker locker(imageItem->m_stateLock);
				if (imageItem->m_task) 
                {
					imageItem->m_task->addRef();
					task = imageItem->m_task;
				}
			}
			if (task) 
            {
				task->elevate();
				task->release();
			}
		}
	}
}
#pragma  mark custom func
- (void) becomeCurrentPageView
{
}
- (void) noDevice
{
    if(0 == [self.tableView numberOfRowsInSection:0])
    {
        self.tableView.hidden = YES;
        m_nodeviceview.hidden = NO;
    }
}

- (int) getMediaSize:(const deejay::DLNAObject*) media
{
    return media->resourceSize();
}
- (deejay::DLNAObjectList*) getMediaObjList
{
    return m_objList;
}

- (BOOL) isServerList
{
    return m_rootViewMode;
}


- (void) refresh
{
    if ([self isEqual:[self.navigationController.viewControllers objectAtIndex:0]])
    {
        if (!m_nodeviceview.hidden)
        {
            [delegate restartDLNAService];
        }
        [delegate refreshAllDevices];
    }
    else
    {
        m_loader.shutdown();
        m_loader.waitForDone();
        m_loader.start();
        
        /*
        //2012.2.8
        m_objList->clear();
        m_objImageList.Clear();
        [self setDirNumberInfo:@"0/0"];
        [self.tableView reloadData];
        [self cleanProgressiveBrowseOp];*/
        
        [delegate refreshCurrentDirectory:self];
    }
}


- (void)reloadRootData:(const deejay::DeviceDescList&)data
{
    m_rootViewMode = YES;
    *m_dmsList = data;
    m_objList->clear();
    m_objImageList.Apply(NPT_ObjectDeleter<DMSImageItem>());
	m_objImageList.Clear();
    [self.tableView reloadData];
}

- (void)reloadLevelData:(const deejay::DLNAObjectList&)data
{
    m_rootViewMode = NO;
    *m_objList = data;
    m_dmsList->clear();
    m_objImageList.Apply(NPT_ObjectDeleter<DMSImageItem>());
	m_objImageList.Clear();
	for (NPT_Ordinal i = 0; i < m_objList->count(); i++) 
    {
		m_objImageList.Add(new DMSImageItem(self, i));
	}
    [self.tableView reloadData];
}

- (void)reloadLevelData:(const deejay::DLNAObjectList&)data withTotalObjNumber:(int) totalNumber
{
    m_rootViewMode = NO;
    *m_objList = data;
    m_dmsList->clear();
    m_objImageList.Apply(NPT_ObjectDeleter<DMSImageItem>());
	m_objImageList.Clear();
	for (NPT_Ordinal i = 0; i < m_objList->count(); i++) 
    {
		m_objImageList.Add(new DMSImageItem(self, i));
	}
    [self setDirNumberInfo:[NSString stringWithFormat:@"%d/%d",m_objList->count(),totalNumber]];
    [self.tableView reloadData];
}

- (void)addLevelData:(const deejay::DLNAObjectList&)ls withTotalObjNumber:(int) totalNumber//2012.2.
{
	for (NPT_Ordinal i = 0; i < ls.count(); i++) 
    {
		m_objImageList.Add(new DMSImageItem(self,m_objList->count() + i));
	}
    m_objList->add(ls);
    [self setDirNumberInfo:[NSString stringWithFormat:@"%d/%d",m_objList->count(),totalNumber]];
    [self.tableView reloadData];
}


- (void) cleanProgressiveBrowseOp
{
    //[delegate cleanProgressiveBrowseOp:self];
}
@end


/////////////////
DMSImageItem::DMSImageItem(DLNAServerList *owner, int index)
: m_owner(owner), m_index(index)
{
	m_image = nil;
	m_task = NULL;
	m_cg = NULL;
	m_finished = false;
	m_loading = false;
}

DMSImageItem::~DMSImageItem()
{
	if (m_image != nil) {
		[m_image release];
		m_image = nil;
	}
	
	deejay::WriteLocker locker(m_stateLock);
	if (m_task)
    {
		m_task->resetCallback();
		m_task->release();
		m_task = NULL;
	}
	
	if (m_cg != NULL) 
    {
		CFRelease(m_cg);
		m_cg = NULL;
	}
}

void DMSImageItem::onURLLoaderTaskFinished(deejay::URLLoaderTask *task)
{
	if (task->succeeded()) 
    {
		NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        
		NSData *imageData = [[NSData alloc] initWithBytes:task->data().GetData() length:task->data().GetDataSize()];
		UIImage *image = [[UIImage alloc] initWithData:imageData];
		if (image != nil) 
        {
            CGSize imgSize = image.size;
			CGSize itemSize = CGSizeMake(40.0f, 40.0f);
            CGFloat scale = 40.0f/(imgSize.width>imgSize.height?imgSize.width:imgSize.height);
			UIGraphicsBeginImageContext(itemSize);
			CGRect imageRect = CGRectMake(0.0, 0.0, imgSize.width*scale, imgSize.height*scale);
            if (imgSize.width>imgSize.height)
            {
                imageRect.origin.y = (40.0f - imageRect.size.height)/2;
            }
            else if (imgSize.width<imgSize.height)
            {
                imageRect.origin.x = (40.0f - imageRect.size.width)/2;
            }
			[image drawInRect:imageRect];
			UIImage *thumbImage = UIGraphicsGetImageFromCurrentImageContext();
			CGImageRef cg = CGImageCreateCopy([thumbImage CGImage]);
			UIGraphicsEndImageContext();			
			[image release];
			{
				deejay::WriteLocker locker(m_stateLock);
				if (m_cg == NULL) 
                {
					m_cg = cg;
					[m_owner requestUpdateItemFromWorkerThread:m_index];
				} else 
                {
					CFRelease(cg);
				}
			}
		}
		[imageData release];
		[pool release];
	}
	
	{
		deejay::WriteLocker locker(m_stateLock);
		m_finished = true;
		if (m_task) {
			m_task->release();
			m_task = NULL;
		}
	}
}

void DMSImageItem::manage(deejay::URLLoaderTask *task)
{
	deejay::WriteLocker locker(m_stateLock);
	if (m_finished) {
		task->release();
	} else {
		m_task = task;
	}
}