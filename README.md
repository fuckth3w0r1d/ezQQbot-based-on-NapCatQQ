## 基于 NapCatQQ 搭建的简易 qq bot
使用 Cpp 编写的 qq 机器人, 实现了一些文本指令响应和`Json`卡片分享的解析功能。~~更多功能后续再写~~ 

### 环境说明
- Windows 11  

- NapCat v4.17.11

- OpenSSL

  ```shell
  > openssl version
  OpenSSL 3.6.1 27 Jan 2026 (Library: OpenSSL 3.6.1 27 Jan 2026)
  ```

- 编译器

  ```shell
  > g++ --version
  g++.exe (GCC) 13.2.0
  Copyright (C) 2023 Free Software Foundation, Inc.
  This is free software; see the source for copying conditions.  There is NO
  warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ```

- 编译命令

  ```shell
  g++ src/main.cpp -O2 -Iinclude -o bot.exe -lssl -lcrypto -lcrypt32 -lws2_32 -lpthread
  ```

### 主要特性

- 使用 cpp-httplib 库，OneBot HTTP Server
- 模块化任务架构，易扩展指令系统
- 支持 @ 指令、天气查询、AI 对话
- 自动解析 QQ 分享的 B站视频并下载转发
- 本地缓存 + 流式下载 + 完整日志机制
- 线程安全

### 部署

参照 [NapCatQQ 使用文档](https://napneko.github.io/)

收发消息格式配置为 `array`

