//
//  main.m
//  friendSmasher
//
//  Created by Ali Parr on 17/07/2012.
//  Copyright (c) 2012 Facebook. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"

int main(int argc, char *argv[])
{
    @autoreleasepool {
#ifdef APPORTABLE
        [[UIScreen mainScreen] setCurrentMode: [UIScreenMode emulatedMode:UIScreenIPhone3GEmulationMode]];
#endif
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
