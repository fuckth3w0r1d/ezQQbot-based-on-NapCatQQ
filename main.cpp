#include <bits/stdc++.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

//////////////////////// config
// naocatqq
std::string BOT_QQ;
std::string SERVER_HOST;
int SERVER_PORT;
std::string SERVER_ACCESS_TOKEN;
std::string CLIENT_HOST;
int CLIENT_PORT;
std::string CLIENT_ACCESS_TOKEN;
// amap
std::string AMAP_KEY;
std::string AMAP_CLIENT_HOST;
int AMAP_CLIENT_PORT;
std::string AMAP_GET_PATH;
// ai
std::string AI_KEY;
std::string AI_CLIENT_HOST;
int AI_CLIENT_PORT;
std::string AI_POST_PATH;
std::string AI_MODEL;
std::string AI_SYS_PROMPTS;
int AI_MAX_TOKENS;

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
};

void getAllConfigVal()
{
    Config cfg;
    // napcatqq
    BOT_QQ = cfg.getBotqq();
    SERVER_HOST = cfg.getServerHost();
    SERVER_PORT = cfg.getServerPort();
    SERVER_ACCESS_TOKEN = cfg.getServerToken();
    CLIENT_HOST = cfg.getClientHost();
    CLIENT_PORT = cfg.getClientPort();
    CLIENT_ACCESS_TOKEN = cfg.getClientToken();
    // amap
    AMAP_CLIENT_HOST = cfg.getAmapHost();
    AMAP_CLIENT_PORT = cfg.getAmapPort();
    AMAP_KEY = cfg.getAmapKey();
    AMAP_GET_PATH = cfg.getAmapGetPath();
    // ai
    AI_CLIENT_HOST = cfg.getAIHost();
    AI_CLIENT_PORT = cfg.getAIPort();
    AI_KEY = cfg.getAIKey();
    AI_POST_PATH = cfg.getAIPostPath();
    AI_MODEL = cfg.getAIModel();
    AI_SYS_PROMPTS = cfg.getAISysPrompts();
    AI_MAX_TOKENS = cfg.getAIMaxTokens();
}

// 消息结构
struct ParsedMsgSegments{
    bool at_me = false;
    bool has_text = false;
    bool has_image = false;
    bool has_json = false;

    std::string text;        // 拼接后的纯文本
    json json_data;          // 如果有分享卡片
};
struct MessageContext{
    size_t group_id;
    size_t user_id;
    std::string msg_type;
    json msg_segments;
    ParsedMsgSegments pmsgsegs;
};

////////////
// 消息管理器
///////////
class MessageManager{
public:
    // 群聊回复函数
    void send_group_msg(size_t group_id, const std::string& message)
    { 
        httplib::Client cli(SERVER_HOST, SERVER_PORT);
        json body;
        body["group_id"] = group_id;
        body["message"] = message;
        // 注意加上 token 否则会 403 导致不能回复
        httplib::Headers headers = {{"Authorization", "Bearer " + SERVER_ACCESS_TOKEN}};
        auto res = cli.Post("/send_group_msg", headers, body.dump(), "application/json");
        if (!res)
        {
            std::cerr << "群聊消息发送失败" << std::endl;
        }else{
            std::cout << "HTTP状态码: " << res->status << std::endl;
        }
    }
    // 私聊回复函数
    void send_private_msg(size_t user_id, const std::string& message)
    {
        httplib::Client cli(SERVER_HOST, SERVER_PORT);
        json body;
        body["user_id"] = user_id;
        body["message"] = message;
        httplib::Headers headers = {{"Authorization", "Bearer " + SERVER_ACCESS_TOKEN}};
        auto res = cli.Post("/send_private_msg", headers, body.dump(), "application/json");
        if (!res)
        {
            std::cerr << "私聊消息发送失败" << std::endl;
        }else{
            std::cout << "HTTP状态码: " << res->status << std::endl;
        }
    }
    // 消息段解析
    ParsedMsgSegments parseMsgSegments(json& msgsegs)
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
        return result;
    }
    // 获取消息结构
    MessageContext getMessageContext(json& data)
    {
        MessageContext msgctx;
        msgctx.msg_type = data["message_type"];
        if (msgctx.msg_type == "group") msgctx.group_id = data["group_id"]; // 私聊没有 group_id
        msgctx.user_id = data["user_id"];
        msgctx.msg_segments = data["message"];
        msgctx.pmsgsegs = parseMsgSegments(msgctx.msg_segments);
        return msgctx;
    }
};


