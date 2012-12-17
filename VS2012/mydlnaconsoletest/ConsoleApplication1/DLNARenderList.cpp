//
//  DLNARenderList.m
//  DLNAdemo
//
//  Created by cs Siteview on 11-9-13.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "DLNARenderList.h"
#import "DLNACenter.h"

#define TAG_DIALOG_SELECTED_RENDER 403

#define REC_IS_PORTRAIT  CGRectMake(0.0f, 0.0f, 768.0f, 1024.0f-50-self.tabBarController.tabBar.frame.size.height)
#define REC_IS_LANDSCAPE CGRectMake(0.0f, 0.0f, 1024.0f, 768.0f-50-self.tabBarController.tabBar.frame.size.height)

@implementation DLNARenderList
@synthesize delegate;
@synthesize tableView = m_tableView;


//bool static isFirstSelectRender = NO;

- (id) init
{
    self = [super init];
    if (self) {
        // Custom initialization
        m_list = new deejay::DeviceDescList();
    }
    return self;
}

- (void)dealloc
{
    delete m_list;
    [m_nodeviceview release];
    self.tableView = nil;
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
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
    
    //
    m_nodeviceview = [[UITextView alloc] init];
//    m_nodeviceview.text = @"No Player is found. To play files on this device, go to Options and select \"Allow play in network\". ";
     m_nodeviceview.text= NSLocalizedStringFromTable(@"No player find", @"Localizable", nil);
    m_nodeviceview.textColor = [UIColor redColor];
    m_nodeviceview.font = [UIFont boldSystemFontOfSize:20];
    m_nodeviceview.editable = NO;
    //m_nodeviceview.numberOfLines = 0;
    m_nodeviceview.hidden = YES;
    [self.view addSubview:m_nodeviceview];
}
- (void)viewDidLoad
{
    [super viewDidLoad];
    UIBarButtonItem * barItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                                                                              target:self
                                                                              action:@selector(refresh)];
    self.navigationItem.rightBarButtonItem = barItem;
    [barItem release];
    *m_list = [delegate snapshotMediaRendererList];
}

- (void) layoutViewWithOrientation:(UIInterfaceOrientation) orientation
{
    CGRect rec;
    if (UIInterfaceOrientationIsPortrait(orientation))
    {
        rec = REC_IS_PORTRAIT;
        self.tableView.frame = rec;
        m_nodeviceview.frame = rec;
    }
    else if (UIInterfaceOrientationIsLandscape(orientation))
    {
        rec = REC_IS_LANDSCAPE;
        self.tableView.frame = rec;
        m_nodeviceview.frame = rec;
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
    [self layoutViewWithOrientation:self.interfaceOrientation];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration;
{
    [self layoutViewWithOrientation:toInterfaceOrientation];
}
#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    int number = m_list->count();
    if (number == 0)
    {
        [NSTimer scheduledTimerWithTimeInterval:2.5 target:self selector:@selector(noDevice) userInfo:nil repeats:NO];
    }
    else
    {
        m_nodeviceview.hidden = YES;
        self.tableView.hidden = NO;
    }
    return number;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
    cell.imageView.image = [UIImage imageNamed:@"defaultrender.png"];
    deejay::DeviceDesc * render = m_list->itemAt(indexPath.row);
    deejay::IconDesc * icon = render->iconAt(0);
    if (NULL != icon)
    {
        const NPT_DataBuffer& iconDataBuffer = icon->iconData();
        cell.imageView.image = [UIImage imageWithData:[NSData dataWithBytes:iconDataBuffer.GetData() length:iconDataBuffer.GetDataSize()]];
    }
    
    cell.textLabel.text = [NSString stringWithUTF8String:render->friendlyName().GetChars()];
    cell.textLabel.font = [UIFont systemFontOfSize:18];
    cell.accessoryType =  m_currentRenderID == render->uuid().toString() ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
    return cell;
}


#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [delegate selectRender:*m_list->itemAt(indexPath.row)];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    UIAlertView * alert = [[UIAlertView alloc] initWithTitle:nil
//                                                     message:@"Please select a source"
                                                    message:NSLocalizedStringFromTable(@"Select source", @"Localizable", nil)
                                                    delegate:self
                                           cancelButtonTitle:NSLocalizedStringFromTable(@"cancel",@"Localizable", nil)
                                           otherButtonTitles:NSLocalizedStringFromTable(@"ok",@"Localizable", nil), nil];
    alert.tag = TAG_DIALOG_SELECTED_RENDER;
    [alert show];
    [alert release];
}

#pragma mark alert delegation
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    switch (alertView.tag)
    {
        case TAG_DIALOG_SELECTED_RENDER:
        {
            if (buttonIndex == 1)
            {
                [delegate gotoBrowsePage];
            }
        }
            break;
            
        default:
            break;
    }
}

#pragma mark action sheet 

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex<m_list->count())
    {
        [actionSheet dismissWithClickedButtonIndex:0 animated:NO];
        [delegate renderPageFirstSelectRender:*m_list->itemAt(buttonIndex) withMediaObj:[self getMediaBuf]];
    }
}
#pragma mark custom func
- (void) becomeCurrentPageView
{
    m_nodeviceview.frame = self.view.frame;
}
- (void) noDevice
{
    if(0 == [self.tableView numberOfRowsInSection:0])
    {
        self.tableView.hidden = YES;
        m_nodeviceview.hidden = NO;
    }
}
- (void)reloadData:(const deejay::DeviceDescList&)data withCurrentRenderID:(const NPT_String&)renderId
{
    *m_list = data;
    m_currentRenderID = renderId;
    [self.tableView reloadData];
}

- (const deejay::DLNAItem*) getMediaBuf
{
    return *m_mediaBuf.GetFirstItem();
}

- (void) shouldSelectRenderForMedia:(const deejay::DLNAItem*)item
{
    m_mediaBuf.Clear();
    m_mediaBuf.Add(item);
    int count = m_list->count();
    if (!count)
    {
        UIAlertView * alert = [[UIAlertView alloc] initWithTitle:nil
//                                                         message:@"No Player is found. To play files on this device, go to Options and select \"Allow play in network\"."
                                                         message:NSLocalizedStringFromTable(@"No player find", @"Localizable", nil)
                                                        delegate:nil
                                               cancelButtonTitle:NSLocalizedStringFromTable(@"ok",@"Localizable", nil)
                                               otherButtonTitles:nil];
        [alert show];
        [alert release];
        return;
    }
    UIActionSheet * renderlist = [[UIActionSheet alloc] 
//                                  initWithTitle:@"Please select a player" 
                                  initWithTitle:NSLocalizedStringFromTable(@"Select player", @"Localizable", nil)  
                                                             delegate:self
                                                    cancelButtonTitle:nil
                                               destructiveButtonTitle:nil
                                                    otherButtonTitles:nil];
    for (int i = 0; i < count; i++)
    {
        NSString * renderTitle = [NSString stringWithUTF8String:m_list->itemAt(i)->friendlyName().GetChars()];
        [renderlist addButtonWithTitle:renderTitle];
    }
    
    [renderlist addButtonWithTitle:NSLocalizedStringFromTable(@"cancel",@"Localizable", nil)];
    [renderlist setDestructiveButtonIndex:count];
    [renderlist setCancelButtonIndex:count];
    [renderlist showFromTabBar:self.tabBarController.tabBar];
    [renderlist release];
}



- (void) refresh
{
    if (!m_nodeviceview.hidden)
    {
        [delegate restartDLNAService];
    }
    [delegate refreshRenderOnly];
}

@end
