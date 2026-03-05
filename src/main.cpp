#include <iostream>      
#include <string>        
#include <vector>        
#include <memory>        
#include <unordered_map> 
#include <ctime>         
#include <cstddef>
#include <regex>
#include <filesystem>
#include <fstream>
#include <unistd.h>  

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"

#include "config.h"
#include "logger.h"

using json = nlohmann::json;

///////////
// config
///////////
Config cfg;
// naocatqq
const std::string BOT_QQ = cfg.getBotqq();
const std::string SERVER_HOST = cfg.getServerHost();
const int SERVER_PORT = cfg.getServerPort();
const std::string SERVER_ACCESS_TOKEN = cfg.getServerToken();
const std::string CLIENT_HOST = cfg.getClientHost();
const int CLIENT_PORT = cfg.getClientPort();
const std::string CLIENT_ACCESS_TOKEN = cfg.getClientToken();
// amap
const std::string AMAP_KEY = cfg.getAmapKey();
const std::string AMAP_CLIENT_HOST = cfg.getAmapHost();
const int AMAP_CLIENT_PORT = cfg.getAmapPort();
const std::string AMAP_GET_PATH = cfg.getAmapGetPath();
// ai chat
const std::string AI_KEY = cfg.getAIKey();
const std::string AI_CLIENT_HOST = cfg.getAIHost();
const int AI_CLIENT_PORT = cfg.getAIPort();
const std::string AI_POST_PATH = cfg.getAIPostPath();
const std::string AI_MODEL = cfg.getAIModel();
const std::string AI_DEFAULT_SYS_PROMPTS = cfg.getAISysPrompts();
const int AI_MAX_TOKENS = cfg.getAIMaxTokens();
const int MAX_CHAT_HISTORY = cfg.getMaxChatHistory();
// bilibili
const std::string B23_APP_ID = cfg.getB23Appid();
const std::string B23_CLIENT_HOST = cfg.getB23Host();
const int B23_CLIENT_PORT = cfg.getB23Port();
const std::string B23_QUERY_PATH = cfg.getB23GetQueryPath();
const std::string B23_PLAY_PATH = cfg.getB23GetPlayPath();
// 本地文件缓存
const size_t DOWNLOAD_SIZE_LIMIT = cfg.getDownloadSizeLimit();
const std::filesystem::path CACHE_PATH = cfg.getCachePath();
const size_t DOWNLOAD_BUFFER_SIZE = cfg.getDownloadBufferSize();
const size_t CACHE_FILE_LIMIT = cfg.getCacheFileLimit();
// 随机图片
const std::string IMG_CLIENT_HOST = cfg.getRandomImgHost();
const int IMG_CLIENT_PORT = cfg.getRandomImgPort();
const std::string IMG_GET_PATH = cfg.getRandomImgPath();


// 消息结构
struct ParsedMsgSegments{
    bool at_me = false;
    bool has_text = false;
    bool has_image = false;
    bool has_json = false;

    std::string text;        // 文本
    std::string image;       // image url
    json json_data;          // json
};
struct MessageContext{
    std::string group_id;
    std::string user_id;
    std::string msg_type;
    json msg_segments;
    ParsedMsgSegments pmsgsegs;
};

