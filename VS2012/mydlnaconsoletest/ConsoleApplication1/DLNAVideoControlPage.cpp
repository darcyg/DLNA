//
//  DLNAVideoControlPage.m
//  Genie
//
//  Created by cs Siteview on 11-10-9.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "DLNAVideoControlPage.h"
#import "DLNADeviceController.h"

#define CONTROL_BTN_SIZE_WIDTH              105
#define CONTROL_BTN_SIZE_HEIGHT             90
#define CONTROL_MUTE_BTN_SIZE               30


#define STOP_BTN_TAG                        1986
#define NEXT_BTN_TAG                        1987
#define PREVIOUS_BTN_TAG                    1988
#define PAUSE_PLAY_BTN_TAG                  1989
#define PROGRESS_LABEL_TAG                  1990
#define MUTE_BTN_TAG                        1991


#define VerticalScreen CGRectMake(0.0f, 0.0f, 768.0f, 910.0f)
#define HorizonScreen  CGRectMake(0.0f, 0.0f, 1024.0f, 654.0f)

@implementation DLNAVideoControlPage
@synthesize delegate;

static BOOL isMute = NO;

#pragma mark life-cycle
- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        bg_view = [[UIImageView alloc] initWithFrame:frame];
        bg_view.userInteractionEnabled = YES;
        [self performSelector:@selector(initPageView)];
    }
    return self;
}

- (id) init
{
    return  [self initWithFrame:CGRectZero];
}

- (void)dealloc
{
    [self performSelector:@selector(finishTimerForGetTrack)];
    [leftProgress release];
    [rightProgress release];
    [mediaObjName release];
    [controlBtnBg release];
    [bg_view release];
    [progressView release];
    [volumeView release];
    [thumbImgView release];
    [super dealloc];
}


