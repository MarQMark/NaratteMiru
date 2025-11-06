#ifndef NARATTEMIRU_UTIL_H
#define NARATTEMIRU_UTIL_H
#include <string>


namespace  Util {
    bool WaitForStableFile(const std::string& path, int retries = 10);
    std::string OpenExplorer(const std::string& filter, const std::string& title);
};


#endif //NARATTEMIRU_UTIL_H