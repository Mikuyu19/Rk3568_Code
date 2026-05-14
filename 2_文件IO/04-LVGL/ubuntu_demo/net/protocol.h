#ifndef PROTOCOL_H
#define PROTOCOL_H

// ========================
// 协议解析模块
// ========================
//
// 负责：
//   1. 解析护士站发来的 JSON 命令
//   2. 调用 app_state_set_xxx() 更新共享状态
//   3. 返回 JSON 格式的应答给 net_server 发送
//
// 用法：
//   net_server 收到完整一行 → protocol_parse() → 拿到应答字符串
//   → send() 发回客户端 → free() 释放应答
//
// 支持的 JSON 命令：
//   {"cmd":"call",   "number":"012","name":"张三","room":"1诊室",...}
//   {"cmd":"wait",   "queue":[{...},...]}
//   {"cmd":"doctor", "index":0,"name":"王医生",...}
//   {"cmd":"notice", "text":"...","duration":10}
//   {"cmd":"clear_notice"}
//   {"cmd":"ping"}

// 解析一行命令，返回 malloc 的应答字符串（调用方负责 free）
// 返回 NULL 表示不需要回复
char *protocol_parse(const char *line);

#endif
