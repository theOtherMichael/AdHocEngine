#include <iostream>

#include <Enterprise/Add.h>

#import <Cocoa/Cocoa.h>

int main(int argc, const char * argv[]) {
    
    std::cout << Engine::Add(1, 2) << std::endl;
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    return NSApplicationMain(argc, argv);
}
