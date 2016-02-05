//
//  mashiro.cpp
//  Mashiro
//
//  Created by BlueCocoa on 16/2/2.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#include "mashiro.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

mashiro::mashiro(Mat& _image) noexcept : image(_image) { }

void mashiro::color(std::uint32_t number, MashiroColorCallback callback, int convertColor) noexcept {
    // 调整一下原始图像的大小
    Mat smallerImage;
    mashiro::resize(this->image, smallerImage, 200, 200, CV_INTER_LINEAR);
    
    if (convertColor != -1) cvtColor(smallerImage, smallerImage, convertColor);
    
    // 获取调整后的图像上每种颜色及其出现的次数
    vector<MashiroColorWithCount> pixels = mashiro::pixels(smallerImage);
    
    // 使用kmeans聚类
    Cluster clusters = this->kmeans(pixels, number);
    
    // 调用回调函数
    callback(this->image, clusters);
}

void mashiro::resize(Mat &src, Mat &dest, int width, int height, int interpolation) noexcept {
    // 如果宽或高有一个为非正数, 则返回原图像的拷贝给调整后的图像
    if (width * height <= 0) {
        dest = src.clone();
        return;
    }
    
    // 获取原图像的宽与高
    int w,h;
    w = src.cols;
    h = src.rows;
    
    // 按比例缩放
    if (width == 0) {
        double ratio = height / double(h);
        width = w * ratio;
    } else {
        double ratio = width / double(w);
        height = h * ratio;
    }
    cv::resize(src, dest, ::Size(width, height), width/w, height/h, interpolation);
}

vector<MashiroColorWithCount> mashiro::pixels(Mat &image) noexcept {
    // OpenCV里是按照BGR排列的
    constexpr int R = 2;
    constexpr int G = 1;
    constexpr int B = 0;
    
    // 统计某种颜色出现的次数
    map<MashiroColor, std::uint32_t> colorCounter;
    uint8_t r, g, b;
    
    // 直接使用C operator[]访问像素
    Vec3b * pixel;
    for(uint32_t i = 0; i < image.rows; ++i) {
        pixel = image.ptr<Vec3b>(i);
        for (uint32_t j = 0; j < image.cols; ++j) {
            r = pixel[j][R];
            g = pixel[j][G];
            b = pixel[j][B];
            colorCounter[MashiroColor(r, g, b)]++;
        }
    }
    
    // 将colorCounter里统计的结果转为vector, 便于之后使用
    vector<MashiroColorWithCount> pixels;
    for_each(colorCounter.cbegin(), colorCounter.cend(), [&pixels](const pair<MashiroColor, std::uint32_t>& pair){
        pixels.emplace_back(MashiroColorWithCount(pair.first, pair.second));
    });
    
    return pixels;
}

MashiroColor mashiro::center(const vector<MashiroColorWithCount> &colors) noexcept {
    map<double, double> vals;
    double plen = 0;
    
    // 计算中心值
    std::for_each(colors.cbegin(), colors.cend(), [&vals, &plen](const MashiroColorWithCount& colorWithCount){
        plen += colorWithCount.second;
        
        MashiroColor color = colorWithCount.first;
        for (int i = 0; i < 3; i++) {
            vals[i] += color[i] * colorWithCount.second;
        }
    });
    
    vals[0] /= plen;
    vals[1] /= plen;
    vals[2] /= plen;
    
    return MashiroColor(vals[0], vals[1], vals[2]);
}

