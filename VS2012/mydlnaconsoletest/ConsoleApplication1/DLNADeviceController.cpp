//
//  DLNADeviceController.m
//  Genie
//
//  Created by cs Siteview on 11-9-16.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "DLNADeviceController.h"
#import "DLNACenter.h"
#import <MediaPlayer/MediaPlayer.h>
#import "DLNAVideoControlPage.h"
#import "DLNAImageControlPage.h"
#import "MyScrollView.h"


#define VerticalScreen CGRectMake(0.0f, 0.0f, 768.0f, 910.0f)
#define HorizonScreen  CGRectMake(0.0f, 0.0f, 1024.0f, 654.0f)
#define AUTO_PLAYBACK_TIMEOUT   5
/////////////
@implementation DLNADeviceController
@synthesize delegate;
@synthesize moviePlayController;


static BOOL currentViewIsControlPointView   = YES;
static BOOL isMoviePlayControllerHidded = YES;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        videoControlPage = [[DLNAVideoControlPage alloc] init];
        videoControlPage.delegate = self;
        imgControlPage = [[DLNAImageControlPage alloc] init];
        imgControlPage.delegate = self;
        [self performSelector:@selector(initControlPointView)];
        [self performSelector:@selector(initMediaPlaybackView)];
        
        //2011.10.14   为静态数据初始化
        currentViewIsControlPointView = YES;
        isMoviePlayControllerHidded = YES;
        needAutoPlayBackPicture = NO;
    }
    return self;
}

- (id) init
{
    return  [self initWithNibName:nil bundle:nil];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self performSelector:@selector(finishTimerForReleaseTrackInfo)];
    [self performSelector:@selector(finishAutoPlayBackPicture)];
    [controlPointView release];
    [videoControlPage release];
    [imgControlPage release];
    
    [mediaPlaybackView release];
    [moviePlayController release];
    [imgView release];
    [notSupportDig release];
    [m_imgData release];
    [super dealloc];
}

- (void) finishTimerForGetTrack
{
    [videoControlPage performSelector:@selector(finishTimerForGetTrack)];
}


- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle


// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView
{
    UIView* bgView = [[UIView alloc] init];
    self.view = bgView;
    bgView.backgroundColor = [UIColor clearColor];
    [bgView release];
}



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(playBackStateChanged:) 
                                                 name:MPMoviePlayerPlaybackStateDidChangeNotification 
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(playBackDidFinish:) 
                                                 name:MPMoviePlayerPlaybackDidFinishNotification 
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(playBackIsPreparedToPlay:) 
                                                 name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification 
                                               object:nil];

    UIImage * swImg = [UIImage imageNamed:@"change.png"];
    UIImageView * swImgView = [[UIImageView alloc] initWithImage:swImg];
    swImgView.frame = CGRectMake(0, 0, swImg.size.width, swImg.size.height);
    UIButton * btn = [UIButton buttonWithType:UIButtonTypeCustom];
    btn.frame = swImgView.frame;
    [btn addTarget:self action:@selector(ChangeView) forControlEvents:UIControlEventTouchUpInside];
    [btn addSubview:swImgView];
    [swImgView release];
    UIBarButtonItem * swBtn = [[UIBarButtonItem alloc] initWithCustomView:btn];
    swBtn.style = UIBarButtonItemStyleBordered;
    self.navigationItem.rightBarButtonItem = swBtn;
    [swBtn release];
	
    //2011.10.10
    [self.view addSubview:controlPointView];//default view is control page
    [self performSelector:@selector(checkChangeViewBtnState)];
}


- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return YES;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    [self showViewWithOrientation:toInterfaceOrientation];
}
- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [self showViewWithOrientation:self.interfaceOrientation];
}

#pragma mark coustom function
- (BOOL) currentRenderIsSelf
{
    return [delegate currentRenderIsSelf];
}
- (void) becomeCurrentPageView
{
    
}
- (void) resignCurrentPageView
{
    if (needAutoPlayBackPicture)// 切换界面时，取消幻灯片功能
    {
        [imgControlPage btnPressForCloseSlideShow];
    }
    else
    {
        [imgControlPage performSelector:@selector(resignSlideShowModeSelectView)];
    }
}

