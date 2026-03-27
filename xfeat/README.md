# 配准算法  xfeat C++

首先需要转为对应尺寸的模型，执行模型转onnx项目(accelerated_features)的export.py
	---在export的parse_args函数中设置args.height width来设置导出尺寸(32的倍数)，选args.xfeat_only_model模式

再执行xfeatDetect2rknn.py可以将export导出的onnx转为rknn




--------------------------------------目录结构说明-------------------------------

registration.cpp是提供对外的接口。
xfeat.cpp是程序。如果rknn模型的输入尺寸变了，xfeat.h也需要改变  1280X1280分辨率下   
	int H_=1280;
    int W_=1280;
    int Hd8_=160;   // H/8
    int Wd8_=160
如果输入分辨率为640X640.对应的值除以2，以此类推，Hd8_是输出的维度大小。
整体思路为，首先加载模型，初始化的时候推理oldlane的信息并保存，以后得到martrix的时候可以直接拿到oldlane的数据。同时为了减少写入数据库次数。会把与oldlane的偏移和上次的偏移综合判断两者来决定是否更新
xfeat.h
 ----ScoredPoint是用来关于oldlaned的储点位信息
 ----last_result_set用来存储上次映射后的点的位置信息
 ----keys1用来存储oldlane的关键点信息
 ----width height用来存储实时的图像的大小
 ----int maxOffsetThresh; int minOffsetThresh用来设置与oldlane偏移的范围，不在范围内说明有问题。
 ----databaseoffset用来设置与上次的偏移的阈值
 void  run(const cv::Mat image, std::vector<cv::KeyPoint> &keys, cv::Mat &descs, int maxCorners);用来对实时的图像进行推理
 int  destroy();用来销毁模型。
 void FisrtInfer(const cv::Mat oldimage); 在模型初始化的时候会调用，用来生成关于模型推理oldlane的相关信息，后续用来得到单应矩阵
 int getLaneLine(nlohmann::json lanedata1); 对车道线的json数据进行解析。
 float getOffset();float getLastOffset();得到当前的偏移和上次的偏移（相对原图。传入的实时的图像）
 int isUpate();判断本次是否要更新数据库
 void Match(const cv::Mat &descs1, const cv::Mat &descs2,
               std::vector<cv::DMatch> &matches, float minScore);
    void  RejectBadMatchesF(std::vector<cv::Point2f> &pts1, std::vector<cv::Point2f> &pts2,
                            std::vector<cv::DMatch> &matches, float thresh,int maxiter); 上面两个函数用来得到单应矩阵。
		

void updateLastMember();用本次的来更新last_result_set；

vector<float> getTransPoint(float ,float);
vector<double> getH();这两个参考配准的文档