#pragma  mark UI
- (void) initPageView
{
    controlBtnBg = [[UIImageView alloc] init];
    controlBtnBg.userInteractionEnabled = YES;
    [bg_view addSubview:controlBtnBg];
    
    
    UIButton* stopBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    stopBtn.tag = STOP_BTN_TAG;
    stopBtn.frame = CGRectMake(0, 0, CONTROL_BTN_SIZE_WIDTH, CONTROL_BTN_SIZE_HEIGHT);
    stopBtn.backgroundColor = [UIColor clearColor];
    [stopBtn setBackgroundImage:[UIImage imageNamed:@"stop.png"] forState:UIControlStateNormal];
    [stopBtn addTarget:self action:@selector(stopBtnPress) forControlEvents:UIControlEventTouchUpInside];
    [controlBtnBg addSubview:stopBtn];
    
    UIButton* preBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    preBtn.tag = PREVIOUS_BTN_TAG;
    preBtn.frame = CGRectMake(0, 0, CONTROL_BTN_SIZE_WIDTH, CONTROL_BTN_SIZE_HEIGHT);
    preBtn.backgroundColor = [UIColor clearColor];
    [preBtn setBackgroundImage:[UIImage imageNamed:@"previous.png"] forState:UIControlStateNormal];
    [preBtn addTarget:self action:@selector(previousBtnPress) forControlEvents:UIControlEventTouchUpInside];
    [controlBtnBg addSubview:preBtn];
    
    UIButton* pauseAndPlayBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    pauseAndPlayBtn.tag = PAUSE_PLAY_BTN_TAG;
    pauseAndPlayBtn.frame = CGRectMake(0, 0, CONTROL_BTN_SIZE_WIDTH, CONTROL_BTN_SIZE_HEIGHT);
    pauseAndPlayBtn.backgroundColor = [UIColor clearColor];
    [pauseAndPlayBtn setBackgroundImage:[UIImage imageNamed:@"play.png"] forState:UIControlStateNormal];
    [pauseAndPlayBtn addTarget:self action:@selector(pausePlayBtnPress) forControlEvents:UIControlEventTouchUpInside];
    [controlBtnBg addSubview:pauseAndPlayBtn];
    
    UIButton* nextBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    nextBtn.tag = NEXT_BTN_TAG;
    nextBtn.frame = CGRectMake(0, 0, CONTROL_BTN_SIZE_WIDTH, CONTROL_BTN_SIZE_HEIGHT);
    nextBtn.backgroundColor = [UIColor clearColor];
    [nextBtn setBackgroundImage:[UIImage imageNamed:@"next.png"] forState:UIControlStateNormal];
    [nextBtn addTarget:self action:@selector(nextBtnPress) forControlEvents:UIControlEventTouchUpInside];
    [controlBtnBg addSubview:nextBtn];
    
    
    UIButton* muteBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    muteBtn.tag = MUTE_BTN_TAG;
    muteBtn.frame = CGRectMake(0, 0, CONTROL_MUTE_BTN_SIZE, CONTROL_MUTE_BTN_SIZE);
    muteBtn.backgroundColor = [UIColor clearColor];
    [muteBtn setBackgroundImage: [UIImage imageNamed:@"voice.png"] forState:UIControlStateNormal];
    [muteBtn addTarget:self action:@selector(muteBtnPress) forControlEvents:UIControlEventTouchUpInside];
    [bg_view addSubview:muteBtn];
    
    volumeView = [[UISlider alloc] init];
    [volumeView addTarget:self action:@selector(changeVolume) forControlEvents:UIControlEventTouchUpInside];
    volumeView.frame = CGRectMake(0, 0, 480, 10);
    [bg_view addSubview:volumeView];
    
    progressView = [[UISlider alloc] init];
    [progressView addTarget:self action:@selector(changeProgress) forControlEvents:UIControlEventTouchUpInside];
    progressView.frame = CGRectMake(0, 0, 530, 10);
    progressView.minimumValue = 0;
    [bg_view addSubview:progressView];
    
    leftProgress = [[UILabel alloc] init];
    leftProgress.frame = CGRectMake(0, 0, 120, 30);
    leftProgress.font = [UIFont systemFontOfSize:22];
    leftProgress.backgroundColor = [UIColor clearColor];
    leftProgress.textAlignment = UITextAlignmentCenter;
    leftProgress.textColor = [UIColor whiteColor];
    [self showTrackTime:0 inLabel:leftProgress];
    [bg_view addSubview:leftProgress];
    
    rightProgress = [[UILabel alloc] init];
    rightProgress.frame = CGRectMake(0, 0, 120, 30);
    rightProgress.font = [UIFont systemFontOfSize:22];
    rightProgress.backgroundColor = [UIColor clearColor];
    rightProgress.textAlignment = UITextAlignmentCenter;
    rightProgress.textColor = [UIColor whiteColor];
    [self showTrackTime:0 inLabel:rightProgress];
    [bg_view addSubview:rightProgress];
    
    mediaObjName = [[UILabel alloc] init];
    mediaObjName.frame = CGRectMake(0, 0, 400, 40);
    mediaObjName.font = [UIFont systemFontOfSize:26];
    mediaObjName.backgroundColor = [UIColor clearColor];
    mediaObjName.textAlignment = UITextAlignmentCenter;
    mediaObjName.textColor = [UIColor whiteColor];
    //mediaObjName.text = @"Unknown";
    [bg_view addSubview:mediaObjName];
    
    //2012.2.9
    thumbImgView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"default_thumbimg_video.png"]];
    [bg_view addSubview:thumbImgView];
    //end
    
    [self addSubview:bg_view];
    [self performSelector:@selector(disableUI)];
}