- (void) ChangeView
{
    if (currentViewIsControlPointView)
    {
        [controlPointView removeFromSuperview];
        [self.view addSubview:mediaPlaybackView];
    }
    else
    {
        [mediaPlaybackView removeFromSuperview];
        [self.view addSubview:controlPointView];
    }
    currentViewIsControlPointView = !currentViewIsControlPointView;
}

- (void) checkChangeViewBtnState
{
    if ([mediaPlaybackView.subviews count])
    {
        self.navigationItem.rightBarButtonItem.enabled = YES;
    }
    else
    {
        self.navigationItem.rightBarButtonItem.enabled = NO;
    }
}

- (void) showViewWithOrientation:(UIInterfaceOrientation) orientation
{
    if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
    {
        self.view.frame = VerticalScreen;
    }
    else
    {
        self.view.frame = HorizonScreen;
    }
    [self transformViewTo:orientation];
}

- (void) transformViewTo:(UIInterfaceOrientation) orientation
{
    controlPointView.frame = self.view.frame;
    mediaPlaybackView.frame = self.view.frame;
    [videoControlPage transformTo:orientation withType:currentMediaObjType];
    [imgControlPage transformTo:orientation];
    [self rotateRenderViewTo:orientation];
}

- (void) rotateRenderViewTo:(UIInterfaceOrientation) orientation
{
    if (moviePlayController)
    { 
        moviePlayController.view.frame = mediaPlaybackView.frame;
    }
    [imgView setRect:mediaPlaybackView.frame];
}

//2012.2.9
- (void) setThumbImg:(NSData*) imgData withMediaObjType:(DLNAMediaObjType)type
{
    if (type == DLNAMediaObjTypeImage)
    {
        [imgControlPage setThumbImg:imgData];
    }
    else 
    {
        [videoControlPage setThumbImg:imgData];
    }
}

- (void) disableControlPointView
{
    [videoControlPage performSelector:@selector(disableUI)];
}


- (void) setPlayList:(const deejay::DLNAObjectList&) playlist withNowPlayIndex:(NSInteger) index
{
    playList = playlist;
    nowPlayIndexAtPlayList = index;
}


- (NSArray*) getCurrentMediaInfo
{
    NSMutableArray* tmp = [[NSMutableArray alloc] initWithObjects:@"UnKnown", nil];
    const deejay::DLNAItem* mediaItem = playList.itemAt(nowPlayIndexAtPlayList)->asItem();
    NSString * title = [NSString stringWithUTF8String:mediaItem->title().GetChars()];
    [tmp replaceObjectAtIndex:0 withObject:title];
    return [tmp autorelease];
}

//2011.11.02
- (void) resignCurrentPage
{
    
}

- (NSInteger) getMediaType:(const deejay::DLNAObject*)media
{
    return [delegate getMediaObjType:media];
}
#pragma mark ---------------------------as control point
- (void) initControlPointView
{
    ///////////////2011.10.10    
    controlPointView = [[UIView alloc] init];
    controlPointView.backgroundColor = [UIColor clearColor];
    
    [controlPointView addSubview:videoControlPage];
    currentMediaObjType = DLNAMediaObjTypeVideo;//default control page is videocontrol page
    
}

