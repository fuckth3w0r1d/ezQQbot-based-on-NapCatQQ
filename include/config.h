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
        if(!std::filesystem::exists("/home/r3t2/PPBot/config.json"))
        {
            std::cerr << "全局配置文件不存在" << std::endl;
        }
        std::ifstream file("/home/r3t2/PPBot/config.json");
        if (!file.is_open()) 
        {
            std::cerr << "无法打开配置文件" << std::endl;
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
        return data["ai"]["key"].get<const std::string>();
    }

    const std::string getAIKey2()
    {
        return data["ai2"]["key"].get<const std::string>();
    }

    const std::string getAIHost()
    {
        return data["ai"]["host"].get<const std::string>();
    }

    const std::string getAIHost2()
    {
        return data["ai2"]["host"].get<const std::string>();
    }

    const int getAIPort()
    {
        return data["ai"]["port"].get<const int>();
    }

    const std::string getAIPostPath()
    {
        return data["ai"]["path"].get<const std::string>();
    }

    const std::string getAIPostPath2()
    {
        return data["ai2"]["path"].get<const std::string>();
    }

    const std::string getAIModel()
    {
        return data["ai"]["model"].get<const std::string>();
    }

    const std::string getAIModel2()
    {
        return data["ai2"]["model"].get<const std::string>();
    }

    const std::string getAISysPrompts()
    {
        return data["ai"]["system_prompts"].get<const std::string>();
    }

    const int getAIMaxTokens()
    {
        return data["ai"]["max_tokens"].get<const int>();
    }

    const size_t getMaxChatRounds()
    {
        return data["ai"]["max_rounds"].get<const size_t>();
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
        return data["file"]["download_size_limit"].get<const size_t>();
    }

    const std::string getCachePath()
    {
        return data["file"]["cache_path"].get<const std::string>();
    }

    const std::string getDataPath()
    {
        return data["file"]["data_path"].get<const std::string>();
    }

    const std::size_t getSaveFrequency()
    {
        return data["file"]["save_frequency_seconds"].get<const size_t>();
    }

    const size_t getDownloadBufferSize()
    {
        return data["file"]["download_buffer_size"].get<const size_t>();
    }

    const size_t getCacheFileLimit()
    {
        return data["file"]["cache_file_limit"].get<const size_t>();
    }

    const std::string getRandomImgHost()
    {
        return data["random_img"]["host"].get<const std::string>();
    }

    const int getRandomImgPort()
    {
        return data["random_img"]["port"].get<const int>();
    }

    const std::string getRandomImgPath()
    {
        return data["random_img"]["path"].get<const std::string>();
    }
};