- (void) layoutPageViewWithInterfaceOrientation:(UIInterfaceOrientation) orientation
{
    [self layoutControlBtnsWithOrientation:orientation];
    CGRect rec = self.frame;
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        controlBtnBg.center = CGPointMake(rec.size.width/2, rec.size.height*0.88);
        progressView.center = CGPointMake(rec.size.width/2, rec.size.height*0.65);
        volumeView.center = CGPointMake(rec.size.width/2+25.0f, rec.size.height*0.74);
        [(UIButton*)[bg_view viewWithTag:MUTE_BTN_TAG] setCenter:CGPointMake(volumeView.center.x-volumeView.frame.size.width/2-30, volumeView.center.y)];
        mediaObjName.center = CGPointMake(rec.size.width/2, rec.size.height*0.08);
    }
    else
    {
        controlBtnBg.center = CGPointMake(rec.size.width*0.87, rec.size.height/2);
        progressView.center = CGPointMake(400.0f, rec.size.height*0.72);
        volumeView.center = CGPointMake(400.0f+25.0f, rec.size.height*0.88);
        [(UIButton*)[bg_view viewWithTag:MUTE_BTN_TAG] setCenter:CGPointMake(volumeView.center.x-volumeView.frame.size.width/2-30, volumeView.center.y)];
        mediaObjName.center = CGPointMake(400.0f, rec.size.height*0.05);
    }
    
    leftProgress.center = CGPointMake(progressView.center.x-progressView.frame.size.width/2, progressView.center.y+22);
    rightProgress.center = CGPointMake(progressView.center.x+progressView.frame.size.width/2, progressView.center.y+22);
    [self showThumbImg];
}


- (void) layoutControlBtnsWithOrientation:(UIInterfaceOrientation) orientation
{
    UIImage * img = nil;
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        controlBtnBg.frame = CGRectMake(0, 0, 668, 110);
        img = [UIImage imageNamed:@"controlbtnviewbg.png"];
        [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setCenter:CGPointMake(668/8, 55)];
        [(UIButton*)[controlBtnBg viewWithTag:STOP_BTN_TAG] setCenter:CGPointMake(668/8+668/4, 55)];
        [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setCenter:CGPointMake(668/8+668/2, 55)];
        [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setCenter:CGPointMake(668/8+668*3/4, 55)];
    }
    else
    {
        controlBtnBg.frame = CGRectMake(0, 0, 142, 552);
        img = [UIImage imageNamed:@"controlBtnViewBg_LandscapeLR.png"];
        [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setCenter:CGPointMake(71, 552/8)];
        [(UIButton*)[controlBtnBg viewWithTag:STOP_BTN_TAG] setCenter:CGPointMake(71, 552/8*3)];
        [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setCenter:CGPointMake(71, 552/8*5)];
        [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setCenter:CGPointMake(71, 552/8*7)];
    }
    controlBtnBg.image = img;
}


- (void) transformTo:(UIInterfaceOrientation)orientation withType:(DLNAMediaObjType) type
{
    //背景图片设置为纯黑色图片，忽略MediaOjbType参数  2012.2.9
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
    //[self setBg_imgWithType:type];
    [self layoutPageViewWithInterfaceOrientation:orientation];
}