//2011.10.10 
- (void) loadControlPointViewWithMediaObjType:(DLNAMediaObjType)type
{
    //needshow 用来标记是否需要显示本次加载的控制界面
    //一般情况下，当RENDER为本机时，优先显示播放界面 所以此时 needshow为假 
    BOOL needshow = YES;
    if ([self currentRenderIsSelf])
    {
        needshow = NO;
    }
    currentMediaObjType = type;
    for (UIView* view in [controlPointView subviews])
    {
        [view removeFromSuperview];
    }
    switch (type)
    {
        case DLNAMediaObjTypeVideo:
        case DLNAMediaObjTypeAudio:
        {
            //2011.11.02  本身作为RENDER时，若播放音乐，缺省界面为控制界面，其他情况下，缺省界面是播放界面
            if (type == DLNAMediaObjTypeAudio)
            {
                needshow = YES;
            }
            //end 1102
            NSArray * arr = [self getCurrentMediaInfo];
            [videoControlPage setMediaObjInfo:arr];
            [videoControlPage setBg_imgWithType:type];
            [controlPointView addSubview:videoControlPage];
            break;
        }
            
        case DLNAMediaObjTypeImage:
            if (needAutoPlayBackPicture)
            {
                [self startAutoPlayBackPicture];
            }
            [imgControlPage setMediaObjInfo:[self getCurrentMediaInfo]];
            [controlPointView addSubview:imgControlPage];
            break;
            
        default:
            [controlPointView addSubview:videoControlPage];
            break;
    }
    if (!needshow)
    {
        return;
    }
    else
    {
        if (!currentViewIsControlPointView)
        {
            [self performSelector:@selector(ChangeView)];
        }
    }
}

#pragma mark  send out control signal
- (void) sendStopSignal
{
    [delegate stop];
}
- (void) sendPauseSignal
{
    [delegate pause];
}
- (void) sendPlaySignal
{
    [delegate play];
}

- (void) sendMuteBtnPressSignal:(BOOL) mute
{
    [delegate setMute:mute];
}

- (void) sendChangeVolumeSignal:(int) volume
{
    [delegate setVolume:volume];
}

- (void) sendChangeProgressSignal:(int) progress
{
    [delegate setProgress:progress];
}

- (void) next
{
    int total = playList.count();
    if (!total)
    {
        return;
    }
    
    deejay::DLNAObject* obj;
    nowPlayIndexAtPlayList++;
    while (1)
    {
        if (nowPlayIndexAtPlayList >= total)
        {
            nowPlayIndexAtPlayList = 0;
        }
        obj = playList.itemAt(nowPlayIndexAtPlayList);
        if (obj->asContainer())
        {
            nowPlayIndexAtPlayList++;
            continue;
        }
        else
        {
            [delegate openMediaObj:obj->asItem()];
            break;
        }
    }
}

- (void) previous
{
    int total = playList.count();
    if (!total)
    {
        return;
    }
    
    deejay::DLNAObject* obj;
    nowPlayIndexAtPlayList--;
    while (1)
    {
        if (nowPlayIndexAtPlayList < 0)
        {
            nowPlayIndexAtPlayList = total-1;
        }
        obj = playList.itemAt(nowPlayIndexAtPlayList);
        if (obj->asContainer())
        {
            nowPlayIndexAtPlayList--;
            continue;
        }
        else
        {
            [delegate openMediaObj:obj->asItem()];
            break;
        }
    }
}

- (void) getTrackInfo
{
    NSLog(@"query track info timer is working now!");
    [delegate getTrackInfo];
}

#pragma mark receive state change signal
   //local device as control point

- (void) receiveUPnPControlSignal:(DLNAControlSignal) signal
{
    switch (currentMediaObjType)
    {
        case DLNAMediaObjTypeVideo:
        case DLNAMediaObjTypeAudio:
            [videoControlPage recevieUPnPControlSignal:signal];
            break;
            
        case DLNAMediaObjTypeImage:
            break;
            
        default:
            break;
    }
}
- (void) receiveSetVolumeSignalWithVolume:(int) volume maxVolume:(int) maxVolume minVolume:(int) minVolume
{
    [videoControlPage receiveSetVolumeSignalWithVolume:volume maxVolume:maxVolume minVolume:minVolume];
}
- (void) receiveSetMuteSignal:(BOOL) needMute
{
    [videoControlPage receiveSetMuteSignal:needMute];
}
- (void) receiveSetTotalDuration:(int) duration
{
    [videoControlPage receiveSetTotalDuration:duration];
}

- (void) receiveTrackSignal:(int) time
{
    [videoControlPage receiveTrackSignal:time];
}

# pragma  mark ------------slide show
- (void) resignSlideShowModeSelectView
{
    if (imgControlPage)
    {
        [imgControlPage performSelector:@selector(resignSlideShowModeSelectView)];
    }
}