////////////
// 消息管理器
///////////
class MessageManager{
private:
    // 消息段解析
    static ParsedMsgSegments parseMsgSegments(const json& msgsegs)
    {
        ParsedMsgSegments result;
        for(auto& seg : msgsegs)
        {
            if(!seg.contains("type") || !seg.contains("data")) continue;
            std::string type = seg["type"].get<std::string>();
            if(type == "at")
            {
                if(seg["data"].contains("qq") && seg["data"]["qq"] == BOT_QQ) 
                    result.at_me = true;
            }
            if(type == "text")
            {
                if (seg["data"].contains("text"))
                {
                    result.has_text = true;
                    result.text += seg["data"]["text"].get<std::string>();
                }
            }
            if(type == "image")
            {
                result.has_image = true;
            }
            if(type == "json")
            {
                result.has_json = true;
                if(seg["data"].contains("data"))
                {
                    result.json_data = json::parse(seg["data"]["data"].get<std::string>());
                }
            }
        }
        while(!result.text.empty() && result.text[0] == ' ')
        {   // 去除文本消息的前置空格
            result.text.erase(0, 1);
        }
        return result;
    }
public:
    // 统一发送消息接口
    static void send_msg(const MessageContext& recv, const json& reply)
    {
        httplib::Client cli(SERVER_HOST, SERVER_PORT);
        // 群聊消息
        if(recv.msg_type == "group")
        {
            for(auto& seg : reply)
            {
                json body;
                body["group_id"] = recv.group_id;
                body["message"] = seg;
                // 注意加上 token 否则会 403 导致不能回复
                httplib::Headers headers = {{"Authorization", "Bearer " + SERVER_ACCESS_TOKEN}};
                auto res = cli.Post("/send_group_msg", headers, body.dump(), "application/json");
                if (!res)
                {
                    Logger::error("群聊消息发送失败", httplib::to_string(res.error()));
                }
                if(res->status != 200)
                {
                    Logger::warn("send_group_msg HTTP状态码: ", res->status);
                    Logger::error("send_group_msg 异常响应体:", json::parse(res->body).dump(4));
                }
            }
        }
        // 私聊消息
        if(recv.msg_type == "private")
        {
            for(auto& seg : reply)
            {
                json body;
                body["user_id"] = recv.user_id;
                body["message"] = seg;
                // 注意加上 token 否则会 403 导致不能回复
                httplib::Headers headers = {{"Authorization", "Bearer " + SERVER_ACCESS_TOKEN}};
                auto res = cli.Post("/send_private_msg", headers, body.dump(), "application/json");
                if (!res)
                {
                    Logger::error("私聊消息发送失败", httplib::to_string(res.error()));
                }
                if(res->status != 200)
                {
                    Logger::warn("send_private_msg HTTP状态码: ", res->status);
                    Logger::error("send_private_msg 异常响应体: ", json::parse(res->body).dump(4));
                }
            }
        }
    }
    // 获取消息结构
    static MessageContext getMessageContext(const json& data)
    {
        MessageContext msgctx;
        msgctx.msg_type = data["message_type"].get<std::string>();
        if(msgctx.msg_type == "group") msgctx.group_id = std::to_string(data["group_id"].get<size_t>());
        msgctx.user_id = std::to_string(data["user_id"].get<size_t>());
        msgctx.msg_segments = data["message"].get<json>();
        msgctx.pmsgsegs = parseMsgSegments(msgctx.msg_segments);
        return msgctx;
    }
    // 构造array格式消息
    static json buildMsg(const std::string& msg_type, const std::string& msg_data)
    {
        json result;
        if(msg_type == "at")
        {
            json data;
            data["qq"] = msg_data;
            result["data"] = data;
            result["type"] = msg_type;
        }
        if(msg_type == "text")
        {
            json data;
            data["text"] = msg_data;
            result["data"] = data;
            result["type"] = msg_type;
        }
        if(msg_type == "image" || msg_type == "video")
        {
            json data;
            data["file"] = msg_data;
            result["data"] = data;
            result["type"] = msg_type;
        }
        return result;
    }
};

//////////
// 文件管理器
///////////
class FileManager{
private:
    static std::unordered_map<std::string, std::shared_ptr<std::mutex>> file_mutex_map; // 文件锁表
    static std::mutex file_mutex_map_lock; // 保护文件锁表本身
    // 获取文件锁
    static std::shared_ptr<std::mutex> getFileMutex(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(file_mutex_map_lock); // 文件锁表上锁
        if(file_mutex_map.find(path) == file_mutex_map.end())
        {
            file_mutex_map[path] = std::make_shared<std::mutex>();
        }
        return file_mutex_map[path];
    }