- (void) setBg_imgWithType:(DLNAMediaObjType) type
{
    //2012.2.9
    /*
    UIImage * bg_img = nil;
    if ((int)self.frame.size.width == 768)
    {
        if (type == DLNAMediaObjTypeAudio)
        {
            bg_img = [UIImage imageNamed:@"music_bg.png"];
        }
        else if(type == DLNAMediaObjTypeVideo)
        {
            bg_img = [UIImage imageNamed:@"video_bg.png"];
        }
        
    }
    else
    {
        if (type == DLNAMediaObjTypeAudio)
        {
            bg_img = [UIImage imageNamed:@"music_bg_LandscapeLR.png"];
        }
        else if (type == DLNAMediaObjTypeVideo)
        {
            bg_img = [UIImage imageNamed:@"video_bg_LandscapeLR.png"];
        }
        
    }
    bg_view.image = bg_img;*/
    if (type == DLNAMediaObjTypeAudio)
    {
        [thumbImgView setImage:[UIImage imageNamed:@"default_thumbimg_music.png"]];
    }
    else
    {
        [thumbImgView setImage:[UIImage imageNamed:@"default_thumbimg_video.png"]];
    }
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
        //thumbImgView.frame = CGRectMake(0, 0, 400.0f, 400.0f);
        [self scaleThumbImgViewIntoSquareWithLength:400.0f];
        thumbImgView.center = CGPointMake(rec.size.width/2, rec.size.height*0.35);
    }
    else
    {
        //2012.2.9
        //thumbImgView.frame = CGRectMake(0, 0, 350.0f, 350.0f);
        [self scaleThumbImgViewIntoSquareWithLength:350.0f];
        thumbImgView.center = CGPointMake(400.0f, rec.size.height*0.37);
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

- (void) disableUI
{
    [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setEnabled:NO];
     [(UIButton*)[controlBtnBg viewWithTag:STOP_BTN_TAG] setEnabled:NO];
     [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setEnabled:NO];
     [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setEnabled:NO];
     [(UIButton*)[bg_view viewWithTag:MUTE_BTN_TAG] setEnabled:NO];
     [progressView setEnabled:NO];
     [volumeView setEnabled:NO];
}
#pragma mark receive control signal
- (void) recevieUPnPControlSignal:(DLNAControlSignal) signal
{
    switch (signal)
    {
        case DLNAControlSignalPlay:
            NSLog(@"receive signal:play");
            {
                [self performSelector:@selector(setPausePlayBtnIconForPause)];
                [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:STOP_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setEnabled:YES];
                [progressView setEnabled:YES];
                [self performSelector:@selector(startTimerForGetTrack)];//9.26
            }
            break;
        case DLNAControlSignalPause:
            NSLog(@"receive signal:pause");
            {
                [self performSelector:@selector(setPausePlayBtnIconForPlay)];
                [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:STOP_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setEnabled:YES];
                [progressView setEnabled:YES];
            }
            break;
        case DLNAControlSignalStop:
            NSLog(@"receive signal:stop");
            {
                [self performSelector:@selector(setPausePlayBtnIconForPlay)];
                [(UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:STOP_BTN_TAG] setEnabled:NO];
                [(UIButton*)[controlBtnBg viewWithTag:NEXT_BTN_TAG] setEnabled:YES];
                [(UIButton*)[controlBtnBg viewWithTag:PREVIOUS_BTN_TAG] setEnabled:YES];
                [self showTrackTime:0 inLabel:leftProgress];
                progressView.value = 0;
                [progressView setEnabled:NO];
                [self performSelector:@selector(finishTimerForGetTrack)];//9.26
            }
            break;
        default:
            break;
    }
}

- (void) startTimerForGetTrack
{
    if (getTrackTimer) 
    {
        return; 
    }
    getTrackTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:delegate selector:@selector(getTrackInfo) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:getTrackTimer forMode:NSDefaultRunLoopMode];
}

- (void) finishTimerForGetTrack
{
    NSLog(@"Finish timer for track!");
    if (getTrackTimer)
    {
        [getTrackTimer invalidate]; 
        getTrackTimer = nil;
    }
}



- (void) receiveSetVolumeSignalWithVolume:(int) volume maxVolume:(int) maxVolume minVolume:(int) minVolume
{
    NSLog(@"receive signal:setVolume");
    volumeView.maximumValue = maxVolume;
    volumeView.minimumValue = minVolume;
    volumeView.value = volume;
}


- (void) receiveSetMuteSignal:(BOOL) needMute
{
    NSLog(@"receive signal:set Mute");
    [(UIButton*)[bg_view viewWithTag:MUTE_BTN_TAG] setEnabled:YES];
    isMute = needMute;
    [volumeView setEnabled:!isMute];
    [self performSelector:@selector(setMuteBtnBgView)];
}
- (void) setMuteBtnBgView
{
    UIButton * muteBtn = (UIButton*)[bg_view viewWithTag:MUTE_BTN_TAG];
    if (isMute)
    {
        [muteBtn setBackgroundImage:[UIImage imageNamed:@"voice_no.png"] forState:UIControlStateNormal];
    }
    else
    {
        [muteBtn setBackgroundImage:[UIImage imageNamed:@"voice.png"] forState:UIControlStateNormal];
    }
}


