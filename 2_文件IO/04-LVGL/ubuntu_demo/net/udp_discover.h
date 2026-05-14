#ifndef UDP_DISCOVER_H
#define UDP_DISCOVER_H

// ========================
// UDP 设备发现模块
// ========================
//
// 独立线程监听 UDP 9999 端口广播。
// 护士站广播 "WHO_IS_CALL_SCREEN" → 本模块回复设备信息。
//
// 用法：
//   udp_discover_start()  → 启动广播监听
//   udp_discover_stop()   → 关闭

// 启动 UDP 发现线程，失败返回 -1
int udp_discover_start(void);

// 停止 UDP 发现
void udp_discover_stop(void);

#endif
