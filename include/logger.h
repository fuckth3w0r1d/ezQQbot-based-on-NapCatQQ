#include <iostream>
#include <string>
#include <mutex>
#include "json.hpp"

using json = nlohmann::json;

class Logger{
private:
    // ANSI 颜色代码
    static constexpr const char* COLOR_RESET = "\033[0m";
    static constexpr const char* COLOR_RED = "\033[31m";
    static constexpr const char* COLOR_GREEN = "\033[32m";
    static constexpr const char* COLOR_YELLOW = "\033[33m";
    static constexpr const char* COLOR_BLUE = "\033[34m";

    static std::string colorize(const std::string& level, const char* color) 
    {
        return std::string(color) + "[" + level + "]" + COLOR_RESET;
    }

public:
    template<typename T> static void info(const std::string& tip, const T& data) 
    {
        log(std::cout, colorize("INFO", COLOR_GREEN), tip, data);
    }

    template<typename T> static void warn(const std::string& tip, const T& data) 
    {
        log(std::cerr, colorize("WARN", COLOR_YELLOW), tip, data);
    }

    template<typename T> static void debug(const std::string& tip, const T& data) 
    {
        log(std::cerr, colorize("DEBUG", COLOR_BLUE), tip, data);
    }

    template<typename T> static void error(const std::string& tip, const T& data) 
    {
        log(std::cerr, colorize("ERROR", COLOR_RED), tip, data);
    }

private:
    template<typename T> static void log(std::ostream& os, const std::string& coloredLevel, const std::string& tip, const T& data) 
    {
        std::lock_guard<std::mutex> lock(getMutex());
        os << coloredLevel << " " << tip;
        print(os, data);
        os << std::endl;
    }

    static void print(std::ostream& os, const nlohmann::json& j) 
    {
        os << "\n" << j.dump(4);
    }

    template<typename T> static void print(std::ostream& os, const T& value) 
    {
        os << value;
    }

    static std::mutex& getMutex() 
    {
        static std::mutex mtx;
        return mtx;
    }
};