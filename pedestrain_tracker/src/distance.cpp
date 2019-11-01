// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "distance.hpp"
#include "logging.hpp"

#include <vector>

CosDistance::CosDistance(const cv::Size &descriptor_size)
    : descriptor_size_(descriptor_size) {
    PT_CHECK(descriptor_size.area() != 0);
}

float CosDistance::Compute(const cv::Mat &descr1, const cv::Mat &descr2) {
    PT_CHECK(!descr1.empty());
    PT_CHECK(!descr2.empty());
    PT_CHECK(descr1.size() == descriptor_size_);
    PT_CHECK(descr2.size() == descriptor_size_);

    double xy = descr1.dot(descr2);
    double xx = descr1.dot(descr1);
    double yy = descr2.dot(descr2);
    double norm = sqrt(xx * yy) + 1e-6;
    return 0.5f * static_cast<float>(1.0 - xy / norm);
}

std::vector<float> CosDistance::Compute(const std::vector<cv::Mat> &descrs1,
                                        const std::vector<cv::Mat> &descrs2) {
    PT_CHECK(descrs1.size() != 0);
    PT_CHECK(descrs1.size() == descrs2.size());

    std::vector<float> distances(descrs1.size(), 1.f);
    for (size_t i = 0; i < descrs1.size(); i++) {
        distances.at(i) = Compute(descrs1.at(i), descrs2.at(i));
    }

    return distances;
}


// 这里输入的特征图都是resize成指定大小的图片 在CreatePedestrianTracker函数中设置
float MatchTemplateDistance::Compute(const cv::Mat &descr1,
                                     const cv::Mat &descr2) {
    PT_CHECK(!descr1.empty() && !descr2.empty());
    PT_CHECK_EQ(descr1.size(), descr2.size());
    PT_CHECK_EQ(descr1.type(), descr2.type());
    cv::Mat res;
    // 模板匹配 在desc1上找descr2最相似的位置 每个坐标相似度以数值大小存放在res里
    // 当两张图片大小相同时，输出[1x1]的mat
    // （image,temp,result,method） result_size=(W-w+1)*(H-h+1)
    cv::matchTemplate(descr1, descr2, res, type_);

    PT_CHECK(res.size() == cv::Size(1,1));
    // std::cout<<"matchTemplate size:"<<res.size()<<std::endl; // [1x1]
    // std::cout<<descr1.size()<<" "<<descr2.size()<<std::endl; // [64 x 160]
    // 访问指定坐标的元素
    float dist = res.at<float>(0, 0);

    // std::cout << scale_ << " " << offset_ <<std::endl;       // -1 1
    return scale_ * dist + offset_;
}

std::vector<float> MatchTemplateDistance::Compute(const std::vector<cv::Mat> &descrs1,
                                                  const std::vector<cv::Mat> &descrs2) {
    std::vector<float> result;
    for (size_t i = 0; i < descrs1.size(); i++) {
        result.push_back(Compute(descrs1[i], descrs2[i]));
    }
    return result;
}
