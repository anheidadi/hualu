//
// Created by 87293 on 2025/4/17.
//

#include "XFeat.h"
using json = nlohmann::json;
using namespace  std;
string logname=readyInit();
Logger logger(logname);
std::string generateImageFilename() {
    // 获取当前时间
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now); // 转换为本地时间

    // 缓冲区存储格式化后的时间字符串
    char buffer[80];
    // 格式化时间字符串（例如：20250826_091804）
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", timeinfo);

    // 添加文件扩展名（例如 .png）
    return "data/testpics/"+std::string(buffer) + ".jpg";
}
json readJsonFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    json jsonData;
    file >> jsonData;
    return jsonData;
}
class ConfigParser {
public:
    void load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open config file");
        }

        std::string line;
        while (std::getline(file, line)) {
            parseLine(line);
        }
    }

    std::string getString(const std::string& section, const std::string& key) const {

        auto it = data.find(section + "." + key);
        if (it != data.end()) {
            return it->second;
        }
        throw std::runtime_error("Key not found in config");
    }

private:
    std::map<std::string, std::string> data;

    void parseLine(const std::string& line) {
        std::istringstream iss(line);
        std::string token;
        if (std::getline(iss, token, '=')) {
            trim(token);
            if (!token.empty() && token.front() != ';' && token.front() != '#') { // Ignore comments and empty lines
                std::string value;
                std::getline(iss, value);
                trim(value);

                if (currentSection.empty() && !token.empty() && token.front() == '[' && token.back() == ']') {
                    currentSection = token.substr(1, token.size() - 2);
                } else if (!currentSection.empty()) {
                    data[currentSection + "." + token] = value;
                }


            }
        }
//        cout << token << endl;
    }

    void trim(std::string& str) {
        str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
        str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
    }

    std::string currentSection;
};
inline float FastExp(float x)
{
    constexpr float a = (1 << 23) / 0.69314718f;
    constexpr float b = (1 << 23) * (127 - 0.043677448f);
    x = a * x + b;

    // Remove these lines if bounds checking is not needed
    constexpr float c = (1 << 23);
    constexpr float d = (1 << 23) * 255;
    if (x < c || x > d)
        x = (x < c) ? 0.0f : d;

    // With C++20 one can use std::bit_cast instead
    uint32_t n = static_cast<uint32_t>(x);
    memcpy(&x, &n, 4);
    return x;
}
inline void CalcBicubicWeights(float t, float &wm1, float &w0, float &w1, float &w2) {
    constexpr float a = -0.75f;
    float t2 = t * t;
    float t3 = t2 * t;
    wm1 = a * (t3 - 2 * t2 + t);
    w0  = (a+2) * t3 - (a+3) * t2 + 1;
    w1  = -(a+2) * t3 + (2*a+3) * t2 - a * t;
    w2  = a * (-t3 + t2);
}
void XFeat::Match(const cv::Mat &descs1, const cv::Mat &descs2,
                    std::vector<cv::DMatch> &matches, float minScore) {





    // 输出结果

    // 输出执行时间

    cv::Mat scores12 = descs1 * descs2.t();

//    cv::Mat scores21 = descs2 * descs1.t();
    cv::Mat scores21 ;
    cv::transpose(scores12,scores21);


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
void  XFeat::RejectBadMatchesF(std::vector<cv::Point2f> &pts1, std::vector<cv::Point2f> &pts2,
                                std::vector<cv::DMatch> &matches, float thresh,int maxiter) {
    assert(pts1.size()==pts2.size() && pts1.size()==matches.size());



    if ((pts1.size()<5) | (pts2.size()< 5)) {
        H_matrix={1,0,0,0,1,0,0,0,0};
        logger.log(Logger::WARNING,"关键点小于5");
        return ;
    }

    std::vector<uchar> status;
    cv::Mat image12image2(3, 3, CV_64F);
    try{
        image12image2=cv::findHomography(pts1, pts2, cv::USAC_MAGSAC, thresh,status,maxiter,0.99);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                H_matrix[i * 3 + j] = image12image2.at<double>(i, j);
            }
        }
    }catch (exception e1){
        logger.log(Logger::WARNING,"匹配不上");
        H_matrix={1,0,0,0,1,0,0,0,0};
    }

}
static void dump_tensor_attr(rknn_tensor_attr *attr)
{
    std::string shape_str = attr->n_dims < 1 ? "" : std::to_string(attr->dims[0]);
    for (int i = 1; i < attr->n_dims; ++i)
    {
        shape_str += ", " + std::to_string(attr->dims[i]);
    }

//    printf("  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, w_stride = %d, size_with_stride=%d, fmt=%s, "
//           "type=%s, qnt_type=%s, "
//           "zp=%d, scale=%f\n",
//           attr->index, attr->name, attr->n_dims, shape_str.c_str(), attr->n_elems, attr->size, attr->w_stride,
//           attr->size_with_stride, get_format_string(attr->fmt), get_type_string(attr->type),
//           get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}
static unsigned char *load_data(FILE *fp, size_t ofst, size_t sz)
{
    unsigned char *data;
    int ret;

    data = NULL;

    if (NULL == fp)
    {
        return NULL;
    }

    ret = fseek(fp, ofst, SEEK_SET);
    if (ret != 0)
    {
        printf("blob seek failure.\n");
        return NULL;
    }

    data = (unsigned char *)malloc(sz);
    if (data == NULL)
    {
        printf("buffer malloc failure.\n");
        return NULL;
    }
    ret = fread(data, 1, sz, fp);
    return data;
}
static unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp;
    unsigned char *data;

    fp = fopen(filename, "rb");
    if (NULL == fp)
    {
        perror("Error opening file");
        printf("Open file %s failed.\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);

    data = load_data(fp, 0, size);

    fclose(fp);

    *model_size = size;
    return data;
}

void XFeat::SoftmaxScore(float *score, int h, int w, int c) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            float *ptr = score + i * w * c + j * c;
            float sum = 0;
            for (int k = 0; k < c; ++k) {
                float exp = FastExp(ptr[k]);
                ptr[k] = exp;
                sum += exp;
            }
            float invSum = 1.0f / sum;
            for (int k = 0; k < c; ++k) {
                ptr[k] *= invSum;
            }
        }
    }
}
void XFeat::FlattenScore(float *src, float *dst) {
    for (int i = 0; i < Hd8_; ++i) {
        for (int j = 0; j < Wd8_; ++j) {
            float* src_ptr = src + i * Wd8_ * 65 + j * 65;
            int iRow = i * 8;
            int jCol = j * 8;
            float* dst_ptr = dst +iRow * W_ + jCol;
            for (int k = 0; k < 8; ++k) {
                for (int l = 0; l < 8; ++l) {
                    dst_ptr[k * W_ + l] = src_ptr[k * 8 + l];
                }
            }
        }
    }
}
void XFeat::Nms(const cv::Mat &scores, float scoreThresh, int kernelSize, std::vector<ScoredPoint> &points) {
    points.clear();

    int rows = scores.rows;
    int cols = scores.cols;
    int halfKernelSize = kernelSize / 2;
    cv::Mat mask = cv::Mat::ones(rows, cols, CV_8U);
    const auto * scorePtr = scores.ptr<float>();
    auto* maskPtr = mask.ptr<uchar>();

    std::vector<int> ptrOffsets;
    ptrOffsets.reserve(kernelSize * kernelSize);
    for (int i = -halfKernelSize; i <= halfKernelSize; i++) {
        for (int j = -halfKernelSize; j <= halfKernelSize; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            ptrOffsets.push_back(i * cols + j);
        }
    }

    for (int i = halfKernelSize; i < rows - halfKernelSize; i++) {
        for (int j = halfKernelSize; j < cols - halfKernelSize; j++) {
            int addr = i * cols + j;
            if (maskPtr[addr] == 0) {
                continue;
            }

            const float score = scorePtr[addr];
            if (score <= scoreThresh) {
                maskPtr[addr] = 0;
                continue;
            }

            // nms
            bool isMax = true;
            for (const auto &offset : ptrOffsets) {
                if (score < scorePtr[addr + offset]) {
                    maskPtr[addr] = 0;
                    isMax = false;
                    break;
                }
            }
            //
            if (isMax) {
                points.push_back({j, i, score});
                // mask out the neighbors
                for (const auto &offset : ptrOffsets) {
                    maskPtr[addr + offset] = 0;
                }
            }
        }
    }


}
void XFeat::InterpDescriptor(const float *descMat, float *descriptor, float ptx, float pty) {
    int x0 = cvFloor(ptx);
    int y0 = cvFloor(pty);
    int xm1 = x0 - 1;
    int ym1 = y0 - 1;
    float dx = ptx - static_cast<float>(x0);
    float dy = pty - static_cast<float>(y0);

    float wxm1, wx0, wx1, wx2;
    float wym1, wy0, wy1, wy2;
    CalcBicubicWeights(dx, wxm1, wx0, wx1, wx2);
    CalcBicubicWeights(dy, wym1, wy0, wy1, wy2);

    const float* desc_xm1_ym1 = descMat + ym1 * Wd8_ * 64 + xm1 * 64;
    const float* desc_x0_ym1 = desc_xm1_ym1 + 64;
    const float* desc_x1_ym1 = desc_x0_ym1 + 64;
    const float* desc_x2_ym1 = desc_x1_ym1 + 64;
    const float* desc_xm1_y0 = desc_xm1_ym1 + Wd8_ * 64;
    const float* desc_x0_y0 = desc_xm1_y0 + 64;
    const float* desc_x1_y0 = desc_x0_y0 + 64;
    const float* desc_x2_y0 = desc_x1_y0 + 64;
    const float* desc_xm1_y1 = desc_xm1_y0 + Wd8_ * 64;
    const float* desc_x0_y1 = desc_xm1_y1 + 64;
    const float* desc_x1_y1 = desc_x0_y1 + 64;
    const float* desc_x2_y1 = desc_x1_y1 + 64;
    const float* desc_xm1_y2 = desc_xm1_y1 + Wd8_ * 64;
    const float* desc_x0_y2 = desc_xm1_y2 + 64;
    const float* desc_x1_y2 = desc_x0_y2 + 64;
    const float* desc_x2_y2 = desc_x1_y2 + 64;

    double sum = 0;
    for (int i = 0; i < 64; ++i) {
        float v_m1 = wxm1 * desc_xm1_ym1[i] + wx0 * desc_x0_ym1[i] + wx1 * desc_x1_ym1[i] + wx2 * desc_x2_ym1[i];
        float v_0 = wxm1 * desc_xm1_y0[i] + wx0 * desc_x0_y0[i] + wx1 * desc_x1_y0[i] + wx2 * desc_x2_y0[i];
        float v_1 = wxm1 * desc_xm1_y1[i] + wx0 * desc_x0_y1[i] + wx1 * desc_x1_y1[i] + wx2 * desc_x2_y1[i];
        float v_2 = wxm1 * desc_xm1_y2[i] + wx0 * desc_x0_y2[i] + wx1 * desc_x1_y2[i] + wx2 * desc_x2_y2[i];
        float v = wym1 * v_m1 + wy0 * v_0 + wy1 * v_1 + wy2 * v_2;
        descriptor[i] = v;
        sum += v * v;
    }

    // normalize
    float invNorm = static_cast<float>(1.0 / std::max(std::sqrt(sum), 1e-12));
    for (int i = 0; i < 64; ++i) {
        descriptor[i] *= invNorm;
    }
}
XFeat::XFeat(json laneData) {

    ConfigParser parser;
    parser.load("laneRegistration/config/config.ini");
    maxCorners=std::stoi(parser.getString("Settings", "maxCorners"));
    //刘沛远修改
    modelfile ="laneRegistration/weights/xfeat1280.rknn";
    std::cout << modelfile  << std:: endl;
    thresh=std::stof(parser.getString("Settings", "thresh"));
    maxiter=std::stoi(parser.getString("Settings", "maxiter"));
    databaseoffset=std::stoi(parser.getString("Settings", "databaseoffset"));
    minScore=std::stof(parser.getString("Settings", "minScore"));
    maxOffsetThresh=std::stoi(parser.getString("Settings", "maxoffset"));
    minOffsetThresh=std::stoi(parser.getString("Settings", "minoffset"));
    model_data = load_model(modelfile, &model_data_size);
    //得到数据库名
    ConfigParser parser2;
    parser2.load("config/config.ini");
    //web.databasename:rvbsdb    应该config的解析有问题正常该sys.databasename,有时间改下
    dataBaseName=parser2.getString("web","databasename");
    //如果getlaneline返回1代表出错了
    if(getLaneLine(laneData)){

        logger.log(Logger::ERROR,"database don't hava data");
        throw runtime_error("laneline data don't exist");
    }

    ret = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
    if (ret < 0)
    {
        printf("rknn_init error ret=%d\n", ret);
        return ;
    }
    else{
        printf("rknn_init success\n");
    }
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0)
    {
        printf("rknn_init query error ret=%d\n", ret);


        return ;
    }
    rknn_core_mask core_mask = RKNN_NPU_CORE_2;
    int ret = rknn_set_core_mask(ctx, core_mask);
    if (ret >= 0)
    {
        printf("success set core_mask ret=%d\n", ret);



    }

