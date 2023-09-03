//
//  main.m
//  Editor
//
//  Created by Michael Martz on 8/27/23.
//

#import <Cocoa/Cocoa.h>

#include <iostream>
#include <Enterprise/Add.h>

int main(int argc, const char * argv[]) {
    
    std::cout << Add(1, 2) << std::endl;
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    return NSApplicationMain(argc, argv);
}