- (void) openAutoPlaybackPicWithSpeed:(NSInteger)speed
{
    NSLog(@" open slide picture");
    needAutoPlayBackPicture = YES;
    slideSpeed = speed;
    [self autoPlayBackPicture];
}
- (void) closeAutoPlaybackPic
{
    NSLog(@"close slide picture");
    [self finishAutoPlayBackPicture];
    needAutoPlayBackPicture = NO;
}

- (void) startAutoPlayBackPicture
{
    if (autoPlayBackPicTimer) 
    {
        [autoPlayBackPicTimer invalidate];
        autoPlayBackPicTimer = nil;
    }
    autoPlayBackPicTimer = [NSTimer scheduledTimerWithTimeInterval:slideSpeed target:self selector:@selector(autoPlayBackPicture) userInfo:nil repeats:NO];
}
- (void) finishAutoPlayBackPicture
{
    if (autoPlayBackPicTimer && [autoPlayBackPicTimer isValid])
    {
        [autoPlayBackPicTimer invalidate];
        autoPlayBackPicTimer = nil;
    }
}
- (void) autoPlayBackPicture
{
    int total = playList.count();
    if (!total)
    {
        return;
    }
    deejay::DLNAObject* obj;
    nowPlayIndexAtPlayList++;
    while (1)
    {
        if (nowPlayIndexAtPlayList >= total)
        {
            nowPlayIndexAtPlayList = 0;
        }
        
        obj = playList.itemAt(nowPlayIndexAtPlayList);
        if (obj->asItem() && DLNAMediaObjTypeImage==[self getMediaType:obj->asItem()])
        {
            //[delegate openMediaObj:obj->asItem()];
            [delegate autoPlaybackMediaObj:obj->asItem() timeOut:AUTO_PLAYBACK_TIMEOUT];
            break;
        }
        else
        {
            nowPlayIndexAtPlayList++;
            continue;
        }
    }
}

- (void) autoPlaybackTimeOut
{
    /*UIAlertView * tip = [[UIAlertView alloc] initWithTitle:nil
                                                   message:@"Timeout" 
                                                  delegate:nil
                                         cancelButtonTitle:@"Ok"
                                         otherButtonTitles:nil];
    [tip show];
    [tip release];*/
    [imgControlPage btnPressForCloseSlideShow];
}
#pragma mark --------------------------- as render
/////////////////////////////////////////////
//// as  render
////////////////////////////////////////////
- (void) initMediaPlaybackView
{
    mediaPlaybackView = [[UIView alloc] init];
    mediaPlaybackView.backgroundColor = [UIColor clearColor];
}

- (void) loadVideo:(NSString*)url
{
    isMoviePlayControllerHidded = NO;
    if (moviePlayController)
    {
        [moviePlayController stop];
    }
    else
    {
        moviePlayController = [[MPMoviePlayerController alloc] init];
        moviePlayController.scalingMode = MPMovieScalingModeAspectFit;
        moviePlayController.view.backgroundColor = [UIColor blackColor];
    }
    NSURL * mediaUrl = [[NSURL alloc] initWithString:url];
    moviePlayController.contentURL = mediaUrl;
    [moviePlayController prepareToPlay];
    [mediaUrl release];
    
    for (UIView* view in mediaPlaybackView.subviews)
    {
        [view removeFromSuperview];
    }
    [mediaPlaybackView addSubview:moviePlayController.view];
}

- (void) loadAudio:(NSString*)url
{
    isMoviePlayControllerHidded = NO;
}

