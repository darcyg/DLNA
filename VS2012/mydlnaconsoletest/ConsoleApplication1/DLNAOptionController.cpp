//
//  DLNAOptionController.m
//  GenieHD
//
//  Created by cs Siteview on 11-12-13.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "DLNAOptionController.h"
#import "DLNACenter.h"
#import "DLNAGolbal.h"

@implementation DLNAOptionController
@synthesize delegate;

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
        m_fileShareSwitch = [[UISwitch alloc] init];
        m_fileShareSwitch.on = YES;
        [m_fileShareSwitch addTarget:self action:@selector(fileShareSwitchChanged:) forControlEvents:UIControlEventValueChanged];
        m_allowRenderSwitch = [[UISwitch alloc] init];
        m_allowRenderSwitch.on = YES;
        [m_allowRenderSwitch addTarget:self action:@selector(allowRenderSwitchChanged:) forControlEvents:UIControlEventValueChanged];
    }
    return self;
}

- (void)dealloc
{
    [m_fileShareSwitch release];
    [m_allowRenderSwitch release];
    [super dealloc];
}

- (id) init
{
    return [self initWithStyle:UITableViewStyleGrouped];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
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

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (section == 0)
    {
        return 2;
    }
    else if (section == 1)
    {
        return 2;
    }
    else return 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
    int rowIndex = [indexPath row];
    if (indexPath.section == 0)
    {
        if (rowIndex == 0)
        {
            cell.selectionStyle = UITableViewCellSelectionStyleBlue;
//            cell.textLabel.text = @"Restart media service";
            cell.textLabel.text=NSLocalizedStringFromTable(@"Restart media", @"Localizable", nil);
            cell.textLabel.textAlignment = UITextAlignmentCenter;
        }
        else if (rowIndex == 1)
        {
            cell.selectionStyle = UITableViewCellSelectionStyleBlue;
//            cell.textLabel.text = @"Refresh source folder";
               cell.textLabel.text = NSLocalizedStringFromTable(@"Refresh source", @"Localizable", nil); 
            cell.textLabel.textAlignment = UITextAlignmentCenter;
        }
    }
    else if (indexPath.section == 1)
    {
        if (rowIndex == 0)
        {
            cell.selectionStyle = UITableViewCellSelectionStyleNone;
//            cell.textLabel.text = @"Share files in network";
            cell.textLabel.text = NSLocalizedStringFromTable(@"Share files", @"Localizable", nil);
            cell.accessoryView = m_fileShareSwitch;
        }
        else if (rowIndex == 1)
        {
            cell.selectionStyle = UITableViewCellSelectionStyleNone;
//            cell.textLabel.text = @"Allow play in network";
            cell.textLabel.text =NSLocalizedStringFromTable(@"Allow play", @"Localizable", nil);
            cell.accessoryView = m_allowRenderSwitch;
        }
    }
    return cell;
}


- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
{
    if (!section)
    {
        return nil;
    }
    UIView * footerView = [[[UIView alloc] init] autorelease];
    UILabel * tip = [[UILabel alloc] init];
    tip.font = [UIFont boldSystemFontOfSize:15];
//    tip.text = @"Note:\nRestart media service when the wireless network setting is changed.\nShare file in network to allow people access files of this device over the network.\nAllow play in network to allow people use this device to play files over the network.\nTap Refresh source folder if there's any change in media folder(s).";

    NSString * mediaOptionNote =[NSString stringWithFormat:@"%@\n%@\n%@\n%@\n%@",
     NSLocalizedStringFromTable(@"myMediaOptionNote_title", @"Localizable", nil),
     NSLocalizedStringFromTable(@"myMediaOptionNote_1", @"Localizable", nil),
     NSLocalizedStringFromTable(@"myMediaOptionNote_2", @"Localizable", nil),
     NSLocalizedStringFromTable(@"myMediaOptionNote_3", @"Localizable", nil),
     NSLocalizedStringFromTable(@"myMediaOptionNote_4", @"Localizable", nil)
     ,nil];
//    tip.text=NSLocalizedStringFromTable(@"Media Note", @"Localizable", nil);
    tip.text=mediaOptionNote;

    tip.textAlignment = UITextAlignmentLeft;
    [tip setFrame:CGRectMake(50,0,600,120)];
    tip.backgroundColor = [UIColor clearColor];
    tip.numberOfLines = 0;
    [footerView addSubview:tip];
    [tip release];
    return footerView;
}
- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    if (section == 1)
    {
        return 120;
    }
    return 0;
}
#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.row == 0)
    {
        [delegate restartDLNAService];
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
    }
    else if (indexPath.row == 1)
    {
        [delegate refreshMediaSource];
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
    }
}


#pragma  mark coustom
- (void) setValueForSwitch
{
    NSMutableArray * arr = [delegate readOptionConfig];
    if (!arr)
    {
        m_allowRenderSwitch.on = YES;
        m_fileShareSwitch.on = YES;
    }
    else
    {
        NSString * fileShareSwitch = (NSString*)[arr objectAtIndex:DLNAOptionSettingServerSwitch];
        if ([fileShareSwitch isEqualToString:@"0"])
        {
            m_fileShareSwitch.on = NO;
        }
        else
        {
            m_fileShareSwitch.on = YES;
        }
        NSString * allowrender = (NSString*)[arr objectAtIndex:DLNAOptionSettingRenderSwitch];
        if ([allowrender isEqualToString:@"0"])
        {
            m_allowRenderSwitch.on = NO;
        }
        else
        {
            m_allowRenderSwitch.on = YES;
        }
    }
}

- (void) fileShareSwitchChanged:(id)switcher
{
    UISwitch * sw = (UISwitch*) switcher;
    if (sw.on)
    {
        [delegate openDLNAServer];
    }
    else
    {
        [delegate closeDLNAServer];
    }
    [delegate writeOptionConfigWithServerOn:m_fileShareSwitch.on renderOn:m_allowRenderSwitch.on];
}

- (void) allowRenderSwitchChanged:(id)switcher
{
    UISwitch * sw = (UISwitch*) switcher;
    if (sw.on)
    {
        [delegate openDLNARender];
    }
    else
    {
        [delegate closeDLNARender];
    }
    [delegate writeOptionConfigWithServerOn:m_fileShareSwitch.on renderOn:m_allowRenderSwitch.on];
}

- (void) becomeCurrentPageView
{
    [self setValueForSwitch];
}
@end
