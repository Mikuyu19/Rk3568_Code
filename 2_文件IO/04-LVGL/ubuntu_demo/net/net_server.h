#ifndef NET_SERVER_H
#define NET_SERVER_H

// ========================
// TCP 服务端模块
// ========================
//
// 功能：
//   1. 在独立线程中监听 8888 端口
//   2. 使用 select() 支持多客户端并发
//   3. 按 '\n' 分行处理粘包
//   4. 每收到完整一行 → 交给 protocol_parse() 解析
//
// 用法：
//   main.c 中调用 net_server_start() 启动
//   程序退出前调用 net_server_stop() 关闭

// 启动 TCP 服务端线程，失败返回 -1
int net_server_start(void);

// 停止服务端，关闭所有连接
void net_server_stop(void);

#endif
