//
//  DLNAThumbImg.m
//  GenieiPhoneiPod
//
//  Created by cs Siteview on 12-2-9.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import "DLNAThumbImg.h"
#import "DLNADelegation.h"


@implementation DLNAThumbImg
@synthesize isNeedShow;
@synthesize isFinished;
@synthesize thumbImgData;
@synthesize type;
- (id) init
{
    self = [super init];
    if (self)
    {
        thumbImgData = [[NSMutableData alloc] init];
        isFinished = NO;
        isNeedShow = YES;
    }
    return self;
}
- (void) setType:(DLNAMediaObjType)mediaType
{
    type = mediaType;
}

- (void) dealloc
{
    [self cleanThread];
    [thumbImgData release];
    [super dealloc];
}
- (void) getThumbImgWithUrl:(NSString*)url
{
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        
        NSData * data = [NSURLConnection sendSynchronousRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:url]] 
                                              returningResponse:nil
                                                          error:nil];
        [thumbImgData appendData:data];
        isFinished = YES;
        dispatch_async(dispatch_get_main_queue(), ^{
            DLNADelegation::GetInstance()->onThumbImgLoadingFinished(self);
        });
    });
}
- (void) cleanThread//clean thread
{
    
}

@end
