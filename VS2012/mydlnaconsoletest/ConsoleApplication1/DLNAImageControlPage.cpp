//
//  DLNAImageControlPage.m
//  Genie
//
//  Created by cs Siteview on 11-10-11.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "DLNAImageControlPage.h"
#import "DLNADeviceController.h"

#define NEXT_BTN_TAG                        1987
#define PREVIOUS_BTN_TAG                    1988
#define PAUSE_PLAY_BTN_TAG                  1989//2011.11.01

#define CONTROL_BTN_SIZE_WIDTH              105
#define CONTROL_BTN_SIZE_HEIGHT             90

#define VerticalScreen CGRectMake(0.0f, 0.0f, 768.0f, 910.0f)
#define HorizonScreen  CGRectMake(0.0f, 0.0f, 1024.0f, 654.0f)

#define SLIDE_MODE_TABLE_VIEW              1103

@implementation DLNAImageControlPage
@synthesize delegate;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        bg_view = [[UIImageView alloc] init];
        bg_view.userInteractionEnabled = YES;
        isAutoPlaybackOpened = NO;//2011.11.01
        [self performSelector:@selector(initPageView)];
    }
    return self;
}

- (id) init
{
    return [self initWithFrame:CGRectZero];
}

- (void)dealloc
{
    [bg_view release];
    [controlBtnBg release];
    [mediaObjName release];
    [slideShowModeBg release];
    [thumbImgView release];
    
    [super dealloc];
}


#pragma  mark UI
- (void) initPageView
{
    controlBtnBg = [[UIImageView alloc] init];
    controlBtnBg.userInteractionEnabled = YES;
    [bg_view addSubview:controlBtnBg];
    
    UIButton* preBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    preBtn.tag = PREVIOUS_BTN_TAG;
    preBtn.frame = CGRectMake(0, 0, CONTROL_BTN_SIZE_WIDTH, CONTROL_BTN_SIZE_HEIGHT);
    preBtn.backgroundColor = [UIColor clearColor];
    [preBtn setBackgroundImage:[UIImage imageNamed:@"previous.png"] forState:UIControlStateNormal];
    [preBtn addTarget:delegate action:@selector(previous) forControlEvents:UIControlEventTouchUpInside];
    [controlBtnBg addSubview:preBtn];
    
    UIButton* nextBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    nextBtn.tag = NEXT_BTN_TAG;
    nextBtn.frame = CGRectMake(0, 0, CONTROL_BTN_SIZE_WIDTH, CONTROL_BTN_SIZE_HEIGHT);
    nextBtn.backgroundColor = [UIColor clearColor];
    [nextBtn setBackgroundImage:[UIImage imageNamed:@"next.png"] forState:UIControlStateNormal];
    [nextBtn addTarget:delegate action:@selector(next) forControlEvents:UIControlEventTouchUpInside];
    [controlBtnBg addSubview:nextBtn];
    
    //2011.11.01
    UIButton* pauseAndPlayBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    pauseAndPlayBtn.tag = PAUSE_PLAY_BTN_TAG;
    pauseAndPlayBtn.frame = CGRectMake(0, 0, CONTROL_BTN_SIZE_WIDTH, CONTROL_BTN_SIZE_HEIGHT);
    pauseAndPlayBtn.backgroundColor = [UIColor clearColor];
    [pauseAndPlayBtn setBackgroundImage:[UIImage imageNamed:@"play.png"] forState:UIControlStateNormal];
    [pauseAndPlayBtn addTarget:self action:@selector(autoPlaybackBtnPress) forControlEvents:UIControlEventTouchUpInside];
    [controlBtnBg addSubview:pauseAndPlayBtn];
    //end
    
    
    mediaObjName = [[UILabel alloc] init];
    mediaObjName.frame = CGRectMake(0, 0, 400, 50);
    mediaObjName.font = [UIFont systemFontOfSize:32];
    mediaObjName.backgroundColor = [UIColor clearColor];
    mediaObjName.textAlignment = UITextAlignmentCenter;
    mediaObjName.textColor = [UIColor whiteColor];
    //mediaObjName.text = @"Unknown"; 
    [bg_view addSubview:mediaObjName];
    
    //2012.2.9
    thumbImgView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"default_thumbimg_image.png"]];
    [bg_view addSubview:thumbImgView];
    //end
    
    //2011.11.14
    slideShowModeBg = [[UIImageView alloc] init];
    slideShowModeBg.backgroundColor = [UIColor clearColor];
    slideShowModeBg.userInteractionEnabled = YES;
    
    UITableView * slideMode = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStylePlain];
    slideMode.tag = SLIDE_MODE_TABLE_VIEW;
    slideMode.delegate = self;
    slideMode.dataSource = self;
    [slideShowModeBg addSubview:slideMode];
    [slideShowModeBg setHidden:YES];
    [slideMode release];
    [bg_view addSubview:slideShowModeBg];
    //end 1114
    
    [self addSubview:bg_view];
}



