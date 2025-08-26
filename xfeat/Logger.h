//
// Created by 87293 on 2025/5/27.
//

#ifndef LOGGER_H
#define LOGGER_H
#include "includelib.h"
void deletelogs();
std::string getCurrentDateTime();
bool fileExists(const std::string& filename);
std::vector<std::string> findLogFiles(std::string imgDirPath);
std::string readyInit();
class Logger {
public:
    enum Level { INFO, WARNING, ERROR };

    Logger(const std::string& filename);
    ~Logger();

    void log(Level level, const std::string& message);

private:
    std::ofstream logFile;
    std::string logFileName;

    std::string getCurrentTime();
    std::string getLevelString(Level level);
    std::string formatLogMessage(Level level, const std::string& message);
    void outputToConsole(const std::string& message);
    void outputToFile(const std::string& message);
};
bool extractDateFromFilename(const std::string& filename, std::tm& date);
int daysBetween(const std::tm& start, const std::tm& end);
#endif // LOGGER_H