Cluster mashiro::kmeans(const vector<MashiroColorWithCount>& pixels, std::uint32_t k, double min_diff) noexcept {
    Cluster clusters;
    
    uint32_t randmax = static_cast<uint32_t>(pixels.size());
    
    // 使用标准MersenneTwister PRNG保证取的点的随机性
    MersenneTwister mt(static_cast<uint32_t>(time(NULL)));
    
    // 取出k个点
    for (uint32_t i = 0; i < k; i++) {
        auto iter = pixels.cbegin();
        for (uint32_t t = 0; t < mt.rand() % randmax; t++, iter++);
        clusters.emplace_back(iter->first);
    }
    
    while (1) {
        ClusteredPoint points;
        
        // 与每一类的中心点比较距离, 找一个最邻近的类
        for (auto iter = pixels.cbegin(); iter != pixels.cend(); iter++) {
            MashiroColor color = iter->first;

            double smallestDistance = DBL_MAX;
            double distance;
            uint32_t smallestIndex;
            for (uint32_t i = 0; i < k; i++) {
                distance = color.euclidean(clusters[i]);
                
                if (distance < smallestDistance) {
                    smallestDistance = distance;
                    smallestIndex = i;
                }
            }
            points[smallestIndex].emplace_back(MashiroColorWithCount(color, iter->second));
        }
        
        // 重新计算每类的中心值
        double diff = 0;
        for (std::uint32_t i = 0; i < k; i++) {
            MashiroColor oldCenter = clusters[i];
            MashiroColor newCenter = mashiro::center(points[i]);
            clusters[i] = newCenter;
            diff = max(diff, oldCenter.euclidean(newCenter));
        }

        // 当差距足够小时, 停止循环
        if (diff < min_diff) {
            break;
        }
    }
    
    return clusters;
}

MashiroColor::MashiroColor(double component1, double component2, double component3) noexcept {
    this->component[0] = component1;
    this->component[1] = component2;
    this->component[2] = component3;
}

double MashiroColor::euclidean(const MashiroColor &color1, const MashiroColor &color2) noexcept {
    double distance = 0;
    
    // 欧几里得算法
    for (int i = 0; i < 3; i++) {
        distance += pow(color1[i] - color2[i], 2);
    }
    
    return sqrt(distance);
}

double MashiroColor::euclidean(const MashiroColor &color) noexcept {
    return MashiroColor::euclidean(*this, color);
}

const MashiroColor MashiroColor::RGB2HSV(const MashiroColor& color) noexcept {
    double R = color[0]/255.0, G = color[1]/255.0, B = color[2]/255.0;
    double H, S, V;
    double min, max, delta,tmp;
    
    tmp = R>G?G:R;
    min = tmp>B?B:tmp;
    tmp = R>G?R:G;
    max = tmp>B?tmp:B;
    V = (max + min) / 2;
    delta = max - min;
    
    assert(max != 0);
    assert(delta != 0);
    S = delta / max;
    
    if (R == max) {
        if (G >= B) {
            H = (G - B) / delta; // between yellow & magenta
        } else {
            H = (G - B) / delta + 6.0;
        }
    } else if( G == max ) {
        H = 2.0 + ( B - R ) / delta; // between cyan & yellow
    } else if (B == max) {
        H = 4.0 + ( R - G ) / delta; // between magenta & cyan
    }
            
    H *= 60.0; // degrees
            
    return MashiroColor(H, S, V);
}

const MashiroColor MashiroColor::HSV2RGB(const MashiroColor& color) noexcept {
    double hue = color[0], p, q, t, ff;
    long i;
    MashiroColor rgb(0.0, 0.0, 0.0);

    assert(color[1] > 0.0);

    if(hue > 360.0) hue = 0.0;
    hue /= 60.0;
    i = (long)hue;
    ff = hue - i;
    p = color[2] * (1.0 - color[1]);
    q = color[2] * (1.0 - (color[1] * ff));
    t = color[2] * (1.0 - (color[1] * (1.0 - ff)));
    
    switch(i) {
        case 0:
            rgb[0] = color[2];
            rgb[1] = t;
            rgb[2] = p;
            break;
        case 1:
            rgb[0] = q;
            rgb[1] = color[2];
            rgb[2] = p;
            break;
        case 2:
            rgb[0] = p;
            rgb[1] = color[2];
            rgb[2] = t;
            break;
        case 3:
            rgb[0] = p;
            rgb[1] = q;
            rgb[2] = color[2];
            break;
        case 4:
            rgb[0] = t;
            rgb[1] = p;
            rgb[2] = color[2];
            break;
        case 5:
        default:
            rgb[0] = color[2];
            rgb[1] = p;
            rgb[2] = q;
            break;
    }
    return rgb;
}