    static bool isValidFilename(const std::string& filename)
    {
        // 非空
        if(filename.empty())
        {
            Logger::warn("文件名为空", "");
            return false;
        }
        // 检查文件名结尾空格或点
        if(!filename.empty() && (filename.back() == ' ' || filename.back() == '.'))
        {
            Logger::warn("文件名不能以空格或点结尾: ", filename);
            return false;
        }
        // 检查ASCII控制字符 (0x00-0x1F)
        for(char c : filename)
        {
            if (static_cast<unsigned char>(c) < 32)
            {
                Logger::warn("文件名包含控制字符", filename);
                return false;
            }
        }
        // 非法字符
        std::string illegal_chars = "<>:\"/\\|?*";
        for(char c : illegal_chars)
        {
            if (filename.find(c) != std::string::npos)
            {
                Logger::warn("文件名包含非法字符", filename);
                return false;
            }
        }
        return true;
    }

public:
    // 缓存清理函数
    static void cleanCache()
    {
        std::filesystem::path cache_dir = CACHE_PATH;
        if(!std::filesystem::exists(cache_dir)) return;
        // 收集所有文件
        std::vector<std::filesystem::path> files;
        for(const auto& entry : std::filesystem::directory_iterator(cache_dir))
        {
            if (entry.is_regular_file())
            {
                files.push_back(entry.path());
            }
        }
        // 如果文件数量小于等于限制则不清理
        if(files.size() <= CACHE_FILE_LIMIT)
        {
            Logger::info("缓存文件数量可接受, 无需清理, 当前缓存文件数量: ", files.size());
            return;
        }
        // 按最后修改时间排序（最旧的在前）
        std::sort(files.begin(), files.end(), 
                  [](const std::filesystem::path& a, const std::filesystem::path& b){
                      return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
                  });
        // 计算需要删除的数量
        size_t files_to_delete = files.size() - CACHE_FILE_LIMIT;
        Logger::info("缓存文件数量超过10, 将删除最旧的文件, 需清理文件数: ", files_to_delete);
        // 删除最旧的文件
        for(size_t i = 0; i < files_to_delete; i++) 
        {
            std::error_code ec;
            std::filesystem::remove(files[i], ec);
            if(!ec)
            {
                Logger::info("已删除旧缓存: ", files[i].filename().string()); 
                // 从锁表中移除
                std::lock_guard<std::mutex> lock(file_mutex_map_lock);
                file_mutex_map.erase(files[i].generic_string());
            }
        }
    }
    // 下载函数
    static std::string downloadFile(const std::string& url, const httplib::Headers& headers, const std::string& filename)
    {
        // 先清理缓存
        cleanCache();
        // 检查文件名称
        if(!isValidFilename(filename)) return "";
        // 使用正则表达式解析url
        std::regex url_pattern(R"(https?://([^/:]+)(:\d+)?(/.*))");
        std::smatch match;
        if(!std::regex_match(url, match, url_pattern))
        {
            Logger::error("URL 解析失败", url);
            return "";
        }
        std::string host = match[1];
        std::string path = match[3];
        // 创建 cache 目录
        std::filesystem::path cache_dir = CACHE_PATH;
        std::filesystem::create_directories(cache_dir); // 这个函数是原子操作
        Logger::info("cache目录: ", cache_dir);
        // 获取下载文件路径
        std::filesystem::path save_path = cache_dir / filename;
        std::shared_ptr<std::mutex> file_mutex = getFileMutex(save_path.generic_string());
        std::lock_guard<std::mutex> file_lock(*file_mutex); // 上锁
        if(std::filesystem::exists(save_path))
        {
            Logger::info("cache目录中已经存在文件", filename);
            return save_path.generic_string();
        }
        // 建立SSL客户端
        httplib::SSLClient cli(host);
        cli.set_follow_location(true);
        cli.enable_server_certificate_verification(false); // 下载关闭证书
        cli.set_read_timeout(60);   // 防止卡死
        cli.set_write_timeout(60);
        // 创建文件
        std::ofstream ofs(save_path, std::ios::binary);
        if (!ofs.is_open())
        {
            Logger::error("文件创建失败: ", save_path.generic_string());
            return "";
        }
        // 设置下载缓冲区
        std::unique_ptr<char[]> buffer(new char[DOWNLOAD_BUFFER_SIZE]);
        ofs.rdbuf()->pubsetbuf(buffer.get(), DOWNLOAD_BUFFER_SIZE);
        // 流式下载
        std::atomic<size_t> downloaded_size{0};
        bool size_limit_exceeded = false;
        auto res = cli.Get(path, headers, [&](const char* data, size_t data_length){
                if(downloaded_size.load() + data_length > DOWNLOAD_SIZE_LIMIT)
                {
                    size_limit_exceeded = true;
                    Logger::warn("下载过程中超过大小限制: ", downloaded_size + data_length);
                    return false;  // 返回 false 中断下载
                }
                ofs.write(data, data_length);
                downloaded_size += data_length; 
                return true;
            });
        ofs.close();
        if (!res)
        {
            Logger::error("文件下载失败", httplib::to_string(res.error()));
            std::filesystem::remove(save_path);
            return "";
        }
        if(res->status != 200)
        {
            Logger::warn("文件下载异常 HTTP状态码: ", res->status);
            std::filesystem::remove(save_path);
            return "";
        }
        // 检查是否下载超过大小停止
        if(size_limit_exceeded)
        {
            std::filesystem::remove(save_path);
            return "";
        }
        // 返回路径
        Logger::info("文件已下载到: ", save_path.generic_string());
        return save_path.generic_string(); // generic_string方法, 使用 / 
    }
};
// 定义文件锁表
std::unordered_map<std::string, std::shared_ptr<std::mutex>> FileManager::file_mutex_map;
std::mutex FileManager::file_mutex_map_lock;

//////////
// 文本指令接口
//////////
class Command{
public:
    virtual std::string name() = 0;
    virtual json execute(const std::string& args) = 0;
    virtual ~Command() = default;
};

///////////
// 各个文本指令
///////////
// 帮助
class HelpCommand : public Command{
private:
    std::string cmd_list;
public:
    HelpCommand(std::string cl)
    {
        cmd_list = cl;
    }
    std::string name() override
    {
        return "帮助";
    }
    json execute(const std::string& args) override
    {
        // 返回维护的指令列表
        return MessageManager::buildMsg("text", "指令格式: @我 指令\n当前支持的指令:\n" + cmd_list);
    }
    ~HelpCommand() override
    {
        return;
    }
};
// 时间
class TimeCommand : public Command{
private:
    // 获取当前时间的函数
    std::string getFormattedTime() 
    { 
        time_t now = time(0);
        tm local_time;
        if(localtime_r(&now, &local_time) == nullptr)
        {
            Logger::error("获取本地时间失败", "");
            return "时间获取失败";
        }
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_time);
        return std::string(buffer);
    }
