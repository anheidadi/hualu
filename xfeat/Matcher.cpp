//
// Created by 87293 on 2025/5/20.
//


#include "Matcher.h"
#include <chrono>
#include <opencv2/opencv.hpp>



void Matcher::Match(const cv::Mat &descs1, const cv::Mat &descs2,
                    std::vector<cv::DMatch> &matches, float minScore) {

    auto start = std::chrono::high_resolution_clock::now();



    // 输出结果

    // 输出执行时间
    cv::Mat scores12 = descs1 * descs2.t();
//    cv::Mat scores21 = descs2 * descs1.t();
    cv::Mat scores21 ;
    cv::transpose(scores12,scores21);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "矩阵运算时间: " << elapsed.count() << " 秒\n";

    std::vector<int> match12(descs1.rows, -1);

    for (int i = 0; i < scores12.rows; i++) {
        auto *row = scores12.ptr<float>(i);
        float maxScore = row[0];
        int maxIdx = 0;
        for (int j = 1; j < scores12.cols; j++) {
            if (row[j] > maxScore) {
                maxScore = row[j];
                maxIdx = j;
            }
        }
        match12[i] = maxIdx;
    }


    std::vector<int> match21(descs2.rows, -1);
    for (int i = 0; i < scores21.rows; i++) {
        auto *row = scores21.ptr<float>(i);
        float maxScore = row[0];
        int maxIdx = 0;
        for (int j = 1; j < scores21.cols; j++) {
            if (row[j] > maxScore) {
                maxScore = row[j];
                maxIdx = j;
            }
        }
        match21[i] = maxIdx;
    }
    // cross-check
    matches.clear();
    for (int i = 0; i < descs1.rows; i++) {
        int j = match12[i];
        if (match21[j] == i && scores12.at<float>(i, j) > minScore) {
            matches.emplace_back(i, j, scores12.at<float>(i, j));
        }
    }

}

bool Matcher::RejectBadMatchesF(std::vector<cv::Point2f> &pts1, std::vector<cv::Point2f> &pts2,
                                std::vector<cv::DMatch> &matches, float thresh) {
    assert(pts1.size()==pts2.size() && pts1.size()==matches.size());
    if (pts1.size() < 8) {
        return false;
    }

    std::vector<uchar> status;
    cv::Mat image12image2=cv::findHomography(pts1, pts2, cv::USAC_MAGSAC, thresh,status,1000,0.99);
    float homographyArray[9];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            homographyArray[i * 3 + j] = image12image2.at<float>(i, j);
        }
    }
//    cv::Mat image12image2=cv::findFundamentalMat(pts1, pts2, cv::FM_RANSAC, thresh, 0.999, status);
    std::cout << "martix is:" << image12image2 << std::endl;
//    ReduceVector(matches, status);
    return true;
}