//
// Created by 87293 on 2025/6/13.
//
#include <iostream>
#include <dlfcn.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "includelib.h"
#include "Logger.h"
#include <vector>
using json = nlohmann::json;
using namespace std;
json readJsonFromFile1(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    json jsonData;
    file >> jsonData;
    return jsonData;
}
int main() {
    // 打开共享库
    json j= readJsonFromFile1("laneRegistration/lane.json");
//    cv::Mat image=cv::imread("data/lanepic/newlane.jpg");
    cv::Mat imageori=cv::imread("data/lanepic/oldlaneori.jpg");
//    cv::Mat image2=cv::imread("data/lanepic/output.jpg");
    if(imageori.empty()){
        std::cout << "image don't exist" << std::endl;
    }
    int ori_h=imageori.rows;
    int ori_w=imageori.cols;
    void* handler = dlopen("/home/ubuntu/rvbs/xfeat/cmake-build-debug/registration.so", RTLD_LAZY);
    if (!handler) {
        std::cerr << "Cannot open library: " << dlerror() << '\n';
        return 1;
    }else{
        std::cout << "success load so"<< std::endl;
    }
    typedef void* (*model)(const cv::Mat img,nlohmann::json laneData);
    typedef void  (*release)(void** model1);
    typedef bool (*getUpdate)(void *model,const cv::Mat image);
    typedef vector<double>  (*getMatrix)(const void * xfeat);
    typedef vector<float> (*getPoint)( const void * xfeat,float x,float y);
    model hello = (model) dlsym(handler, "create");
    release realse1=(release) dlsym(handler, "relase");
    getUpdate getupdate=(getUpdate) dlsym(handler, "getUpdate");
    getMatrix  getMatrix1=(getMatrix)dlsym(handler, "getMatrix");
    getPoint getPoint1=(getPoint)dlsym(handler, "getPoint");

    auto start1 = std::chrono::high_resolution_clock::now();
    void * model1=hello(imageori,j);
    auto end1 = std::chrono::high_resolution_clock::now();
    float aaa=319.9999;
    float  bbbb=319.9999;

    vector<float> ress=getPoint1(model1,aaa,bbbb);
    printf("输出22::%f",ress[0]);
    printf("输出22::%f",ress[1]);
    exit(0);
    // 计算持续时间（微秒）
    std::chrono::duration<double, std::milli> duration1 = end1 - start1;

    // 输出执行时间
    std::cout << "Execution time: " << duration1.count() << " microseconds" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
//    bool infer_res=getupdate(model1,image);
    float x_s=1024.0/320.0;
    float y_s=768.0/320.0;

    std::string videoPath = "149output_2025-08-25_11-23-36.mp4";
    std::vector<float>x_1{124,132,135,137,133,129};
    std::vector<float>y_1{169,153,140,129,118,107};


    cv::VideoCapture cap(videoPath);
    double fps = cap.get(cv::CAP_PROP_FPS);               // 帧率
    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));  // 原始宽度
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)); // 原始高度

    // 3. 设置输出参数（例如调整为1080P）
    int new_width = 1024;   // 新宽度（1080P）
    int new_height = 768;  // 新高度（1080P）
    // 4. 定义编解码器（MJPG 或 XVID）
    int fourcc = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');  // MJPG 编码
    cv::VideoWriter output("output1_1080p.mp4", fourcc, 10,
                           cv::Size(new_width, new_height), true);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat frame;
    bool infer_res4;
    int ssss=0;
    while (true) {
        ssss+=1;
        // 从视频中读取一帧
        cap >> frame;

        // 如果帧为空，则表示视频结束
        if (frame.empty()) {
            std::cout << "帧为none" << std::endl;
            break;
        }
        std::cout << "循环运行中" << std::endl;
        infer_res4=getupdate(model1,frame);

        vector<double> H_=getMatrix1(model1);
        for(int k=0;k<x_1.size();k++){
            std::vector<float> trans=getPoint1(model1,x_1[k],y_1[k]);
//            std::cout << "原位置:"<< x_1[k]<<":"<< y_1[k]<<
//            "新位置:"<< int(trans[0]*x_s)<<":"<< int(trans[1]*y_s) <<std::endl;
            cv::circle(frame, cv::Point(int(trans[0]*x_s), int(trans[1]*y_s)), 2, cv::Scalar(0,0,255), -1);

        }

//        std::cout << "输出单应矩阵信息:"  << std:: endl;
//        cout<<"[";
//        int count=0;
//        for (const auto& num : H_) {
//            if (count==0){
//                cout << "[";
//            }
//
//            if (count==2){
//                std::cout << num << "],";
//                count=-1;
//            }
//            else{
//                std::cout << num << ",";
//            }
//            count++;
//
//        }
//        std::cout << "]"  << std:: endl;
//        cv::imwrite("test1/"+ std::to_string(ssss)+".jpg",frame);
        std::cout << "current image: " <<ssss << std::endl;
        output.write(frame);
        if (ssss>=1000){
            break;
        }



        // 等待30毫秒（大约对应33 FPS），或者按下ESC键退出

    }
    output.release();
    cap.release();









    auto end = std::chrono::high_resolution_clock::now();

    // 计算持续时间（微秒）
    std::chrono::duration<double, std::milli> duration = end - start;

    // 输出执行时间
    std::cout << "Execution time: " << duration.count() << " microseconds" << std::endl;
//    bool infer_res1=getupdate(model1,image);

    realse1(&model1);

    std::cout << "成功"  << std:: endl;

    // 关闭共享库

    dlclose(handler);
    return 0;
}