- (void) receiveSetTotalDuration:(int) duration
{
    NSLog(@"receive signal:set duration");
    progressView.maximumValue = duration;//总时间不是一个稳定的值。有些RENDER要在发送几次状态信息之后才会给出正确的总播放时间
    [self showTrackTime:duration inLabel:rightProgress];
}

- (void) receiveTrackSignal:(int) time
{
    progressView.value = time;
    [self showTrackTime:time inLabel:leftProgress];
}

- (void) showTrackTime:(int) time inLabel:(UILabel*) label
{
    label.text = [NSString stringWithFormat:@"%@",[self formatterTime:time]];
}

- (NSString*) formatterTime:(int) millisecond
{
    NSMutableString* timeStr = [[NSMutableString alloc] init];
    [timeStr setString:@"00:00:00"];
    int totalSeconds = millisecond/1000;
    int hour = totalSeconds/3600;
    int minute = totalSeconds/60%60;
    int second = totalSeconds%60;
    
    int firstNum;
    int secondNum;
    
    firstNum = hour/10;
    secondNum = hour%10;
    if (firstNum>9)
    {
        firstNum = 9;
    }
    [timeStr replaceCharactersInRange:NSMakeRange(0, 1) withString:[NSString stringWithFormat:@"%d",firstNum]];
    [timeStr replaceCharactersInRange:NSMakeRange(1, 1) withString:[NSString stringWithFormat:@"%d",secondNum]];
    
    firstNum = minute/10;
    secondNum = minute%10;
    [timeStr replaceCharactersInRange:NSMakeRange(3, 1) withString:[NSString stringWithFormat:@"%d",firstNum]];
    [timeStr replaceCharactersInRange:NSMakeRange(4, 1) withString:[NSString stringWithFormat:@"%d",secondNum]];
    
    firstNum = second/10;
    secondNum = second%10;

    [timeStr replaceCharactersInRange:NSMakeRange(6, 1) withString:[NSString stringWithFormat:@"%d",firstNum]];
    [timeStr replaceCharactersInRange:NSMakeRange(7, 1) withString:[NSString stringWithFormat:@"%d",secondNum]];
    return [timeStr retain];
}
#pragma mark send out control signal
- (void) stopBtnPress
{
    [delegate sendStopSignal];
}

- (void) pausePlayBtnPress
{
    if (pausePlayBtnType == PLAY_BTN)
    {
        [delegate sendPlaySignal];
    }
    if (pausePlayBtnType == PAUSE_BTN)
    {
        [delegate sendPauseSignal];
    }
}

- (void) previousBtnPress
{
    [delegate previous];
}

- (void) nextBtnPress
{
    [delegate next];
}

- (void) muteBtnPress
{
    [delegate sendMuteBtnPressSignal:!isMute];
}


- (void) changeVolume
{
    [delegate sendChangeVolumeSignal:volumeView.value];
}

- (void) changeProgress
{
    [delegate sendChangeProgressSignal:progressView.value];
}


- (void) setPausePlayBtnIconForPlay
{
    pausePlayBtnType = PLAY_BTN;
    UIButton* playBtn = (UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG];
    [playBtn setBackgroundImage:[UIImage imageNamed:@"play.png"] forState:UIControlStateNormal];
}

- (void) setPausePlayBtnIconForPause
{
    pausePlayBtnType = PAUSE_BTN;
    UIButton* pauseBtn = (UIButton*)[controlBtnBg viewWithTag:PAUSE_PLAY_BTN_TAG];
    [pauseBtn setBackgroundImage:[UIImage imageNamed:@"pause.png"] forState:UIControlStateNormal];
}
@end