public:
    std::string name() override
    {
        return "时间";
    }
    json execute(const std::string& args) override
    {
        return MessageManager::buildMsg("text", getFormattedTime());
    }
    ~TimeCommand() override
    {
        return;
    }
};
// 天气
class WeatherCommand : public Command{
private:
    // 简单提取城市名称
    std::string getCityName(const std::string& args)
    {
        if (args.empty()) 
        {
            return "";
        }
        std::string city = args;
        // 去除可能的前置空格
        while(!city.empty() && city[0] == ' ')
        {
            city.erase(0, 1);
        }
        // 去除可能的末尾空格
        while (!city.empty() && city.back() == ' ')
        {
            city.pop_back();
        }
        // 只取第一个城市名称
        size_t pos = city.find(' ');
        if (pos != std::string::npos) 
        {
            city = city.substr(0, pos);
        }
        return city;
    }

    // 天气信息结构体
    struct Weatherinfo{
        std::string city; // 城市
        std::string weather; // 天气
        std::string temper; // 温度
        std::string winddir; // 风向
        std::string windpow; // 风力
        std::string humidity; // 湿度
        std::string reporttime; // 查询时间
    };
    // 获取天气信息
    Weatherinfo getWeatherinfo(const json& raw_data)
    {
        Weatherinfo result;
        const json& data = raw_data["lives"][0];
        result.city = data["province"].get<std::string>() + " "
            + data["city"].get<std::string>();
        result.weather = data["weather"].get<std::string>();
        result.temper = data["temperature"].get<std::string>();
        result.winddir = data["winddirection"].get<std::string>();
        result.windpow = data["windpower"].get<std::string>();
        result.humidity = data["humidity"].get<std::string>();
        result.reporttime = data["reporttime"].get<std::string>();
        return result;
    }
    // 访问Amap api获取信息
    std::string askAmap(const std::string& args)
    {
        std::string city = getCityName(args);
        if(city.empty())
        {
            return "请输入城市名称, 格式: 天气 城市名称";
        }
        httplib::SSLClient cli(AMAP_CLIENT_HOST, AMAP_CLIENT_PORT);
        auto res = cli.Get(AMAP_GET_PATH + "?city=" + city + "&key=" + AMAP_KEY);
        if(!res)
        {
            Logger::error("天气网络请求失败", httplib::to_string(res.error()));
            return "天气网络请求失败";
        }
        if(res->status != 200)
        {
            Logger::warn("天气请求 HTTP状态码: ", res->status);
            Logger::error("天气请求 异常响应体:", json::parse(res->body).dump(4));
            return "天气请求异常";
        }
        // 先解析返回的JSON
        json raw_data = json::parse(res->body);
        if(raw_data["lives"].empty())
        {
            Logger::warn("天气查询返回异常", raw_data.dump(4));
            return "天气查询失败, 请输入正确的格式: 天气 城市名称\n暂时只支持国内城市";
        }
        Weatherinfo winfo = getWeatherinfo(raw_data);
        std::string result = "城市: " + winfo.city;
        result += "\n天气: " + winfo.weather;
        result += "\n温度: " + winfo.temper + "℃";
        result += "\n风向: " + winfo.winddir + "风 " + winfo.windpow + "级";
        result += "\n湿度: " + winfo.humidity + "%";
        result += "\n查询时间: " + winfo.reporttime;
        return result;
    }
public:
    std::string name() override
    {
        return "天气";
    }
    json execute(const std::string& args) override
    {
        return MessageManager::buildMsg("text", askAmap(args));
    }
    ~WeatherCommand() override
    {
        return;
    }
};

// 随机图片
class RandomImgCommand : public Command{
private:
    std::string getImgUrl()
    {
        httplib::SSLClient cli(IMG_CLIENT_HOST, IMG_CLIENT_PORT);
        auto res = cli.Get(IMG_GET_PATH);
        if(!res)
        {
            Logger::error("随机图片网络请求失败", httplib::to_string(res.error()));
            return "";
        }
        if(res->status != 200)
        {
            Logger::warn("随机图片请求 HTTP状态码: ", res->status);
            Logger::error("随机图片请求 异常响应体:", json::parse(res->body).dump(4));
            return "";
        }
        json data = json::parse(res->body);
        std::string img_url = data["url"].get<std::string>();
        return img_url;
    }

public:
    std::string name() override
    {
        return "随机二次元图片";
    }

    json execute(const std::string& args) override
    {
        return MessageManager::buildMsg("image", getImgUrl());
    }   

    ~RandomImgCommand() override
    {
        return;
    }
};



///////////
// 任务管理器接口
///////////
class BaseTaskManager{
public:
    virtual bool canHandle(const MessageContext& msgctx) = 0;
    virtual json handleTask(const MessageContext& msgctx) = 0;
    virtual ~BaseTaskManager() = default;
};

