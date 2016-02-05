//
//  main.m
//  iTunesMeta
//
//  Created by BlueCocoa on 16/2/5.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#import <opencv2/opencv.hpp>
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <iTunesLibrary/iTunesLibrary.h>
#import <iostream>
#import <vector>
#import "mashiro.h"

using namespace cv;
using namespace std;

@interface NSImage (OpenCV)
@property (nonatomic, readonly) Mat Mat;
@end

@implementation NSImage (OpenCV)

- (NSImage *)imageResizeTo:(NSSize)newSize {
    NSImage *sourceImage = self;
    [sourceImage setScalesWhenResized:YES];
    
    // Report an error if the source isn't a valid image
    if (![sourceImage isValid]){
        NSLog(@"Invalid Image");
    } else {
        NSImage *smallImage = [[NSImage alloc] initWithSize: newSize];
        [smallImage lockFocus];
        [sourceImage setSize: newSize];
        [[NSGraphicsContext currentContext] setImageInterpolation:NSImageInterpolationHigh];
        [sourceImage drawAtPoint:NSZeroPoint fromRect:CGRectMake(0, 0, newSize.width, newSize.height) operation:NSCompositeCopy fraction:1.0];
        [smallImage unlockFocus];
        return smallImage;
    }
    return nil;
}

- (CGImageRef)CGImage {
    CGContextRef bitmapCtx = CGBitmapContextCreate(NULL, [self size].width, [self size].height,
                                                   8 /*bitsPerComponent*/,
                                                   0 /*bytesPerRow */,
                                                   [[NSColorSpace genericRGBColorSpace] CGColorSpace],
                                                   kCGBitmapByteOrder32Host | kCGImageAlphaNoneSkipFirst);
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:bitmapCtx flipped:NO]];
    [self drawInRect:NSMakeRect(0,0, [self size].width, [self size].height) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    [NSGraphicsContext restoreGraphicsState];
    
    CGImageRef cgImage = CGBitmapContextCreateImage(bitmapCtx);
    CGContextRelease(bitmapCtx);
    
    return cgImage;
}

- (Mat)Mat {
    CGImageRef imageRef = [self CGImage];
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(imageRef);
    CGFloat cols = self.size.width;
    CGFloat rows = self.size.height;
    Mat cvMat(rows, cols, CV_8UC4); // 8 bits per component, 3 channels
    
    CGContextRef contextRef = CGBitmapContextCreate(cvMat.data,                 // Pointer to backing data
                                                    cols,                      // Width of bitmap
                                                    rows,                     // Height of bitmap
                                                    8,                          // Bits per component
                                                    cvMat.step[0],              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    kCGImageAlphaNoneSkipFirst |
                                                    kCGBitmapByteOrder32Host); // Bitmap info flags
    
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), imageRef);
    CGContextRelease(contextRef);
    CGImageRelease(imageRef);
    return cvMat;
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        ITLibrary * library = [ITLibrary libraryWithAPIVersion:@"1.0" error:nil];
        if (library) {
            int classes = MAX(atoi(argv[1]), 2);
            constexpr int linePadding = 15;
            constexpr int size = 300;
            
            __block NSMutableDictionary<NSString *, ITLibArtwork *> * albums = [[NSMutableDictionary alloc] init];
            [[library allMediaItems] enumerateObjectsUsingBlock:^(ITLibMediaItem * item, NSUInteger idx, BOOL *stop) {
                switch ([item mediaKind]) {
                    case ITLibMediaItemMediaKindSong: {
                        if ([[item album] title].length > 0 && [[item artwork] imageDataFormat] != ITLibArtworkFormatNone && [[[item artwork] image] size].width > 0) {
                            [albums setValue:[item artwork] forKey:[[item album] title]];
                        }
                        break;
                    }
                    default:
                        break;
                }
            }];
            
            NSMutableArray<NSMutableArray *> * typedArtworks = [[NSMutableArray alloc] init];
            
            for (int i = 0; i < classes; i++) {
                [typedArtworks addObject:[NSMutableArray new]];
            }
            
            [albums enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull title, ITLibArtwork * _Nonnull artwork, BOOL * _Nonnull stop) {
                Mat image = [[artwork image] Mat];
                mashiro mashiro(image);
                mashiro.color(1, [&](Mat& image, Cluster colors){
                    MashiroColor color = MashiroColor::RGB2HSV(colors.back());
                    int index = floor(color[0] / (360.0 / classes));
                    NSMutableArray * type = typedArtworks[index];
                    NSImage * cover = [artwork image];
                    NSBitmapImageRep * imageRep = [NSBitmapImageRep imageRepWithData:[cover TIFFRepresentation]];
                    [imageRep setSize:[cover size]];
                    cover = [[[NSImage alloc] initWithData:[imageRep representationUsingType:NSPNGFileType properties:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.0] forKey:NSImageCompressionFactor]]] imageResizeTo:NSMakeSize(size, size)];
                    [type addObject:cover];
                    
                    cout<<[title cStringUsingEncoding:NSUTF8StringEncoding]<<" ("<<color[0]<<", "<<color[1]<<", "<<color[2]<<") index: "<<index<<endl;
                }, CV_RGBA2RGB);
            }];
            
            int longest = -1, length;
            for (int i = 0; i < classes; i++) {
                length = (int)typedArtworks[i].count;
                if (longest < length) {
                    longest = length;
                }
            }
            
            Mat statistics = Mat::ones(cv::Size((size + (linePadding << 1)) * classes, size + linePadding + longest * (size + (linePadding << 1))), CV_8UC3);
            rectangle(statistics, cv::Point(0, 0), cv::Point((size + (linePadding << 1)) * classes, size + linePadding + longest * (size + (linePadding << 1))), Scalar(255, 255, 255), CV_FILLED);
            
            cv::Point tl = cv::Point(linePadding, linePadding);
            cv::Point br = cv::Point(linePadding + size, linePadding + size);
            for (int i = 0; i < classes; i++) {
                if (i) {
                    tl.x += (linePadding << 1) + size;
                    br.x += (linePadding << 1) + size;
                }
                MashiroColor rgb(MashiroColor::HSV2RGB(MashiroColor((360.0 / classes) * i, 1, 1)));
                cout<<"("<<rgb[0]<<", "<<rgb[1]<<", "<<rgb[2]<<")"<<endl;
                rectangle(statistics, tl, br, Scalar(rgb[2] * 255, rgb[1] * 255, rgb[0] * 255), CV_FILLED);
                
                int numbered = 1;
                for (NSImage * artwork in typedArtworks[i]) {
                    Mat small = [artwork Mat];
                    cvtColor(small, small, CV_RGBA2RGB);
                    Mat ROI = statistics(cv::Rect(tl.x, tl.y + numbered * (linePadding * 2 + size), size, size));
                    small.copyTo(ROI);
                    numbered++;
                }
            }
            imwrite("iTunesStatictics.jpg", statistics);
        }
    }
    return 0;
}
