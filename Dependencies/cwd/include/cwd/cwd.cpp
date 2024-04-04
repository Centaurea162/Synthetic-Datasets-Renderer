#ifndef CWD_CPP
#define CWD_CPP

#include <io.h>
#include <string>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;


std::string cwd()
{
    char buff[250];
    _getcwd(buff, 250);
    std::string current_working_directory(buff);
    current_working_directory = current_working_directory.substr(0,current_working_directory.rfind("\\"));
    return current_working_directory;
}

std::string exewd()
{
    char buff[250];
    _getcwd(buff, 250);
    std::string current_working_directory(buff);
    return current_working_directory;
}

//遍历该目录下的所有文件(夹)
std::vector<std::string> dir (const std::string& path) {
    std::vector<std::string> res;
    for (const auto& entry : fs::directory_iterator(path)) {
        res.push_back(entry.path().filename().string());
    }
    return res;
}

#endif