//////////
// 文本指令接口
//////////
class Command{
public:
    virtual std::string name() = 0;
    virtual std::string execute(std::string& args) = 0;
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
    std::string execute(std::string& args) override
    {
        // 返回维护的指令列表
        return "指令格式: @我 指令\n当前支持的指令:\n" + cmd_list;
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
        tm* ltm = localtime(&now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
        return std::string(buffer);
    }
public:
    std::string name() override
    {
        return "时间";
    }
    std::string execute(std::string& args) override
    {
        return getFormattedTime();
    }
    ~TimeCommand() override
    {
        return;
    }
};
// 天气
class WeatherCommand : public Command{
public:
    std::string name() override
    {
        return "天气";
    }
    std::string execute(std::string& args) override
    {
        if (args.empty()) 
        {
            return "请输入城市名, 例如: 天气 北京";
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
        httplib::SSLClient cli(AMAP_CLIENT_HOST, AMAP_CLIENT_PORT);
        auto res = cli.Get(AMAP_GET_PATH + "?city=" + city + "&key=" + AMAP_KEY);
        if (res && res->status == 200)
        {
            // 先解析返回的JSON
            json raw_data = json::parse(res->body);
            if(raw_data["lives"].empty())
            {
                return "天气查询失败, 请输入正确的格式: 天气 城市名称\n暂时只支持国内城市";
            }
            auto data = raw_data["lives"][0];
            std::string result = "城市: " + data["province"].get<std::string>() + " "
                + data["city"].get<std::string>() + "\n";
            result += "天气: " + data["weather"].get<std::string>() + "\n";
            result += "温度: " + data["temperature"].get<std::string>() + "℃\n";
            result += "风向: " + data["winddirection"].get<std::string>() + "风 "
                + data["windpower"].get<std::string>() + "级\n";;
            result += "湿度: " + data["humidity"].get<std::string>() + "%\n";
            result += "查询时间: " + data["reporttime"].get<std::string>();
            return result;
        }
        return "天气请求失败";
    }
    ~WeatherCommand() override
    {
        return;
    }
};

// AI对话
class AICommand : public Command{
private:
    std::string askAI(std::string& args)
    {
        httplib::SSLClient cli(AI_CLIENT_HOST, AI_CLIENT_PORT);
        json body;
        httplib::Headers headers = {
            {"Authorization", "Bearer " + AI_KEY},
            {"Content-Type", "application/json"}
        };
        body["model"] = AI_MODEL;
        body["messages"] = json::array({
            {
                {"role", "system"},
                {"content", AI_SYS_PROMPTS}
            },
            {
                {"role", "user"},
                {"content", args}
            }
        });
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
            return "网络请求失败";
        }else{
            std::cout << "HTTP状态码: " << res->status << std::endl;
            if(res->status == 200)
            {
                json result = json::parse(res->body);
                // 硅基流动api reponse 结构示例
                // {
                //     "id": "019bdaa55225ef854b320e9b838f77ce",
                //     "object": "chat.completion",
                //     "created": 1768899826,
                //     "model": "Pro/zai-org/GLM-4.7",
                //     "choices": [
                //         {
                //         "index": 0,
                //         "message": {
                //             "role": "assistant",
                //             "content": "你好！...",
                //             "reasoning_content": "..."
                //         },
                //         "finish_reason": "stop"
                //         }
                //     ],
                //     "usage": {
                //         "prompt_tokens": 15,
                //         "completion_tokens": 1540,
                //         "total_tokens": 1555,
                //         "completion_tokens_details": {
                //         "reasoning_tokens": 1190
                //         },
                //         "prompt_tokens_details": {
                //         "cached_tokens": 0
                //         },
                //         "prompt_cache_hit_tokens": 0,
                //         "prompt_cache_miss_tokens": 15
                //     },
                //     "system_fingerprint": ""
                // }
                if (result.contains("choices") && !result["choices"].empty())
                {
                    auto& choices = result["choices"][0];
                    auto& usage = result["usage"];
                    std::string ai_reply = choices["message"]["content"].get<std::string>();
                    std::string total_tokens = std::to_string(usage["total_tokens"].get<int>());
                    std::string reply = "模型: " + result["model"].get<std::string>() 
                        + "\nAI回复内容: " + ai_reply + "\n使用 token: " + total_tokens;
                    return reply;
                }
                return "AI 回复异常";
            }
            std::cerr << json::parse(res->body).dump() << std::endl;
            return "AI 请求异常";
        }
    }
public:
    std::string name() override
    {
        return "AI对话";
    }

