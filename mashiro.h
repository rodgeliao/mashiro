//
//  mashiro.hpp
//  Mashiro
//
//  Created by BlueCocoa on 16/2/2.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#ifndef MASHIRO_H
#define MASHIRO_H

#include <assert.h>
#include <cmath>
#include <float.h>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include "MersenneTwister.h"

/**
 *  @brief Forward
 */
namespace cv { class Mat; };
class mashiro;
class MashiroColor;

/**
 RGB色彩空间
 */
enum MashiroColorSpaceRGB { Red, Green, Blue };

/**
 *  @brief 颜色与其出现的次数
 */
using MashiroColorWithCount = std::pair<MashiroColor, std::uint32_t>;

/**
 *  @brief 聚类后的颜色
 */
using Cluster = std::vector<MashiroColor>;

/**
 *  @brief 某一类的颜色
 */
using ClusteredPoint = std::map<std::uint32_t, std::vector<MashiroColorWithCount>>;

/**
 *  @brief 聚类完成后的回调函数
 *
 *  @param image  分析的是哪张图
 *  @param colors 它的主要颜色
 */
using MashiroColorCallback = std::function<void(cv::Mat& image, const Cluster& colors)>;

class mashiro {
public:
    /**
     *  @brief 使用cv::Mat初始化
     */
    mashiro(cv::Mat& image) noexcept;
    
    /**
     *  @brief 开始识别主要颜色
     *
     *  @param number   需要几种主要颜色
     *  @param callback 聚类完成后的回调
     */
    void color(std::uint32_t number, MashiroColorCallback callback, int convertColor = -1) noexcept;
    
    /**
     *  @brief 快速访问std::tuple里的元素
     *
     *  @param enumerator 需要指定一个枚举类型
     *
     *  @return 对应位置上的值
     */
    template<typename E>
    static constexpr auto toType(E enumerator) noexcept {
        return static_cast<std::underlying_type_t<E>>(enumerator);
    };
    
    /**
     *  @brief 调整图片大小
     *
     *  @param src           源图片
     *  @param dest          调整后的图片
     *  @param width         宽度
     *  @param height        高度
     *  @param interpolation 方法
     */
    static void resize(cv::Mat &src, cv::Mat &dest, int width, int height, int interpolation) noexcept;
    
    /**
     *  @brief 获取一张图上所有的颜色及其出现的次数
     *
     *  @param image 源图片
     *
     *  @return 图上所有的颜色及其出现的次数
     */
    static std::vector<MashiroColorWithCount> pixels(cv::Mat &image) noexcept;
    
    
    /**
     *  @brief 给定一组带出现次数的颜色求其中心
     *
     *  @param colors 一组带出现次数的颜色
     *
     *  @return 该组颜色的中心值
     */
    static MashiroColor center(const std::vector<MashiroColorWithCount>& colors) noexcept;
private:
    /**
     *  @brief 需要处理的图像
     */
    cv::Mat& image;
    
    /**
     *  @brief kmeans聚类
     *
     *  @param pixels       图上出现的颜色及其次数
     *  @param k            聚类种数
     *  @param min_diff     偏差
     *
     *  @return 聚类后的k个颜色
     */
    Cluster kmeans(const std::vector<MashiroColorWithCount>& pixels, std::uint32_t k, double min_diff = 1.0) noexcept;
};

/**
 *  @brief 颜色
 */
class MashiroColor {
public:
    MashiroColor(double component1, double component2, double component3) noexcept;
    
    double operator [](int value) const {
        assert((0 <= value && value <= 2));
        return this->component[value];
    }
    
    double & operator [](int index) {
        assert((0 <= index && index <= 2));
        return component[index];
    }
    
    int operator < (const MashiroColor& color2) const {
        const MashiroColor color1 = * this;
        if (color1[0] < color2[0]) {
            return -1;
        } else if (color1[0] == color2[0]) {
            if (color1[1] < color2[1]) {
                return -1;
            } else if (color1[1] == color2[1]) {
                if (color1[2] < color2[2]) {
                    return -1;
                } else if (color1[2] == color2[2]) {
                    return 0;
                } else {
                    return 1;
                }
            } else {
                return 1;
            }
        } else {
            return 1;
        }
    }
    
    /**
     *  @brief 计算两个颜色的欧几里得距离
     *
     *  @discussion 仅限RGB
     *
     *  @param color1 Color 1
     *  @param color2 Color 2
     *
     *  @return 两个颜色的欧几里得距离
     */
    static double euclidean(const MashiroColor& color1, const MashiroColor& color2) noexcept;
    
    /**
     *  @brief 计算与另一个颜色的欧几里得距离
     *
     *  @discussion 仅限RGB
     *
     *  @param color1 Color 1
     *  @param color2 Color 2
     *
     *  @return 两个颜色的欧几里得距离
     */
    double euclidean(const MashiroColor& color) noexcept;
    
    /**
     *  @brief 将RGB颜色转为HSV颜色
     *
     *  @param color RGB颜色
     *
     *  @return HSV颜色
     */
    static const MashiroColor RGB2HSV(const MashiroColor& color) noexcept;
    
    /**
     *  @brief 将HSV颜色转为RGB颜色
     *
     *  @param color HSV颜色
     *
     *  @return RGB颜色
     */
    static const MashiroColor HSV2RGB(const MashiroColor& color) noexcept;
private:
    /**
     *  @brief RGB颜色分量
     */
    double component[3];
};

#endif /* MASHIRO_H */
