//
//  DLNAThumbImg.h
//  GenieiPhoneiPod
//
//  Created by cs Siteview on 12-2-9.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DLNAGolbal.h"

@interface DLNAThumbImg : NSObject {
    NSMutableData                   * thumbImgData;
    BOOL                            isFinished;//判断缩略图是否已经加载完成
    BOOL                            isNeedShow;//判断该缩略图是否需要显示
    DLNAMediaObjType                type;
    
}
@property (nonatomic,assign) BOOL isNeedShow; 
@property (nonatomic,readonly) BOOL isFinished;
@property (nonatomic, readonly) NSMutableData *thumbImgData;
@property (nonatomic, readonly) DLNAMediaObjType type;

- (id) init;
- (void) setType:(DLNAMediaObjType)mediaType;
- (void) getThumbImgWithUrl:(NSString*)url;
- (void) cleanThread;//clean thread

@end
