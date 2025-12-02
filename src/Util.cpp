#include "Util.h"

#include <chrono>
#include <filesystem>
#include <thread>


bool Util::WaitForStableFile(const std::string& path, const int retries) {
    namespace fs = std::filesystem;
    uintmax_t lastSize = 0;

    if (!fs::exists(path))
        return false;

    for (int i = 0; i < retries; i++) {
        const auto curSize = fs::file_size(path);
        if (curSize == lastSize && curSize != 0)
            return true;

        lastSize = curSize;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return false;
}

#ifdef __linux__
std::string Util::OpenExplorer(const std::string &filter, const std::string &title) {
    const std::string cmd = ("zenity --file-selection --file-filter='" + filter + "'  --title='" + title + "'");

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        printf("Error: Failed to open pipe for Zenity.\n");
        return "";
    }

    char buf[256];
    std::string result;

    while (fgets(buf, sizeof(buf), pipe))
        result += buf;

    pclose(pipe);

    if (!result.empty() && result.back() == '\n')
        result.pop_back();

    return result;
}
#endif
#ifdef _WIN32
#include <windows.h>

std::string Util::OpenExplorer(const std::string &filter, const std::string &title)
{
    char filename[MAX_PATH] = {0};

    // Convert `filter` into Win32 format:  "txt files (*.txt)\0*.txt\0\0"
    std::string formattedFilter;
    {
        // Convert: "txt" → "*.txt"
        formattedFilter = filter;
        if (!filter.empty() && filter[0] != '*')
            formattedFilter = "*." + filter;

        formattedFilter += '\0';
        formattedFilter += '\0';
    }

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = formattedFilter.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = title.c_str();

    if (GetOpenFileNameA(&ofn))
        return filename;

    return "";
}
#endif
#ifdef __APPLE__
#include <cstdio>
#include <string>

std::string Util::OpenExplorer(const std::string &filter, const std::string &title)
{
    std::string cmd =
        "osascript -e '"
        "try "
        "set f to POSIX path of (choose file with prompt \"" + title + "\") "
        "on error "
        "return \"\" "
        "end try'";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return "";

    char buf[256];
    std::string result;

    while (fgets(buf, sizeof(buf), pipe))
        result += buf;

    pclose(pipe);

    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();

    return result;
}
#endif
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
std::string Util::OpenExplorer(const std::string&, const std::string&)
{
    return "";
}
#endif