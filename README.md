## 基于 NapCatQQ 搭建的简易 qq bot
使用 Cpp 编写的 qq 机器人, 实现了一些文本指令响应、`Json`卡片分享解析和AI对话功能。

### 环境说明
- Ubuntu 24.04  

- NapCat v4.17.29

- OpenSSL

  ```shell
  $ openssl version
  OpenSSL 3.0.13 30 Jan 2024 (Library: OpenSSL 3.0.13 30 Jan 2024)
  ```

- 编译器

  ```shell
  $ g++ --version
  g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
  Copyright (C) 2023 Free Software Foundation, Inc.
  This is free software; see the source for copying conditions.  There is NO
  warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ```

- 编译命令

  ```shell
  g++ src/main.cpp -O2 -Iinclude -o bot -lssl -lcrypto -lpthread
  ```

### 主要特性

- 使用 cpp-httplib 库，OneBot HTTP Server
- 模块化任务架构，易扩展指令系统
- 支持 @ 指令: 天气查询、随机二次元图片等
- 自动解析 QQ 分享的 B站视频并下载转发
- 本地缓存 + 流式下载 + 完整日志机制
- AI 对话: 每个用户单独对话历史留存
- 线程安全

### 部署

参照 [NapCatQQ 使用文档](https://napneko.github.io/)

收发消息格式配置为 `array`

