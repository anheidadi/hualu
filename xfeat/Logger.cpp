/*#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

const std::string LOG_FILE = "interface.log";

std::string getCurrentDateTime() {
    auto t = std::time(nullptr);
    tm* now = std::localtime(&t);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", now);
    return std::string(buffer);
}

void logMessage(const std::string& message) {
    std::ofstream outFile(LOG_FILE, std::ios_base::app);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
        return;
    }
    outFile << "[" << getCurrentDateTime() << "] " << message << std::endl;
}*/

//另一个程序
#include "Logger.h"
#include <ctime>
#include <iostream>
#include<cstdio>
using  namespace std;

std::string getCurrentDateTime() {
    auto t = std::time(nullptr);
    tm* now = std::localtime(&t);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y_%m_%d_%H_%M_%S", now);
    return std::string(buffer);
}
bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}
//bool fileExists(const std::string& path) {
//    FILE* file = fopen(path.c_str(), "r");
//    if (file != nullptr) {
//        fclose(file);
//        return true;
//    }
//    return false;
//}

void createFileWithPermissions(const std::string& filename) {
    int fd = open(filename.c_str(),   O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd != -1) {
        close(fd);
        std::cout << "File created successfully with full permissions." << std::endl;
    } else {
        std::cout << "error file create" << std::endl;
    }
}
Logger::Logger(const std::string& filename) : logFileName(filename), logFile(filename, std::ios::app) {
    if (!fileExists(filename)){

        createFileWithPermissions(filename);
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(Level level, const std::string& message) {
    std::string logMessage = formatLogMessage(level, message);
//    outputToConsole(logMessage);
    outputToFile(logMessage);
}

std::string Logger::getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

std::string Logger::getLevelString(Level level) {
    switch (level) {
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::formatLogMessage(Level level, const std::string& message) {
    return getCurrentTime() + " [" + getLevelString(level) + "] " + message;
}

void Logger::outputToConsole(const std::string& message) {
    std::cout << message << std::endl;
}

void Logger::outputToFile(const std::string& message) {
    if (!logFile.is_open()) {
        logFile.open(logFileName, std::ios::app);
    }

    if (logFile.is_open()) {
        logFile << message << std::endl;
    } else {
        std::cerr << "Failed to open log file: " << logFileName << std::endl;
    }
}

vector<string> findLogFiles(string imgDirPath)
{
    vector<string> vimgPath;
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(imgDirPath.c_str())))
    {
        cout<<"Folder doesn't Exist!"<<endl;
        return vimgPath;
    }

    while((ptr = readdir(pDir))!=0)
    {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
        {
            vimgPath.push_back( ptr->d_name);
        }
    }


    closedir(pDir);
    return vimgPath;
}

string readyInit(){
    vector<string>exitsLogName= findLogFiles("laneRegistration/Rslogs");
    string logName;
    if (exitsLogName.empty()){
        string current_time=getCurrentDateTime();
        logName="laneRegistration/Rslogs/lane_"+current_time+".log";
    }
    else{

        logName="laneRegistration/Rslogs/"+exitsLogName[0];
        std::tm date = {};
        extractDateFromFilename(logName, date);
        std::time_t currentTime = std::time(nullptr);

        // 将时间戳转换为本地时间的tm结构体

        std::tm localTime = *std::localtime(&currentTime);
        int differenceInSeconds=daysBetween(date,localTime);
        int diffdays = static_cast<int>(differenceInSeconds / (60 * 60 * 24));
        if (diffdays>=2){

            remove(logName.c_str());
            string current_time=getCurrentDateTime();
            logName="laneRegistration/Rslogs/lane_"+current_time+".log";
        }


    }

    return logName;

}

bool extractDateFromFilename(const std::string& filename, std::tm& date) {


    // Extract the date part from the filename
    std::istringstream ss(filename.substr(5, 14));
    char delim;
    ss >> std::get_time(&date, "%Y_%m_%d_%H_%M_%S");

    return true;
}
int daysBetween(const std::tm& start, const std::tm& end) {
    // 将 tm 结构体转换为 time_t 类型的时间戳
    std::time_t startTime = std::mktime(const_cast<std::tm*>(&start));
    std::time_t endTime = std::mktime(const_cast<std::tm*>(&end));

    // 计算时间差（以秒为单位）
    double differenceInSeconds = std::difftime(endTime, startTime);

    // 将时间差转换为天数
    int differenceInDays = static_cast<int>(differenceInSeconds / (60 * 60 * 24));

    return differenceInDays;
}