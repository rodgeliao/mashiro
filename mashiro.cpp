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

void mashiro::color(std::uint32_t number, MashiroColorCallback callback) noexcept {
    // 调整一下原始图像的大小
    Mat smallerImage;
    mashiro::resize(this->image, smallerImage, 200, 200, CV_INTER_LINEAR);
    
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

double mashiro::euclidean(const MashiroColor &color1, const MashiroColor &color2) noexcept {
    double distance = 0;
    
    // 欧几里得算法
    distance += pow((std::get<mashiro::toType(MashiroColorSpaceRGB::Red)>(color1) - std::get<mashiro::toType(MashiroColorSpaceRGB::Red)>(color2)), 2);
    distance += pow((std::get<mashiro::toType(MashiroColorSpaceRGB::Green)>(color1) - std::get<mashiro::toType(MashiroColorSpaceRGB::Green)>(color2)), 2);
    distance += pow((std::get<mashiro::toType(MashiroColorSpaceRGB::Blue)>(color1) - std::get<mashiro::toType(MashiroColorSpaceRGB::Blue)>(color2)), 2);
    
    return sqrt(distance);
}

MashiroColor mashiro::center(const vector<MashiroColorWithCount> &colors) noexcept {
    map<double, double> vals;
    double plen = 0;
    
    // 计算中心值
    std::for_each(colors.cbegin(), colors.cend(), [&vals, &plen](const MashiroColorWithCount& colorWithCount){
        plen += colorWithCount.second;
        
        MashiroColor color = colorWithCount.first;
        vals[0] += std::get<mashiro::toType(MashiroColorSpaceRGB::Red)>(color) * colorWithCount.second;
        vals[1] += std::get<mashiro::toType(MashiroColorSpaceRGB::Green)>(color) * colorWithCount.second;
        vals[2] += std::get<mashiro::toType(MashiroColorSpaceRGB::Blue)>(color) * colorWithCount.second;
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
                distance = mashiro::euclidean(color, clusters[i]);
                
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
            diff = max(diff, mashiro::euclidean(oldCenter, newCenter));
        }

        // 当差距足够小时, 停止循环
        if (diff < min_diff) {
            break;
        }
    }
    
    return clusters;
}