//2011.11.14
- (void) showSlideModeSelectViewWithOrientation:(UIInterfaceOrientation) orientation
{
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        slideShowModeBg.frame = CGRectMake(0, 0, 620, 380+30);
        slideShowModeBg.image = [UIImage imageNamed:@"speedMode_bg.png"];
        UITableView * slideModeTableView = (UITableView*)[slideShowModeBg viewWithTag:SLIDE_MODE_TABLE_VIEW];
        slideModeTableView.frame = CGRectMake(25, 100, 570, 250);
        slideShowModeBg.center = CGPointMake(self.frame.size.width/2, self.frame.size.height*0.58);
        
    }
    else
    {
        slideShowModeBg.frame = CGRectMake(0, 0, 620, 380);
        slideShowModeBg.image = [UIImage imageNamed:@"speedMode_bg_LandscapeLR.png"];
        UITableView * slideModeTableView = (UITableView*)[slideShowModeBg viewWithTag:SLIDE_MODE_TABLE_VIEW];
        slideModeTableView.frame = CGRectMake(25, 100, 540, 250);
        slideShowModeBg.center = CGPointMake(520.0f, self.frame.size.height/2-30);
    }
}
//end 2011.11.14




- (void) layoutPageViewWithInterfaceOrientation:(UIInterfaceOrientation) orientation
{
    [self showSlideModeSelectViewWithOrientation:orientation];//2011.11.15
    [self layoutControlBtnsWithOrientation:orientation];
    CGRect rec = self.frame;
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        controlBtnBg.center = CGPointMake(rec.size.width/2, rec.size.height*0.88);
        mediaObjName.center = CGPointMake(rec.size.width/2, rec.size.height*0.06);
    }
    else
    {
        controlBtnBg.center = CGPointMake(rec.size.width*0.9, rec.size.height/2);
        mediaObjName.center = CGPointMake(380.0f, rec.size.height*0.06);
    }
    [self showThumbImg];
}


- (void) layoutControlBtnsWithOrientation:(UIInterfaceOrientation) orientation
{
    UIImage * img = nil;
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        controlBtnBg.frame = CGRectMake(0, 0, 668, 110);
        img = [UIImage imageNamed:@"controlbtnviewbg.png"];
        [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setCenter:CGPointMake(668/6, 55)];
        [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setCenter:CGPointMake(668/6+668/3, 55)];
        [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setCenter:CGPointMake(668/6+668*2/3, 55)];//2011.11.01
    }
    else
    {
        controlBtnBg.frame = CGRectMake(0, 0, 142, 552);
        img = [UIImage imageNamed:@"controlBtnViewBg_LandscapeLR.png"];
        [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setCenter:CGPointMake(71, 552/6)];
        [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setCenter:CGPointMake(71, 552/2)];
        [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setCenter:CGPointMake(71, 552/6*5)];//2011.11.01
    }
    controlBtnBg.image = img;
}


- (void) transformTo:(UIInterfaceOrientation)orientation
{
    m_orientation = orientation;
    UIImage * img = nil;
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        self.frame = VerticalScreen;
        img = [UIImage imageNamed:@"control_page_bg.png"];
    }
    else
    {
        self.frame = HorizonScreen;
        img = [UIImage imageNamed:@"control_page_bg_landscape.png"];
    }
    [bg_view setFrame:self.frame];
    bg_view.image = img;
    [self layoutPageViewWithInterfaceOrientation:orientation];
}

