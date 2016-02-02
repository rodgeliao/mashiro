//
//  main.cpp
//  Mashiro
//
//  Created by BlueCocoa on 16/2/2.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#include <getopt.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include "mashiro.h"

using namespace cv;
using namespace std;

char * imageFile = NULL;
uint32_t color = 3;

static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"image", required_argument, 0, 'i'},
    {"color", optional_argument, 0, 'c'},
    {0, 0, 0, 0}
};

void print_usage();
int parse(int argc, const char * argv[]);

void print_usage() {
    printf("Usage:\n");
    printf("\t-i [image file] -c [number of color to cluster]\n");
    printf("\t-h Print this help\n");
}

int parse(int argc, const char * argv[]) {
    int c;
    int option_index = 0;
    
    while (1) {
        c = getopt_long(argc, (char * const *)argv, "hs:i:c:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'h':
                print_usage();
                return 0;
            case 'i': {
                imageFile = (char *)malloc(sizeof(char) * strlen(optarg) + 1);
                memset(imageFile, 0, sizeof(char) * strlen(optarg) + 1);
                memcpy(imageFile, optarg, sizeof(char) * strlen(optarg));
                break;
            }
            case 'c': {
                color = abs(atoi(optarg));
                break;
            }
            case '?':
                print_usage();
                return 0;
            default:
                return 0;
        }
    }
    return 1;
}

int main(int argc, const char * argv[]) {
    if (parse(argc, argv)) {
        if (imageFile && strlen(imageFile) > 0) {
            Mat image = imread(imageFile);
            assert((image.rows * image.cols) != 0);
            
            mashiro shiro(image);
            shiro.color(color, [](cv::Mat& image, Cluster colors){
                for_each(colors.cbegin(), colors.cend(), [](const MashiroColor& color){
                    cout<<"("<<std::get<mashiro::toType(MashiroColorSpaceRGB::Red)>(color)<<", "<<std::get<mashiro::toType(MashiroColorSpaceRGB::Green)>(color)<<", "<<std::get<mashiro::toType(MashiroColorSpaceRGB::Blue)>(color)<<")"<<endl;
                });
            });
        } else {
            print_usage();
        }
    } else {
        print_usage();
    }
    return 0;
}