///////////////////// 各个任务管理器
////////////
// 被at的文本命令任务管理器
////////////
class CmdTaskManager : public BaseTaskManager{
private: 
    // 维护一个指令表，用于存储多种可支持的指令
    std::unordered_map<std::string, std::unique_ptr<Command>> cmd_map;
    // 注册一个用指令接口实现的指令
    void registerCommand(std::unique_ptr<Command> cmd)
    { 
        cmd_map[cmd->name()] = std::move(cmd);
    }

public:
    // 用于获取指令列表
    std::string getCommandList()
    {
        std::string cmd_list = "帮助\n"; // 初始 list 带有帮助指令
        for(const auto& cmd : cmd_map)
        {
            cmd_list += cmd.first + "\n"; 
        }
        // 去除最后一个回车符
        cmd_list.pop_back();
        return cmd_list;
    }

    CmdTaskManager()
    {
        // 注册各个指令
        registerCommand(std::make_unique<TimeCommand>());
        registerCommand(std::make_unique<WeatherCommand>());
        registerCommand(std::make_unique<RandomImgCommand>());
        // 后续文本指令也在此注册
        registerCommand(std::make_unique<HelpCommand>(getCommandList()));
    }

    // 能否处理
    bool canHandle(const MessageContext& msgctx) override
    {
        // 仅能处理群聊中被at的消息
        if(!(msgctx.pmsgsegs.at_me && (msgctx.msg_type == "group")))
        return false;
        if(msgctx.pmsgsegs.text.empty()) return "指令格式: @我 指令\n先试试 帮助 吧";
        // 先按照空格分割指令名称和参数
        size_t pos = msgctx.pmsgsegs.text.find(' ');
        std::string cmd_name = msgctx.pmsgsegs.text.substr(0, pos);
        // 去除可能的前置空格
        while(!cmd_name.empty() && cmd_name[0] == ' ')
        {
            cmd_name.erase(0, 1);
        }
        return cmd_map.count(cmd_name); // 仅能处理指令表中存在的指令
    }

    // 处理某个被at的文本指令
    json handleTask(const MessageContext& msgctx) override
    {
        if(msgctx.pmsgsegs.text.empty()) return "指令格式: @我 指令\n先试试 帮助 吧";
        // 执行指令
        // 先按照空格分割指令名称和参数
        size_t pos = msgctx.pmsgsegs.text.find(' ');
        std::string cmd_name = msgctx.pmsgsegs.text.substr(0, pos);
        // 去除可能的前置空格
        while(!cmd_name.empty() && cmd_name[0] == ' ')
        {
            cmd_name.erase(0, 1);
        }
        std::string cmd_args = (pos == std::string::npos) ? "" : msgctx.pmsgsegs.text.substr(pos + 1);
        // 去除可能的前置空格
        while(!cmd_args.empty() && cmd_args[0] == ' ')
        {
            cmd_args.erase(0, 1);
        }
        json result = json::array();
        if(cmd_map.count(cmd_name))
        {
            // 调用对应指令
            result.emplace_back(cmd_map[cmd_name]->execute(cmd_args));
            return result;
        }
        result.emplace_back(MessageManager::buildMsg("text", "未知指令: " + cmd_name));
        return result;
    }
};