    std::string execute(std::string& args) override
    {
        if(args.empty())
        {
            return "请输入对话内容, 例如 AI对话 对话内容";
        }
        return askAI(args);
    }
    ~AICommand() override
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
    virtual std::string handleTask(MessageContext& msgctx) = 0;
    virtual ~BaseTaskManager() = default;
};

///////////////////// 各个任务管理器
////////////
// 被at的文本任务管理器
////////////
class AtTaskManager : public BaseTaskManager{
private: 
    // 维护一个指令表，用于存储多种可支持的指令
    std::unordered_map<std::string, std::unique_ptr<Command>> cmd_map;

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
    // 注册一个用指令接口实现的指令
    void registerCommand(std::unique_ptr<Command> cmd)
    { 
        cmd_map[cmd->name()] = std::move(cmd);
    }

    AtTaskManager()
    {
        // 注册各个指令
        registerCommand(std::make_unique<TimeCommand>());
        registerCommand(std::make_unique<WeatherCommand>());
        registerCommand(std::make_unique<AICommand>());
        registerCommand(std::make_unique<HelpCommand>(getCommandList()));
    }

    // 能否处理
    bool canHandle(const MessageContext& msgctx) override
    {
        // 仅能处理群聊中被at的消息
        return msgctx.pmsgsegs.at_me && (msgctx.msg_type == "group");
    }

    // 处理某个文本指令
    std::string handleTask(MessageContext& msgctx) override
    {
        // 去除可能的前置空格
        while(!msgctx.pmsgsegs.text.empty() && msgctx.pmsgsegs.text[0] == ' ')
        {
            msgctx.pmsgsegs.text.erase(0, 1);
        }
        if(msgctx.pmsgsegs.text.empty()) return "指令格式: @我 指令\n先试试 帮助 吧";
        // 执行指令
        // 先按照空格分割指令名称和参数
        size_t pos = msgctx.pmsgsegs.text.find(' ');
        std::string cmd_name = msgctx.pmsgsegs.text.substr(0, pos);
        std::string cmd_args = (pos == std::string::npos) ? "" : msgctx.pmsgsegs.text.substr(pos + 1);
        if(cmd_map.count(cmd_name))
        {
            // 调用对应文本指令
            return cmd_map[cmd_name]->execute(cmd_args);
        }
        return "未知指令: " + cmd_name;
    }
};

//////////////
// 分享消息任务管理器
//////////////
class JsonTaskManager : public BaseTaskManager{
public:
    bool canHandle(const MessageContext& msgctx) override
    {
        return msgctx.msg_type == "group" && msgctx.pmsgsegs.has_json;
    }
    std::string handleTask(MessageContext& msgctx) override
    {
        return "Json 任务测试";
    }
};

//////////////
// 总的任务管理器
//////////////
class TaskManager{
private:
    std::vector<std::unique_ptr<BaseTaskManager>> tsk_managers;
public:
    void registerTaskManager(std::unique_ptr<BaseTaskManager> tsk_manager)
    {
        tsk_managers.emplace_back(std::move(tsk_manager));
    }
    // 总的任务处理函数
    std::string handleTask(MessageContext& msgctx)
    {
        std::cout << "开始处理任务: " << msgctx.msg_segments.dump() << std::endl;
        for(auto& tsk_manager : tsk_managers)
        {
            if(tsk_manager->canHandle(msgctx))
            {
                // 按照能否处理自动分发任务处理器
                return tsk_manager->handleTask(msgctx);
            }
        }
        return "";
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
    // 创建消息管理器
    MessageManager msg_manager;
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
        std::cout << data.dump(4) << std::endl;
        ////////////////////////////////////////////// 只处理消息事件, 其余事件todo
        if (data["post_type"] != "message")
        {
            res.set_content("{}", "text/plain");
            return;
        }

        // 调用消息管理器 构造 MessageContext
        MessageContext msgctx;
        msgctx = msg_manager.getMessageContext(data);
        // 调用任务管理器 得到回复
        std::string reply_msg = tsk_manager.handleTask(msgctx);
        std::cout << "回复内容: " << reply_msg << std::endl;
        if (!reply_msg.empty())
        {
            if(msgctx.msg_type == "group") msg_manager.send_group_msg(msgctx.group_id, reply_msg);
            if(msgctx.msg_type == "private") msg_manager.send_private_msg(msgctx.user_id, reply_msg);
        }
        res.set_content("{}", "text/plain");
    }
};

int main()
{
    getAllConfigVal();
    Manager m;
    // 创建被at时的文本指令管理器
    m.tsk_manager.registerTaskManager(std::make_unique<AtTaskManager>());
    // 创建分享的json任务管理器
    m.tsk_manager.registerTaskManager(std::make_unique<JsonTaskManager>());
    // ...其他任务类型管理器 todo
    m.start();
    return 0;
}
