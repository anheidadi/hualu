//
// Created by 87293 on 2025/4/17.
//

#ifndef XFEAT_XFEAT_H
#define XFEAT_XFEAT_H
#include <iostream>
#include "rknn_api.h"
#include <cstring>
#include "includelib.h"
#include <opencv2/opencv.hpp>
#include "Logger.h"

using namespace std;
extern string readyInit();
class XFeat{
public:
    explicit  XFeat(nlohmann::json laneData);
    struct ScoredPoint {
        int x;
        int y;
        float score;
    };
    //TODO：run对模型进行推理
    void  run(const cv::Mat image, std::vector<cv::KeyPoint> &keys, cv::Mat &descs, int maxCorners);
    //TODO:对模型进行释放销毁
    int  destroy();
    void SoftmaxScore(float *score, int h, int w, int c);
    void FlattenScore(float *src, float *dst);
    void Nms(const cv::Mat &scores, float scoreThresh, int kernelSize, std::vector<ScoredPoint> &points);
    void InterpDescriptor(const float *descMat, float *descriptor, float ptx, float pty);
    //初始化完成后进行第一次oldlane推理缓存
    void FisrtInfer(const cv::Mat oldimage);
    bool Infer(const cv::Mat image);
    int getLaneLine(nlohmann::json lanedata1);
    float getOffset();
    float getLastOffset();
    int isUpate();
    void Match(const cv::Mat &descs1, const cv::Mat &descs2,
               std::vector<cv::DMatch> &matches, float minScore);
    void  RejectBadMatchesF(std::vector<cv::Point2f> &pts1, std::vector<cv::Point2f> &pts2,
                            std::vector<cv::DMatch> &matches, float thresh,int maxiter);
    void updateLastMember();
    vector<float> getTransPoint(float ,float);
    vector<double> getH();
    ~XFeat();

private:
    int ret;
    unsigned char *model_data;
    rknn_context ctx;
    size_t actual_size = 0;
    rknn_input_output_num io_num;
    rknn_sdk_version version;
    rknn_input inputs[1];
    rknn_tensor_attr output_attrs[3];
    int model_data_size = 0;
    int width = 0;
    int height = 0;
    int channel = 3;

    //刘沛远修改
    int H_=1280;
    int W_=1280;
    int Hd8_=160;   // H/8
    int Wd8_=160;

    const int nmsKernelSize_ = 5;
    int maxCorners;
    const char * modelfile;
    float thresh;
    int maxiter;
    float minScore;
    std::vector<cv::KeyPoint> keys1;
    cv::Mat descs1;

    string dataBaseName;
    //sql
//    const char * host="127.0.0.1";
//    const char * user="root";
//    const char * password="123457";
//    unsigned int port =3306;
    vector<std::vector<std::string>> result_set;
    float scaleSize=4;
    float width_scale_;
    float height_scale_;
    int ori_h;
    int ori_w;
    int maxOffsetThresh;
    int minOffsetThresh;
    int databaseoffset;
    vector<double> H_matrix={1,0,0,0,1,0,0,0,0};
    vector<std::vector<float>> last_result_set;
    bool  isupdate=false;








};


#endif //XFEAT_XFEAT_H