//////////////
// JSON消息任务管理器
//////////////
class JsonTaskManager : public BaseTaskManager{
private:
    // B站视频信息结构
    struct BVinfo{
        std::string title;
        std::string bvid;
        std::string cid; // 获取视频需要的请求参数
        std::string up; // up主昵称
        std::string face; // 头像url
        std::string url; // 视频URL
        size_t size; // 视频大小
        int view; // 观看次数
        int reply; // 评论数
        int favorite; // 收藏数
        int coin; // 投币数
        int share; // 分享数
        int like; // 点赞数
    };
    // 获取 bvid
    std::string getBVid(const json& data)
    {
        std::string qqdocurl = data["meta"]["detail_1"]["qqdocurl"].get<std::string>();
        // 使用正则表达式解析url
        std::regex url_pattern(R"(https?://([^/:]+)(:\d+)?(/.*))");
        std::smatch url_match;
        if(!std::regex_match(qqdocurl, url_match, url_pattern))
        {
            Logger::error("URL 解析失败", qqdocurl);
            return "";
        }
        // 找到qq分享的b站视频链接的 host 和 path
        std::string host = url_match[1];
        std::string path = url_match[3];
        httplib::SSLClient cli(host, 443); // 默认443端口
        auto res = cli.Head(path); // 这里仅为了获取重定向后的url, 所以用Head
        if(!res)
        {
            Logger::error("B站短链网络请求失败", httplib::to_string(res.error()));
            return "B站视频短链网络请求失败";
        }
        if(res->status != 302)
        {
            Logger::warn("B站短链 HTTP状态码: ", res->status);
            Logger::error("B站短链 异常响应体:", json::parse(res->body).dump(4));
            return "B站视频短链网络请求异常";
        }
        std::string real_url = res->get_header_value("Location"); // 获取重定向后的真正B站url
        // 正则匹配获取bvid
        std::regex bv_pattern(R"(BV[A-Za-z0-9]{10})");
        std::smatch bv_match; 
        if (std::regex_search(real_url, bv_match, bv_pattern))
            return bv_match[0];  // 返回匹配到的BV号
        Logger::error("未找到BV号", real_url);
        return "";
    }
    // 获取B站视频信息
    BVinfo getBVinfo(const json& raw_data)
    {
        BVinfo bvinfo;
        const json& data = raw_data["data"];
        bvinfo.cid = std::to_string(data["cid"].get<size_t>());
        bvinfo.bvid = data["bvid"].get<std::string>();
        bvinfo.title = data["title"].get<std::string>();
        bvinfo.up = data["owner"]["name"].get<std::string>();
        bvinfo.face = data["owner"]["face"].get<std::string>();
        bvinfo.view = data["stat"]["view"].get<int>();
        bvinfo.reply = data["stat"]["reply"].get<int>();
        bvinfo.favorite = data["stat"]["favorite"].get<int>();
        bvinfo.coin = data["stat"]["coin"].get<int>();
        bvinfo.share = data["stat"]["share"].get<int>();
        bvinfo.like = data["stat"]["like"].get<int>();
        getBVUrlandSize(bvinfo.bvid, bvinfo.cid, bvinfo);
        return bvinfo;
    }
    // 获取B站视频直链url和视频大小
    void getBVUrlandSize(const std::string& bvid, const std::string& cid, BVinfo& bvinfo)
    {
        httplib::SSLClient cli(B23_CLIENT_HOST, B23_CLIENT_PORT);
        auto res = cli.Get(B23_PLAY_PATH + "?bvid=" + bvid + "&cid=" + cid);
        if (!res)
        {
            Logger::error("B站播放请求失败", httplib::to_string(res.error()));
            return;
        }
        if(res->status != 200)
        {
            Logger::warn("B站播放api HTTP状态码: ", res->status);
            Logger::error("B站播放api 异常响应体:", json::parse(res->body).dump(4));
            return;
        }
        json data = json::parse(res->body);
        if(data["code"].get<int>() != 0 || data["message"].get<std::string>() != "OK" || data["data"]["durl"].empty())
        {
            Logger::error("获取视频播放信息异常", data);
            return;
        }
        bvinfo.url = data["data"]["durl"][0]["url"].get<std::string>();
        Logger::info("获取视频链接: ", bvinfo.url);
        bvinfo.size = data["data"]["durl"][0]["size"].get<size_t>();
        Logger::info("获取视频大小: ", bvinfo.size);
    }   
    
    // 处理B站视频
    std::pair<std::string, std::string> handleBV(const json& data)
    {
        std::string bvid = getBVid(data);
        Logger::info("Bvid: ", bvid);
        httplib::SSLClient cli(B23_CLIENT_HOST, B23_CLIENT_PORT);
        auto res = cli.Get(B23_QUERY_PATH + "?bvid=" + bvid);
        if (!res)
        {
            Logger::error("B站查询请求失败", httplib::to_string(res.error()));
            return std::make_pair("网络请求失败", "");
        }
        if(res->status != 200)
        {
            Logger::warn("B站查询api HTTP状态码: ", res->status);
            Logger::error("B站查询api 异常响应体:", json::parse(res->body).dump(4));
            return std::make_pair("获取视频信息失败", "");
        }
        json raw_bvinfo = json::parse(res->body);
        if(raw_bvinfo["code"].get<int>() != 0 || raw_bvinfo["message"].get<std::string>() != "OK")
        {
            Logger::error("获取视频信息异常", raw_bvinfo);
            return std::make_pair("获取视频信息异常", "");
        }
        // 解析并处理B站视频信息
        BVinfo bvinfo = getBVinfo(raw_bvinfo);
        std::string result;
        result = "视频标题: " + bvinfo.title;
        result += "\nup主: " + bvinfo.up;
        result += "\nup主头像: " + bvinfo.face;
        result += "\nbvid: " + bvinfo.bvid;
        result += "\n观看次数: " + std::to_string(bvinfo.view);
        result += "\n评论数: " + std::to_string(bvinfo.reply);
        result += "\n收藏数: " + std::to_string(bvinfo.favorite);
        result += "\n投币数: " + std::to_string(bvinfo.coin);
        result += "\n分享数: " + std::to_string(bvinfo.share);
        result += "\n点赞数: " + std::to_string(bvinfo.like);
        // 下载B站视频
        httplib::Headers headers = {
            {"Referer", "https://www.bilibili.com"},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}
        };
        std::string video_path = FileManager::downloadFile(bvinfo.url, headers, bvinfo.bvid + ".mp4");
        return std::make_pair(result, video_path);
    }
