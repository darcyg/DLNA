//
//  MyScrollView.h
//  ScollImage
//
//  Created by cs Siteview on 11-10-24.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface MyScrollView : UIScrollView<UIScrollViewDelegate> {
    
    UIImageView         * imgView;
}


- (void) setImage:(UIImage*) image;
- (void) setRect:(CGRect)rec;
@end
