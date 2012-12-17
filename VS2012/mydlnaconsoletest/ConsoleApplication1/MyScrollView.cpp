//
//  MyScrollView.m
//  ScollImage
//
//  Created by cs Siteview on 11-10-24.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import "MyScrollView.h"


@implementation MyScrollView

- (id) initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        imgView = [[UIImageView alloc] init];
        [self addSubview:imgView];
        [self setDelegate:self];
        self.backgroundColor = [UIColor blackColor];
        self.scrollEnabled = YES;
        self.showsVerticalScrollIndicator = YES;
        self.showsHorizontalScrollIndicator = YES;
        self.indicatorStyle = UIScrollViewIndicatorStyleWhite;
    }
    return  self;
}

- (id) init
{
    return [self initWithFrame:CGRectZero];
}


- (void) dealloc
{
    [imgView release];
    [super dealloc];
}

#pragma  mark---- 
- (void) setImage:(UIImage*) img
{
    //初始化图片初次显示时的参数：以图片的原始大小进行显示。
    imgView.image = img;
    imgView.frame = CGRectMake(0, 0, img.size.width, img.size.height);
    self.contentSize = img.size;
}
- (void) setRect:(CGRect)rec
{
    self.frame = rec;
    [self performSelector:@selector(setImageViewShowInCenter)];
    [self performSelector:@selector(setUpZoomBound)];
}


- (void) setUpZoomBound//根据需要显示的图片的SIZE设置缩放的范围
{
    CGSize imgSize = imgView.image.size;//图片比屏幕小时，缩放范围：0.8~将较小的值放大到屏幕尺寸
    CGSize screenSize = self.frame.size;
    
    float scaleWidth = 1.0f;
    float scaleHeight = 1.0f;
    if (imgSize.width <= screenSize.width && imgSize.height <= screenSize.height)
    {
        scaleWidth = screenSize.width/imgSize.width;
        scaleHeight = screenSize.width/imgSize.height;
        self.maximumZoomScale = scaleWidth>scaleHeight?scaleWidth:scaleHeight;
        self.minimumZoomScale = 0.8f;
    }
    else//图片比屏幕大时，缩放范围：将较大的值缩小为屏幕的0.95~原图尺寸（1.0）
    {
        scaleWidth = screenSize.width/imgSize.width*0.95;
        scaleHeight = screenSize.height/imgSize.height*0.95;
        self.maximumZoomScale = 1.0;
        self.minimumZoomScale = scaleWidth<scaleHeight?scaleWidth:scaleHeight;
    }
    //
    self.zoomScale = self.minimumZoomScale;
}

/*- (void) resolveRotationBug//解决如下问题：1、图片发生旋转，旋转前，图片的宽或者高中有一个值小于屏幕尺寸，从而使得屏幕上留有黑色背景
                           //旋转后，图片留有黑色背景的那个尺寸值却依然大于当前屏幕对应的尺寸值，从而使得屏幕不能居中显示  
{                    
    if (isRotating)
    {
        CGRect rec1 = imgView.frame;
        CGRect rec2 = self.frame;
        if (rec1.size.height <= rec2.size.height || rec1.size.width <= rec2.size.width)
        {
            self.zoomScale = self.minimumZoomScale;
        }
        isRotating = NO;
    }
}*/

- (void) setImageViewShowInCenter//当图片SIZE 的宽或者高小于 SCROLLVIEW 时，进行相应的居中处理
{
    CGRect rec1 = imgView.frame;
    CGRect rec2 = self.frame;
    if (rec1.size.height <= rec2.size.height)
    {
        imgView.center = CGPointMake(imgView.center.x, rec2.size.height/2.0f);
    }
    if (rec1.size.width <= rec2.size.width)
    {
        imgView.center = CGPointMake(rec2.size.width/2.0f, imgView.center.y);
    }
}
#pragma mark delegate
- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
    return imgView;
}

- (void)scrollViewDidZoom:(UIScrollView *)scrollView
{
    [self performSelector:@selector(setImageViewShowInCenter)];
}


@end