public:
    bool canHandle(const MessageContext& msgctx) override
    {
        // 当群聊消息类型为 json 时能处理
        return (msgctx.msg_type == "group") && msgctx.pmsgsegs.has_json;
    }
    json handleTask(const MessageContext& msgctx) override
    {
        const json& data = msgctx.pmsgsegs.json_data;
        if(data.contains("meta"))
        {
            if(data["meta"].contains("detail_1"))
            {
                if(data["meta"]["detail_1"]["appid"].get<std::string>() == B23_APP_ID)
                { // 暂时只处理B站分享视频
                    auto [reply_text, video_path] = handleBV(data);
                    json result = json::array();
                    result.emplace_back(MessageManager::buildMsg("text", reply_text));
                    if(!video_path.empty())
                    {
                        result.emplace_back(MessageManager::buildMsg("video", "file:///" + video_path));
                    }else{
                        result.emplace_back(MessageManager::buildMsg("text", "视频下载异常, 可能是视频太大了"));
                    }
                    return result;
                }
            }
        }
        Logger::warn("json 消息内容不符合预期", data);
        return {};
    }
};

// AI对话任务
class ChatTaskManager : public BaseTaskManager{
private:
    // AI 对话历史结构体
    struct ChatMemCtx{
        std::string role;
        std::string content;
    };
    // 对话历史保存
    static std::unordered_map<std::string, std::vector<ChatMemCtx>> chat_memory;
    // 对话历史表锁表
    static std::unordered_map<std::string, std::shared_ptr<std::mutex>> chat_mutex_map;
    // 保护对话历史表锁表的锁
    static std::mutex chat_mutex_map_lock;
    // 获取对话 session id
    std::string getSessionId(const MessageContext& msgctx)
    {
        if(msgctx.msg_type == "group")
        {
            return "group_" + msgctx.group_id + "_user_" + msgctx.user_id;
        }else{
            return "private_" + msgctx.user_id;
        }
        return "";
    }
    // 获取或者创建 session 对应的锁
    std::shared_ptr<std::mutex> getSessionMutex(const std::string& session_id)
    {
        std::lock_guard<std::mutex> lock(chat_mutex_map_lock);
        if(chat_mutex_map.find(session_id) == chat_mutex_map.end())
        {
            chat_mutex_map[session_id] = std::make_shared<std::mutex>();
        }
        return chat_mutex_map[session_id];
    }
    // 获取或者创建 session 对应的对话历史
    std::vector<ChatMemCtx> getSessionHistory(const std::string& session_id)
    {
        // 上锁
        std::shared_ptr<std::mutex> session_mutex = getSessionMutex(session_id);
        std::lock_guard<std::mutex> session_lock(*session_mutex);
        return chat_memory[session_id];
    }
    // 更新对话历史
    void UpdateChatMemory(const std::string& session_id, const std::string& user_input, const std::string& ai_reply)
    {
        ChatMemCtx mem1;
        ChatMemCtx mem2;
        mem1.role = "user";
        mem1.content = user_input;
        mem2.role = "assistant";
        mem2.content = ai_reply;
        // 先上锁
        std::shared_ptr<std::mutex> session_mutex = getSessionMutex(session_id);
        std::lock_guard<std::mutex> session_lock(*session_mutex);
        chat_memory[session_id].emplace_back(mem1);
        chat_memory[session_id].emplace_back(mem2);
        // 如果历史过多则清理历史
        if(chat_memory[session_id].size() > MAX_CHAT_HISTORY)
        {
            chat_memory[session_id].erase(chat_memory[session_id].begin(), chat_memory[session_id].begin()+2);
        }
    }
    //与AI交互
    std::string askAI(const std::string& session_id, const std::string& user_input)
    {
        if(user_input.empty())
        {
            return "请输入对话内容";
        }
        httplib::SSLClient cli(AI_CLIENT_HOST, AI_CLIENT_PORT);
        json body;
        httplib::Headers headers = {
            {"Authorization", "Bearer " + AI_KEY},
            {"Content-Type", "application/json"}
        };
        // 获取或者创建对话历史
        auto history = getSessionHistory(session_id);
        Logger::info("获取对话历史成功, 轮数: ", history.size()/2);
        json messages = json::array();
        // 加入系统提示词
        messages.push_back({
            {"role","system"},
            {"content",AI_DEFAULT_SYS_PROMPTS}
        });
        // 加入对话历史
        for(auto& msg : history)
        {
            messages.push_back({
                {"role", msg.role},
                {"content", msg.content}
            });
        }
        // 加入用户输入
        messages.push_back({
            {"role", "user"},
            {"content", user_input}
        });
        // 构造请求体
        body["model"] = AI_MODEL;
        body["messages"] = messages;
        body["max_tokens"] = AI_MAX_TOKENS;
        auto res = cli.Post(AI_POST_PATH, headers, body.dump(), "application/json");
        // 硅基流动 post 请求示例
        //         curl --request POST \
        //   --url https://api.siliconflow.cn/v1/chat/completions \
        //   -H "Content-Type: application/json" \
        //   -H "Authorization: Bearer YOUR_API_KEY" \
        //   -d '{
        //     "model": "Pro/zai-org/GLM-4.7",
        //     "messages": [
        //       {"role": "system", "content": "你是一个有用的助手"},
        //       {"role": "user", "content": "你好，请介绍一下你自己"}
        //     ]
        //   }'
        if(!res)
        {
            Logger::error("AI网络请求失败", httplib::to_string(res.error()));
            return "AI网络请求失败";
        }
        if(res->status != 200)
        {
            Logger::warn("AI请求 HTTP状态码: ", res->status);
            Logger::error("AI请求 异常响应体:", json::parse(res->body).dump(4));
            return "AI请求异常";
        }
        json raw_data = json::parse(res->body);
        if (!raw_data.contains("choices") || raw_data["choices"].empty())
        {
            Logger::warn("AI回复异常", raw_data.dump(4));
            return "AI 回复异常";
        }
        std::string ai_reply = raw_data["choices"][0]["message"]["content"].get<std::string>();
        Logger::info("获取ai回复成功, 长度: ", ai_reply.length()); 
        UpdateChatMemory(session_id, user_input, ai_reply);
        Logger::info("更新对应 session 对话历史成功, 轮数: ", chat_memory[session_id].size()/2);
        return ai_reply;
    }
public:
    bool canHandle(const MessageContext& msgctx) override
    {
        return msgctx.pmsgsegs.at_me && (msgctx.msg_type == "group");
    }    

