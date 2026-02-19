#include <fstream>
#include <filesystem>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

////////////////////
// 用于获取 config 信息
///////////////////
class Config{
private:
    json data;

public:
    Config()
    {
        std::ifstream file("config.json");
        if (!file.is_open()) 
        {
            std::cerr << "无法打开文件" << std::endl;
        }
        data = json::parse(file);
    }

    const std::string getBotqq()
    {
        return data["bot"]["qq"].get<const std::string>();
    }

    const std::string getServerHost()
    {
        return data["server"]["host"].get<const std::string>();
    }

    const int getServerPort()
    {
        return data["server"]["port"].get<const int>();
    }

    const std::string getServerToken()
    {
        return data["server"]["access_token"].get<const std::string>();
    }

    const std::string getClientHost()
    {
        return data["client"]["host"].get<const std::string>();
    }

    const int getClientPort()
    {
        return data["client"]["port"].get<const int>();
    }

    const std::string getClientToken()
    {
        return data["client"]["access_token"].get<const std::string>();
    }

    const std::string getAmapKey()
    {
        return data["amap_api"]["key"].get<const std::string>();
    }

    const std::string getAmapHost()
    {
        return data["amap_api"]["host"].get<const std::string>();
    }

    const int getAmapPort()
    {
        return data["amap_api"]["port"].get<const int>();
    }

    const std::string getAmapGetPath()
    {
        return data["amap_api"]["path"].get<const std::string>();
    }

    const std::string getAIKey()
    {
        return data["ai_api"]["key"].get<const std::string>();
    }

    const std::string getAIHost()
    {
        return data["ai_api"]["host"].get<const std::string>();
    }

    const int getAIPort()
    {
        return data["ai_api"]["port"].get<const int>();
    }

    const std::string getAIPostPath()
    {
        return data["ai_api"]["path"].get<const std::string>();
    }

    const std::string getAIModel()
    {
        return data["ai_api"]["model"].get<const std::string>();
    }

    const std::string getAISysPrompts()
    {
        return data["ai_api"]["system_prompts"].get<const std::string>();
    }

    const int getAIMaxTokens()
    {
        return data["ai_api"]["max_tokens"].get<const int>();
    }

    const std::string getB23Appid()
    {
        return data["b23"]["app_id"].get<const std::string>();
    }

    const std::string getB23Host()
    {
        return data["b23"]["host"].get<const std::string>();
    }

    const int getB23Port()
    {
        return data["b23"]["port"].get<const int>();
    }

    const std::string getB23GetQueryPath()
    {
        return data["b23"]["query_path"].get<const std::string>();
    }

    const std::string getB23GetPlayPath()
    {
        return data["b23"]["play_path"].get<const std::string>();
    }

    const size_t getDownloadSizeLimit()
    {
        return data["cache"]["download_size_limit"].get<const size_t>();
    }

    const std::filesystem::path getCachePath()
    {
        return std::filesystem::path(data["cache"]["cache_path"].get<std::string>());
    }
};