- (void) setMediaObjInfo:(NSArray*) info
{
    mediaObjName.text = [info objectAtIndex:0];
}

- (void) setThumbImg:(NSData*) imgData
{
    if ([imgData length])
    {
        UIImage * img = [UIImage imageWithData:imgData];
        [thumbImgView setImage:img];
        [self showThumbImg];
    }
}

- (void) showThumbImg
{
    CGRect rec = self.frame;
    if (UIInterfaceOrientationIsPortrait(m_orientation))
    {
        //2012.2.9
        //thumbImgView.frame = CGRectMake(0, 0, 450.0f, 450.0f);
        [self scaleThumbImgViewIntoSquareWithLength:450.0f];
        thumbImgView.center = CGPointMake(rec.size.width/2, rec.size.height*0.4);
    }
    else
    {
        //2012.2.9
        //thumbImgView.frame = CGRectMake(0, 0, 450.0f, 450.0f);
        [self scaleThumbImgViewIntoSquareWithLength:450.0f];
        thumbImgView.center = CGPointMake(380.0f, rec.size.height*0.5);
    }
}

- (void) scaleThumbImgViewIntoSquareWithLength:(float)length
{
    if (!thumbImgView.image)
    {
        return ;
    }
    CGSize s = thumbImgView.image.size;
    float big = s.width>s.height?s.width:s.height;
    float scale = length/big;
    thumbImgView.frame = CGRectMake(0, 0, s.width*scale, s.height*scale);
}


#pragma mark slide show 
//2011.11.15
- (void) resignSlideShowModeSelectView
{
    if (slideShowModeBg.hidden == NO)
    {
        [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setEnabled:YES];
        [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setEnabled:YES];
        [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setEnabled:YES];
        [slideShowModeBg setHidden:YES];
    }
}
//2011.11.15

//2011.11.01
- (void) autoPlaybackBtnPress
{
    if (isAutoPlaybackOpened)
    {
        [self btnPressForCloseSlideShow];
    }
    else
    {
        [self btnPressForOpenSlideShow];
    }
}
//end

- (void) btnPressForOpenSlideShow
{
    if (!isAutoPlaybackOpened)
    {
        [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setEnabled:NO];
        [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setEnabled:NO];
        [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setEnabled:NO];
        [slideShowModeBg setHidden:NO];
    }
}
- (void) btnPressForCloseSlideShow
{
    if (isAutoPlaybackOpened)
    {
        UIButton* btn = (UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG];
        [btn setBackgroundImage:[UIImage imageNamed:@"play.png"] forState:UIControlStateNormal];
        [delegate performSelector:@selector(closeAutoPlaybackPic)];
        [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setEnabled:YES];
        [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setEnabled:YES];
        isAutoPlaybackOpened = NO;
    }
}

//2011.11.02
- (void) openAutoPlayback
{
    [slideShowModeBg setHidden:YES];
    UIButton* btn = (UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG];
    [btn setBackgroundImage:[UIImage imageNamed:@"stop.png"] forState:UIControlStateNormal];
    [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setEnabled:YES];
    isAutoPlaybackOpened = YES;
    [delegate openAutoPlaybackPicWithSpeed:slideSpeed];
}

#pragma  mark tableview delegate datasource
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section;
{
    return 3;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSString *CellIdentifier = @"cell";
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
	if (cell == nil) 
	{
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    NSString * str = nil;
    NSInteger index = [indexPath row];
    if(index == 0)
    {
        str = @"Slow";
    }
    else if (index == 1)
    {
        str =  @"Normal";
    }
    else
    {
        str = @"Fast";
    }
    cell.textLabel.text = str;
    //cell.textLabel.textAlignment = UITextAlignmentCenter;
    cell.textLabel.font = [UIFont systemFontOfSize:30];
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 80;
}
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return NO;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    int index = [indexPath row];
    if(index == 0)
    {
        slideSpeed = 30;
    }
    else if (index == 1)
    {
        slideSpeed = 20;
    }
    else
    {
        slideSpeed = 10;
    }
    [self performSelector:@selector(openAutoPlayback)];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