    json handleTask(const MessageContext& msgctx) override
    {
        std::string session_id = getSessionId(msgctx);
        std::string user_input = msgctx.pmsgsegs.text;
        return MessageManager::buildMsg("text", askAI(session_id, user_input));
    }
};
std::unordered_map<std::string, std::vector<ChatTaskManager::ChatMemCtx>> ChatTaskManager::chat_memory;
std::unordered_map<std::string, std::shared_ptr<std::mutex>> ChatTaskManager::chat_mutex_map;
std::mutex ChatTaskManager::chat_mutex_map_lock;


//////////////
// 总的任务管理器
//////////////
class TaskManager{
private:
    std::vector<std::unique_ptr<BaseTaskManager>> tsk_managers;
    void registerTaskManager(std::unique_ptr<BaseTaskManager> tsk_manager)
    {
        tsk_managers.emplace_back(std::move(tsk_manager));
    }

public:
    TaskManager()
    {
        // 注册特定任务管理器
        registerTaskManager(std::make_unique<CmdTaskManager>());
        registerTaskManager(std::make_unique<ChatTaskManager>()); // 注意这里AI任务优先级低于命令任务
        registerTaskManager(std::make_unique<JsonTaskManager>());
    }
    // 总的任务处理函数
    json handleTask(const MessageContext& msgctx)
    {
        for(auto& tsk_manager : tsk_managers)
        {
            if(tsk_manager->canHandle(msgctx))
            {
                // 按照能否处理自动分发任务处理器
                return tsk_manager->handleTask(msgctx);
            }
        }
        return {};
    }
};

////////////
// 总管理器
////////////
class Manager{
private:
    // 创建服务器
    httplib::Server svr;
    
public:
    // 创建任务管理器
    TaskManager tsk_manager;

    Manager()
    {
        // 注册 POST 路由
        svr.Post("/", [this](const httplib::Request& req, httplib::Response& res){
            this->handlePost(req, res);
        });
    }

    // 启动服务器
    void start()
    {
        svr.listen(CLIENT_HOST, CLIENT_PORT);
    }

    // 处理 POST 请求
    void handlePost(const httplib::Request& req, httplib::Response& res)
    {
        // 先解析JSON
        json data = json::parse(req.body);
        ////////////////////////////////////////////// 只处理消息事件, 其余事件todo
        if (data["post_type"] != "message")
        {
            res.set_content("{}", "text/plain");
            return;
        }

        // 构造 MessageContext
        MessageContext msgctx;
        msgctx = MessageManager::getMessageContext(data);
        Logger::info("获取消息结构成功", msgctx.msg_segments);
        // 调用任务管理器 得到回复
        json reply = tsk_manager.handleTask(msgctx);
        Logger::info("回复内容: ", reply);
        if (!reply.empty())
        {
            MessageManager::send_msg(msgctx, reply);
        }
        res.set_content("{}", "text/plain");
    }
};

int main()
{
    Logger::info(" === 创建管理器", " === ");
    Manager m;
    Logger::info(" === 开始监听", " === ");
    m.start();
    return 0;
}
// g++ src/main.cpp -O2 -Iinclude -o bot -lssl -lcrypto -lpthread