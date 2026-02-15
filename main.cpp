#include <bits/stdc++.h>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

// config
std::string BOT_QQ;
std::string SERVER_HOST;
int SERVER_PORT;
std::string SERVER_ACCESS_TOKEN;
std::string CLIENT_HOST;
int CLIENT_PORT;
std::string CLIENT_ACCESS_TOKEN;

std::string AMAP_KEY;
std::string AMAP_CLIENT_HOST;
int AMAP_CLIENT_PORT;
std::string AMAP_GET_PATH;

// 用于获取 config 信息
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
};

void getAllConfigVal()
{
    Config cfg;
    BOT_QQ = cfg.getBotqq();
    SERVER_HOST = cfg.getServerHost();
    SERVER_PORT = cfg.getServerPort();
    SERVER_ACCESS_TOKEN = cfg.getServerToken();
    CLIENT_HOST = cfg.getClientHost();
    CLIENT_PORT = cfg.getClientPort();
    CLIENT_ACCESS_TOKEN = cfg.getClientToken();
    AMAP_CLIENT_HOST = cfg.getAmapHost();
    AMAP_CLIENT_PORT = cfg.getAmapPort();
    AMAP_KEY = cfg.getAmapKey();
    AMAP_GET_PATH = cfg.getAmapGetPath();
}

// 消息结构
struct MessageContext{
    size_t group_id;
    size_t user_id;
    std::string msg_type;
    std::string raw_msg;
};

//////////
// 指令接口
//////////
class Command{
public:
    virtual std::string name() = 0;
    virtual std::string execute(std::string& args) = 0;
    virtual ~Command() = default;
};

////////////
// 指令管理器
////////////
class CommandManager{
private: 
    // 维护一个指令表，用于存储多种可支持的指令
    std::unordered_map<std::string, std::unique_ptr<Command>> cmd_map;

public:
    // 用于获取指令列表
    std::string getCommandList()
    {
        std::string cmd_list = "";
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

    // 处理某个指令
    std::string handleCommand(MessageContext& msgctx)
    {
        std::cout << "开始处理指令: " << msgctx.raw_msg << std::endl;
        if(msgctx.msg_type == "private")
        {
            return "暂不支持私聊功能";
        }else if(msgctx.msg_type == "group"){
            // 先去除艾特的代码, 只处理被 at 的消息 //////////////////////////////////// 其他消息todo
            std::string at_code = "[CQ:at,qq=" + BOT_QQ + "]";
            size_t pos = msgctx.raw_msg.find(at_code);
            if(pos != std::string::npos)
            {
                msgctx.raw_msg.erase(pos, at_code.length());
            }else{ /// 只处理被@的消息
                return "";
            }
            // 去除可能的前置空格
            while(!msgctx.raw_msg.empty() && msgctx.raw_msg[0] == ' ')
            {
                msgctx.raw_msg.erase(0, 1);
            }
            if(msgctx.raw_msg.empty()) return "指令格式: @我 指令\n先试试 help 吧";
            // 执行指令
            // 先按照空格分割指令名称和参数
            pos = msgctx.raw_msg.find(' ');
            std::string cmd_name = msgctx.raw_msg.substr(0, pos);
            std::string cmd_args = (pos == std::string::npos) ? "" : msgctx.raw_msg.substr(pos + 1);
            if(cmd_map.count(cmd_name))
            {
                return cmd_map[cmd_name]->execute(cmd_args);
            }else{
                return "未知指令: " + cmd_name;
            }
        }
        return "";
    }

};

////////////
// 消息管理器
////////////
class MessageManager{
private:
    httplib::Server svr;
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
public:
    // 创建指令管理器
    CommandManager cmd_manager;

    MessageManager()
    {
        // 注册 POST 路由
        svr.Post("/", [this](const httplib::Request& req, httplib::Response& res){
            this->handle_post(req, res);
        });
    }

    // 启动服务器
    void start()
    {
        svr.listen(CLIENT_HOST, CLIENT_PORT);
    }

    // 处理 POST 请求
    void handle_post(const httplib::Request& req, httplib::Response& res)
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

        // 构造 MessageContext
        MessageContext msgctx;
        msgctx.msg_type = data["message_type"];
        if (msgctx.msg_type == "group") msgctx.group_id = data["group_id"]; // 私聊没有 group_id
        msgctx.user_id = data["user_id"];
        msgctx.raw_msg = data["raw_message"];

        // 调用指令管理器
        std::string reply_msg = cmd_manager.handleCommand(msgctx);
        std::cout << "回复内容: " << reply_msg << std::endl;
        if (!reply_msg.empty())
        {
            if(msgctx.msg_type == "group") send_group_msg(msgctx.group_id, reply_msg);
            if(msgctx.msg_type == "private") send_private_msg(msgctx.user_id, reply_msg);
        }
        res.set_content("{}", "text/plain");
    }
};

///////////
// 各个指令
///////////
// help
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
        return "help";
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
// time
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
        return "time";
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
        httplib::Client cli(AMAP_CLIENT_HOST, AMAP_CLIENT_PORT);
        auto res = cli.Get(AMAP_GET_PATH + "?city=" + city + "&key=" + AMAP_KEY);
        if (res && res->status == 200)
        {
            // 先解析返回的JSON
            json raw_data = json::parse(res->body);
            if(raw_data["lives"].empty())
            {
                return "天气查询失败, 请输入正确的格式: 天气 城市名称\n城市名称不要带有除空格外的其他字符\n暂时只支持国内城市";
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
        }else{
            return "天气请求失败";
        }
    }
    ~WeatherCommand() override
    {
        return;
    }
};


int main()
{
    getAllConfigVal();
    MessageManager m;
    m.cmd_manager.registerCommand(std::make_unique<TimeCommand>());
    m.cmd_manager.registerCommand(std::make_unique<WeatherCommand>());
    m.cmd_manager.registerCommand(std::make_unique<HelpCommand>(m.cmd_manager.getCommandList()));
    m.start();
    return 0;
}
// g++ main.cpp -o bot.exe -Iinclude -lws2_32 -lpthread