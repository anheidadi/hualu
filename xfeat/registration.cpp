
#include "XFeat.h"
#include "Logger.h"
#include <vector>
#include <unistd.h>

//#include "CL/cl.h"


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
extern "C" void *create(const cv::Mat img,nlohmann::json laneData){
    XFeat *xfeat=new XFeat(laneData);

    xfeat->FisrtInfer(img);
    return (void*) xfeat;
};
extern "C" bool getUpdate(void *xfeat,const cv::Mat image){
    XFeat * model=(XFeat *)xfeat;
    return model->Infer(image);


}
//extern "C" vector<float> getNewPoint(const float x,const float y){
//    XFeat * model=(XFeat *)xfeat;
//    return model->getTransPoint(x,y);
//}
extern "C" void relase(void **model){
    XFeat * temp_model=(XFeat *)(*model);
    delete  temp_model;
    temp_model= nullptr;
    cout << "success release" << endl;
}
extern "C" vector<double>  getMatrix(const void * xfeat){
    XFeat * temp_model=(XFeat *) xfeat;
    return  temp_model->getH();
}
extern "C" vector<float> getPoint( const void * xfeat,float x,float y){
    XFeat * temp_model=(XFeat *) xfeat;
    return  temp_model->getTransPoint(x,y);
}


//int main(){
//    json j= readJsonFromFile1("../lane.json");
//    std::cout << "初步解析"  << std:: endl;
//    cv::Mat image=cv::imread("../../data/lanepic/newlane.jpg");
//    cv::Mat oldimage=cv::imread("../../data/lanepic/oldlane.jpg");
//    void* model =create(oldimage,j);
//    cout << "finish init" << endl;
//    auto start = std::chrono::high_resolution_clock::now();
//    for(int k=0;k<1;k++){
//        std::cout << k << std:: endl;
////        sleep(2);
//        getUpdate(model,image);
//    }
//
//    vector<double> H=getMatrix(model);
//    vector<float> points= getPoint(model,50.0,100.0);
//    std::cout << points[0]  << std:: endl;
//    std::cout << points[1]  << std:: endl;
//    std::cout << "输出单应矩阵信息:"  << std:: endl;
//     cout<<"[";
//     int count=0;
//     for (const auto& num : H) {
//         if (count==0){
//             cout << "[";
//         }
//
//         if (count==2){
//             std::cout << num << "],";
//             count=-1;
//         }
//         else{
//             std::cout << num << ",";
//         }
//         count++;
//
//     }
//     std::cout << "]"  << std:: endl;
//
//    relase(&model);
//    auto end = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> elapsed = end - start;
//    std::cout << "运算时间: " << elapsed.count() << " 秒\n";
//
//
//
//
//
//};