- (void) loadImage:(NSString*)url
{
    isMoviePlayControllerHidded = YES;
    if (moviePlayController)
    {
        [moviePlayController stop];
    } 
    if (imgView)
    {
        [imgView release];
        imgView = nil;
    }
    imgView = [[MyScrollView alloc] init];
    NSURL * imgUrl = [[NSURL alloc] initWithString:url];
    NSURLRequest * imgReq = [[NSURLRequest alloc] initWithURL:imgUrl];
    [imgUrl release];
    if (m_imgData)
    {
        [m_imgData setLength:0];
    }
    else
    {
        m_imgData = [[NSMutableData alloc] init];
    }
    [self getImgData:imgReq];
    
    /*NSData * img = [NSURLConnection sendSynchronousRequest:imgReq returningResponse:nil error:nil];
    [imgReq release];
    UIImage * image = [[UIImage alloc] initWithData:img];
    [imgView setImage:image];
    [image release];*/
    
    [imgReq release];
    UIImage * image = [[UIImage alloc] initWithData:m_imgData];
    [imgView setImage:image];
    [image release];
    for (UIView* view in mediaPlaybackView.subviews)
    {
        [view removeFromSuperview];
    }
    [mediaPlaybackView addSubview:imgView];
}


- (void) loadMediaWithType:(DLNAMediaObjType)type andURL:(NSString*)url
{
    NSLog(@"render load media type:%d  url:%@",type,url);
    switch (type)
    {
        case DLNAMediaObjTypeVideo:
            [self loadVideo:url];
            break;
            
        case DLNAMediaObjTypeAudio:
            //[self loadAudio:url];
            [self loadVideo:url];
            break;
            
        case DLNAMediaObjTypeImage:
            [self loadImage:url];
            break;
            
        default:
            break;
    }
    if (currentViewIsControlPointView)
    {
        [self performSelector:@selector(ChangeView)];
    }
    [self performSelector:@selector(checkChangeViewBtnState)];
    [self showViewWithOrientation:self.interfaceOrientation];

    if (type == DLNAMediaObjTypeAudio || type == DLNAMediaObjTypeVideo)
    {
        if (!loadWaitingView)
        {
            NSString * title = NSLocalizedStringFromTable(@"Wait Message", @"Localizable", nil) ;
            title = [title stringByAppendingString:@"\n\n\n"];
            loadWaitingView = [[UIAlertView alloc] initWithTitle:title message:nil delegate:self cancelButtonTitle:NSLocalizedStringFromTable(@"cancel",@"Localizable",nil ) otherButtonTitles: nil];
            [loadWaitingView show];
        }
    }
}
- (void) onRenderPlay
{
    NSLog(@"render play");
    if (moviePlayController && !isMoviePlayControllerHidded)
    {
        [moviePlayController play];
    }
}
- (void) onRenderPause
{
    NSLog(@"render Pause");
    if(moviePlayController && !isMoviePlayControllerHidded)
    {
        [moviePlayController pause];
    }
}
- (void) onRenderStop
{
    NSLog(@"render stop");
    if (moviePlayController && !isMoviePlayControllerHidded)
    {
        [moviePlayController stop];
    }
}

- (void) onRenderSeekTo:(long long) timeInMillis
{
    NSLog(@"render seek to:%lld",timeInMillis);
    if (moviePlayController && !isMoviePlayControllerHidded)
    {
        [moviePlayController setCurrentPlaybackTime:timeInMillis/1000];
    }
}
- (void) onRenderSetMute:(BOOL)mute
{
    NSLog(@"render set Mute:%d",mute);
}
- (void) onRenderSetVolume:(int) volume
{
    NSLog(@"render set Volume:%d",volume);
}

#pragma mark     send out state change
- (void) startTimerForReleaseTrackInfo
{
    if (releaseTrackTimer) 
    {
        return; 
    }
    releaseTrackTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(releaseTrackInfo) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:releaseTrackTimer forMode:NSDefaultRunLoopMode];
}

- (void) finishTimerForReleaseTrackInfo
{
    if (releaseTrackTimer)
    {
        [releaseTrackTimer invalidate]; 
        releaseTrackTimer = nil;
    }
}

- (void) releaseTrackInfo
{
    NSLog(@"release track info");
    if (moviePlayController && !isMoviePlayControllerHidded)
    {
        [delegate renderReportProgress:moviePlayController.currentPlaybackTime withDuration:moviePlayController.duration];
    }
}