//    rknn_mem_size mem_size;
//    ret = rknn_query(ctx, RKNN_QUERY_MEM_SIZE, &mem_size, sizeof(mem_size));

//    printf("总内存: %d bytes\n", mem_size.total_weight_size);
//    printf("tensor内存: %d bytes\n", mem_size.total_internal_size);
//    printf("dma内存: %ld bytes\n", mem_size.total_dma_allocated_size);
//    printf("sram内存: %d bytes\n", mem_size.total_sram_size );
//    printf("空闲sram内存: %d bytes\n", mem_size.free_sram_size );


    rknn_tensor_attr input_attrs[io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++)
    {
        input_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret < 0)
        {
            printf("rknn_init error ret=%d\n", ret);
            return ;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }
//    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++)
    {
        output_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        dump_tensor_attr(&(output_attrs[i]));
/*        printf("Output Tensor %d:\n", output_attrs[i].index);
        printf("  Name: %s\n", output_attrs[i].name);
        printf("  Element Count: %d\n", output_attrs[i].n_elems);
        printf("  Data Size: %d bytes\n", output_attrs[i].size);
        printf("  Data Type: %d\n", output_attrs[i].type); // 数据类型
        printf("  Quantization Type: %d\n", output_attrs[i].qnt_type); // 量化类型*/

    }
    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
    {
        printf("model is NCHW input fmt\n");
        channel = input_attrs[0].dims[1];
        height = input_attrs[0].dims[2];
        width = input_attrs[0].dims[3];
    }
    else
    {
        printf("model is NHWC input fmt\n");
        height = input_attrs[0].dims[1];
        width = input_attrs[0].dims[2];
        channel = input_attrs[0].dims[3];
    }
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = width * height * channel;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].pass_through = 0;
}
XFeat::~XFeat(){
    if (model_data) {
        free(model_data);  // 释放模型数据
        model_data = nullptr;
        std::cout << "release model_Data success" << std::endl;
    }
    ret = rknn_destroy(ctx);
    if(ret==0){
        printf("release model success");
    }else{
        printf("release model failed");
    }
}
void XFeat::run(const cv::Mat image, std::vector<cv::KeyPoint> &keys, cv::Mat &descs, int maxCorners) {
    if(image.empty()){
        logger.log(Logger::ERROR,"oldlane数据不存在");
        return;
    }

    std::string filename = generateImageFilename();
    cv::imwrite(filename,image);
    cv::Mat imgs;
    ori_h=image.rows;
    ori_h=image.rows;
    ori_w=image.cols;
    //刘沛远修改
    width_scale_=float(ori_w)/1280.0;
    height_scale_=float(ori_h)/1280.0;
    cv::resize(image, imgs, cv::Size(1280, 1280),0,0, cv::INTER_LINEAR);
    const int roiX = (imgs.cols - W_) / 2;
    const int roiY = (imgs.rows - H_) / 2;
    // convert image to tensor
    /*cv::Mat fimg;
    if (image.rows == H_ && image.cols == W_) {
        image.convertTo(fimg, CV_32F, 1.0/255.0);
    } else {
        cv::Rect roi(roiX, roiY, W_, H_);
        cv::Mat roiImg = image(roi);
        roiImg.convertTo(fimg, CV_32F, 1.0/255.0);
    }*/
    inputs[0].buf = imgs.data;
    ret=rknn_inputs_set(ctx, io_num.n_input, inputs);
    rknn_output outputs[io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < io_num.n_output; i++)
    {
        outputs[i].index = i;
        outputs[i].want_float = 1;
    }
    ret = rknn_run(ctx, NULL);

    if(ret<0){
        logger.log(Logger::ERROR,"rknn infer error");
    }

    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
    size_t element_count = outputs[0].size / sizeof(float);
//    printf("infer result:%d\n",ret);

    //对模型输出结果kp进行处理

    auto* kptScorePtr=static_cast<float *>(outputs[1].buf);
    const int shw = Hd8_ * Wd8_;

    for (int i = 0; i < shw; ++i) {
        float sum = 0;
        for (int j = 0; j < 65; ++j) {
            sum += std::exp(kptScorePtr[j * shw + i]);
        }
        for (int j = 0; j < 65; ++j) {
            kptScorePtr[j * shw + i] = std::exp(kptScorePtr[j * shw + i]) / sum;
        }
    }

    cv::Mat scoreImg(H_, W_, CV_32F);

    for (int i = 0; i < 64; ++i) {
        const int ir = i / 8;
        const int ic = i % 8;
        const int iShw = i * shw;

        for (int k = 0; k < Hd8_; ++k) {
            const int row = k * 8 + ir;
            const int kWd8 = k * Wd8_;
            for (int j = 0; j < Wd8_; ++j) {
                int col = j * 8 + ic;
                scoreImg.at<float>(row, col) = kptScorePtr[iShw + kWd8 + j];
            }
        }
    }

    /*auto *scoreImgPtr = scoreImg.ptr<float>();
    FlattenScore(kptScorePtr, scoreImgPtr);*/
    std::vector<ScoredPoint> scoredPoints_;
    Nms(scoreImg, 0.05f, nmsKernelSize_, scoredPoints_);

    //对数据H1进行处理
    auto * heatMapPtr=static_cast<float *>(outputs[2].buf);
    cv::Mat heatMapSmall(Hd8_, Wd8_, CV_32F, heatMapPtr);
    // resize it to [H, W]
    cv::Mat heaMapFull;
    cv::resize(heatMapSmall, heaMapFull, cv::Size(W_, H_));

    for (auto &pt : scoredPoints_) {
        pt.score *= heaMapFull.at<float>(pt.y, pt.x);
    }

    std::sort(scoredPoints_.begin(), scoredPoints_.end(), [](const ScoredPoint &a, const ScoredPoint &b) {
        return a.score > b.score;
    });


    if (scoredPoints_.size() > maxCorners) {
        scoredPoints_.resize(maxCorners);
    }

    keys.clear();
    for (const auto &pt : scoredPoints_) {
        keys.emplace_back(pt.x, pt.y, 0);
    }
    //对输出数据M进行处理
    int lpy_cc=0;
    auto *descTensorPtr = static_cast<float *>(outputs[0].buf);
    int element_count_by_dims=outputs[0].size;



    for (int i = 0; i < shw; ++i) {
        double sum = 0;
        for (int j = 0; j < 64; ++j) {
            sum += descTensorPtr[j * shw + i] * descTensorPtr[j * shw + i];
        }
        float invNorm = static_cast<float>(1.0 / std::max(std::sqrt(sum), 1e-12));
        for (int j = 0; j < 64; ++j) {
            descTensorPtr[j * shw + i] *= invNorm;
            lpy_cc=j * shw + i;
        }
    }



    descs = cv::Mat::zeros((int)keys.size(), 64, CV_32F);

    float wxm1, wx0, wx1, wx2;
    float wym1, wy0, wy1, wy2;
    const float width_scale = float(Wd8_) / float(W_ - 1);
    const float height_scale = float(Hd8_) / float(H_ - 1);


    for (int n = 0; n < (int)(keys.size()); ++n) {
        const auto &pt = keys[n];
        // align_corner = False
        float x = pt.pt.x * width_scale - 0.5f;
        float y = pt.pt.y * height_scale - 0.5f;
//        align_corner = True
//        float x = (pt.pt.x / 639.f * 79.f);
//        float y = (pt.pt.y / 639.f * 79.f);

        // interpolate and normalize the descriptor



        int x0 = cvFloor(x);
        int y0 = cvFloor(y);
        int xm1 = x0 - 1;
        int ym1 = y0 - 1;
        int x1 = x0 + 1;
        int y1 = y0 + 1;
        int x2 = x0 + 2;
        int y2 = y0 + 2;
        float dx = x - static_cast<float>(x0);
        float dy = y - static_cast<float>(y0);

        CalcBicubicWeights(dx, wxm1, wx0, wx1, wx2);
        CalcBicubicWeights(dy, wym1, wy0, wy1, wy2);

        auto* desc_n_ptr = descs.ptr<float>(n);
        double sum = 0;

        for (int i = 0; i < 64; ++i) {
            int iShw = i * shw;
            int idx_m1 = iShw + ym1 * Wd8_ + xm1;
            float v_m1,v0,v1,v2;
            if(idx_m1>=0 && idx_m1<=1638396){
//                std::cout << "idx:" <<idx_m1<< "::" <<ym1 << ":"<<xm1 <<"::"<<shw <<std::endl;
                v_m1 = wxm1 * descTensorPtr[idx_m1] + wx0 * descTensorPtr[idx_m1 + 1] + wx1 * descTensorPtr[idx_m1 + 2] + wx2 * descTensorPtr[idx_m1 + 3];
                //v_m1 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001 + wx2 * 0.001;
            }
            else{
                v_m1 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001 + wx2 * 0.001;

            }

            int idx0 = iShw + y0 * Wd8_ + xm1;
            if(idx0>=0 && idx0<=1638396){
                v0 = wxm1 * descTensorPtr[idx0] + wx0 * descTensorPtr[idx0 + 1] + wx1 * descTensorPtr[idx0 + 2] + wx2 * descTensorPtr[idx0 + 3];
                //v0 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001 + wx2 * 0.001;
            }else{
                v0 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001 + wx2 * 0.001;

            }
            int idx1 = iShw + y1 * Wd8_ + xm1;
            if(idx1>=0 && idx1<=1638396){
                v1 = wxm1 * descTensorPtr[idx1] + wx0 * descTensorPtr[idx1 + 1] + wx1 * descTensorPtr[idx1 + 2] + wx2 * descTensorPtr[idx1 + 3];
                //v1 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001+ wx2 * 0.001;
            }
            else{
                v1 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001+ wx2 * 0.001;

            }
            int idx2 = iShw + y2 * Wd8_ + xm1;


//            if(idx2>=lpy_cc){
//                logger.log(Logger::INFO,"错误:"+std::to_string(idx2)+"::"+
//                to_string(iShw )+"::"+ to_string(y1)
//                                       +"::"+ to_string(xm1)
//                                       +"::"+ to_string(ym1)
//                                       +"::"+ to_string(i)
//                                       +"::"+ to_string(descTensorPtr[idx2])
//                                       +"::"+ to_string(descTensorPtr[1638406])
//                                       +"::"+ to_string(descTensorPtr[1638399])
//
//                );
//            }
            if(idx2 >=0 && idx2<=1638396){
                v2 = wxm1 * descTensorPtr[idx2] + wx0 * descTensorPtr[idx2 + 1] + wx1 * descTensorPtr[idx2 + 2] + wx2 * descTensorPtr[idx2 + 3];
                //v2 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001 + wx2 * 0.001;
            }else{
                v2 = wxm1 * 0.001 + wx0 * 0.001 + wx1 * 0.001 + wx2 * 0.001;

            }

            float v = wym1 * v_m1 + wy0 * v0 + wy1 * v1 + wy2 * v2;
            desc_n_ptr[i] = v;
            sum += v * v;




        }



    // normalize


        float invNorm = static_cast<float>(1.0 / std::max(std::sqrt(sum), 1e-12));
        for (int i = 0; i < 64; ++i) {
            desc_n_ptr[i] *= invNorm;
        }





    }

// add the edge
    for (auto &key : keys) {
        key.pt.x += static_cast<float>(roiX);
        key.pt.y += static_cast<float>(roiY);
    }

    rknn_outputs_release(ctx, io_num.n_output, outputs);



}
void XFeat::FisrtInfer(const cv::Mat oldimage){
//    cv::Mat oldimage = cv::imread("../../data/lanepic/oldlane.jpg", cv::IMREAD_UNCHANGED);
    run(oldimage,keys1,descs1,maxCorners);

};
bool XFeat::Infer(const cv::Mat image) {
    if (image.empty()){
        logger.log(Logger::ERROR,"图像数据不存在");
        isupdate= false;
        return false;
    }
    cv::Mat descs2;
    std::vector<cv::KeyPoint> keys2;
    run(image,keys2,descs2,maxCorners);

    std::vector<cv::DMatch> matches;

    Match(descs1,descs2,matches, minScore);

    std::vector<cv::Point2f> pts1, pts2;

    for (auto& m : matches) {
        pts1.push_back(keys1[m.queryIdx].pt);
        pts2.push_back(keys2[m.trainIdx].pt);
    }
    RejectBadMatchesF(pts1, pts2, matches, thresh,maxiter);

    isUpate();
    if (isupdate){
        logger.log(Logger::INFO, "更新");
    } else{
        logger.log(Logger::INFO, "未更新");
    }
    logger.log(Logger::INFO, "成功执行");
    return isupdate;

}
//0 is normal
int XFeat::getLaneLine(json lanedata){
//    std::cout << "开始解析json"  << std:: endl;
//    std::cout << lanedata["data"] << std:: endl;
    for(auto line:lanedata["data"]){
//        std::cout << line  << std:: endl;
        for (auto poi:line["point"]){

            result_set.push_back({
                to_string(line["line"]),
                to_string(poi["point_id"]),
                to_string(poi["point_x"]),
                to_string(poi["point_y"])
            });

    }

    }
    for(const auto tmp:result_set) {
        last_result_set.push_back({stof(tmp[0]), stof(tmp[1]), stof(tmp[2]), stof(tmp[3])});
    }
    return 0;

}
float XFeat::getOffset() {

    float maxOffset=0;

    for(const auto i : result_set){

        double trans_x= stof(i[2])*H_matrix[0]*scaleSize+ stof(i[3])*H_matrix[1]*scaleSize+H_matrix[2];
        double trans_y= stof(i[2])*H_matrix[3]*scaleSize+ stof(i[3])*H_matrix[4]*scaleSize+H_matrix[5];
        double currentOffset=sqrt(pow((trans_x-stof(i[2])*scaleSize)*width_scale_,2)+ pow((trans_y-stof(i[3])*scaleSize)*height_scale_,2));
//        std::cout << currentOffset  << std:: endl;
        if (currentOffset>maxOffset){
            maxOffset  = currentOffset;

        }

    }
    return maxOffset;

}
float XFeat::getLastOffset() {
    float maxOffset=0;
    int count=0;
//    std::cout << "上次偏移函数中" << std::endl;
    for(const auto i : result_set){
        //std::cout << "上次偏移数据:" <<last_result_set[count][2]<< "---------"<<last_result_set[count][3] <<std::endl;
        double trans_x= stof(i[2])*H_matrix[0]*scaleSize+ stof(i[3])*H_matrix[1]*scaleSize+H_matrix[2];
        double trans_y= stof(i[2])*H_matrix[3]*scaleSize+ stof(i[3])*H_matrix[4]*scaleSize+H_matrix[5];
//        double trans_x= stof(i[2])*H_matrix[0]*scaleSize+ stof(i[3])*H_matrix[1]*scaleSize+H_matrix[2];
//        double trans_y= stof(i[2])*H_matrix[3]*scaleSize+ stof(i[3])*H_matrix[4]*scaleSize+H_matrix[5];

//        double trans_x= i[2]*H_matrix[0]*scaleSize+ i[3]*H_matrix[1]*scaleSize+H_matrix[2];
//        double trans_y= i[2]*H_matrix[3]*scaleSize+ i[3]*H_matrix[4]*scaleSize+H_matrix[5];
        float currentOffset=sqrt(pow((trans_x-last_result_set[count][2]*scaleSize)*width_scale_,2)+
                pow((trans_y-last_result_set[count][3]*scaleSize)*height_scale_,2));
        count++;
        if (currentOffset>maxOffset){
            maxOffset  = currentOffset;

        };
    }
    return maxOffset;
}
int XFeat::isUpate(){
    isupdate=false;
    float offset=getOffset();
    float compareoffset=getLastOffset();
    if(offset >= minOffsetThresh && offset <= maxOffsetThresh){
        if(compareoffset<=databaseoffset){

            isupdate= false;
        }
        else{

            isupdate= true;
        }

    }else if(minOffsetThresh>offset){
        //出现当前偏移小，但是与上次偏移大

        if(compareoffset>databaseoffset){

            isupdate= true;
        }
    }else{
        //偏移过大，不更新
       
        logger.log(Logger::WARNING,"偏移过大");
        isupdate= false;
    }
    //更新上次last_result
    updateLastMember();
    logger.log(Logger::INFO,"与当前偏移:"+ to_string(static_cast<int>(offset * 10) / 10.0)+",与上次偏移:"+
    to_string(static_cast<int>(compareoffset * 10) / 10.0));
    return isupdate;
}
void XFeat::updateLastMember() {
//    for(const auto tmp:result_set){
//        for (auto k:tmp){
//            cout << k << endl;
//        }
//    }
    if(isupdate){
        last_result_set.clear();
        for(const auto tmp:result_set){
            float temp_x=stof(tmp[2])*H_matrix[0]+ stof(tmp[3])*H_matrix[1]+H_matrix[2]/scaleSize;
            float temp_y=stof(tmp[2])*H_matrix[3]+ stof(tmp[3])*H_matrix[4]+H_matrix[5]/scaleSize;
            last_result_set.push_back({stof(tmp[0]), stof(tmp[1]),temp_x, temp_y});
        }
    }

}
vector<double> XFeat::getH() {
    if (isupdate){
        return H_matrix;
    } else{
        return {1,0,0,0,1,0,0,0,1};
    }

}
vector<float> XFeat::getTransPoint(float x, float y ) {

    //除以2，是用来映射回320的分辨率
/*    std::cout << "函数内输出矩阵信息:"  << std:: endl;
    cout<<"[";
    int count=0;
    for (const auto& num : H_matrix) {
        if (count==0){
            cout << "[";
        }

        if (count==2){
            std::cout << num << "],";
            count=-1;
        }
        else{
            std::cout << num << ",";
        }
        count++;

    }
    std::cout << "]"  << std:: endl;
    std::cout << "函数内数据:" <<x << ":"<< y<<std::endl;*/
    printf("函数输入:%f",x);
    printf("函数输入:%f",y);
    float tran_X=x*H_matrix[0]+ y*H_matrix[1]+H_matrix[2]/scaleSize;
    float tran_Y=x*H_matrix[3]+ y*H_matrix[4]+H_matrix[5]/scaleSize;
    //std::cout << "函数内转换后数据:" <<tran_X << ":"<< tran_Y<<std::endl;
    if(tran_X<0){
        tran_X=0.0002;
//        logger.log(Logger::ERROR,"数据X越界");
    }
    else if(tran_X>320){
        tran_X=319.999911;;
//        logger.log(Logger::ERROR,"数据X越界");
    }
    if(tran_Y<0){
        tran_Y=0.0002;
//        logger.log(Logger::ERROR,"数据Y越界");
    }
    else if(tran_Y>320){
        tran_Y=319.999911;
//        logger.log(Logger::ERROR,"数据Y越界");
    }
    return {tran_X,tran_Y};

}