- (void) playBackStateChanged:(NSNotification*)notify
{
    NSInteger newState = DLNAMediaPlayBackStateNone;
    switch (moviePlayController.playbackState)
    {
            
        case MPMoviePlaybackStateStopped:
            [self performSelector:@selector(finishTimerForReleaseTrackInfo)];
            newState = DLNAMediaPlayBackStateStop;
            break;
            
        case MPMoviePlaybackStatePlaying:
            [self performSelector:@selector(startTimerForReleaseTrackInfo)];
            newState = DLNAMediaPlayBackStatePlaying;
            break;
            
        case MPMoviePlaybackStatePaused:  
            [self performSelector:@selector(finishTimerForReleaseTrackInfo)];
            newState = DLNAMediaPlayBackStatePause;
            break;
            
        default:
            newState = DLNAMediaPlayBackStateNone;
            break;
    }
    [delegate renderReportPlayBackState:newState];
}

- (void) playBackDidFinish:(NSNotification*)notify
{
    int finishReason = [[[notify userInfo] objectForKey:MPMoviePlayerPlaybackDidFinishReasonUserInfoKey] intValue];
    if (finishReason == MPMovieFinishReasonPlaybackError)
    {
        //NSLog(@"finish for  MPMovieFinishReasonPlaybackError");
        //media type is not supported
        //2011.10.25
        [loadWaitingView dismissWithClickedButtonIndex:0 animated:YES];
        [loadWaitingView release];
        loadWaitingView = nil;
        if (!notSupportDig)
        {
            notSupportDig = [[UIAlertView alloc] initWithTitle:nil 
//                                                             message:@"Can not support the media file" 
                                                      message:NSLocalizedStringFromTable(@"Media not support", @"Localizable", nil) 
                                                      delegate:self 
                                                   cancelButtonTitle:NSLocalizedStringFromTable(@"ok",@"Localizable",nil ) 
                                                   otherButtonTitles:nil];
            [notSupportDig show];
        }
        //end
    }
    //[moviePlayController stop];
}


- (void) playBackIsPreparedToPlay:(NSNotification*)  notify
{
    if (moviePlayController.isPreparedToPlay)
    {
        [loadWaitingView dismissWithClickedButtonIndex:0 animated:NO];
        [loadWaitingView release];
        loadWaitingView = nil;
        [delegate renderReportProgress:0 withDuration:moviePlayController.duration];
    }
}

///////////
//STOP
///////////
- (void) STOP
{
    if (moviePlayController)
    {
        [moviePlayController stop];
    }
}

#pragma mark alert delegate
- (void)willPresentAlertView:(UIAlertView *)alertView
{
    if (alertView == loadWaitingView)
    {
        UIActivityIndicatorView * aiv = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
        aiv.center = CGPointMake(alertView.bounds.size.width/2.0f, alertView.bounds.size.height/2.0f-15);
        [aiv startAnimating];
        [alertView addSubview:aiv];
        [aiv release];
    }
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if ([alertView isEqual:loadWaitingView])
    {
        [moviePlayController stop];
        loadWaitingView = nil;
        [self performSelector:@selector(playbackError)];
        return;
    }
    if ([alertView isEqual:notSupportDig])
    {
        [notSupportDig release];
        notSupportDig = nil;
        [self performSelector:@selector(playbackError)];
        return;
    }
}


- (void) playbackError
{
    [delegate renderReportPlayBackState:DLNAMediaPlayBackStateErr];
}



#pragma mark get img data
//////////////

- (void) getImgData:(NSURLRequest*)req
{
    [NSThread detachNewThreadSelector:@selector(getImgOnThread:) toTarget:self withObject:req];
    CFRunLoopRun();
}

- (void) getImgOnThread:(NSURLRequest*)req
{
    NSAutoreleasePool * tmpPool = [[NSAutoreleasePool alloc] init];
    [m_imgData setData:[NSURLConnection sendSynchronousRequest:req returningResponse:nil error:nil]];
    [self performSelectorOnMainThread:@selector(wakeup) withObject:nil waitUntilDone:NO];
    [tmpPool release];
}
- (void) wakeup
{
    CFRunLoopStop(CFRunLoopGetCurrent());
}
////////////////////////////////////////
@end
