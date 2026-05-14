# 医疗科室联网呼号显示屏系统

基于 RK3568 / Linux C / LVGL 开发，通过 TCP 接收护士站远程叫号指令，实时刷新叫号屏界面。

---

## 通信协议

### 传输层

| 项目 | 说明 |
|---|---|
| 协议 | TCP |
| 端口 | 8888 |
| 编码 | UTF-8 |
| 格式 | JSON，每条命令以 `\n` 结尾 |
| 应答 | 每条命令返回一行 JSON |

### 命令列表

#### 1. 叫号 `call`

护士站呼叫一个病人到指定诊室。医生信息根据诊室号自动匹配。

```json
→ {"cmd":"call","number":"012","name":"张三","room":"1诊室"}
← {"status":"ok"}
```

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| cmd | string | 是 | 固定值 `"call"` |
| number | string | 是 | 排队号码，如 `"012"` |
| name | string | 是 | 病人姓名 |
| room | string | 是 | 诊室，`"1诊室"` 或 `"2诊室"` |
| doctor | string | 否 | 医生姓名（不传则自动匹配该诊室当前医生） |
| title | string | 否 | 医生职称（不传则保留原职称） |

收到 `call` 后，叫号屏会：更新叫号大字 → 闪烁动画 → 语音播报 → 从队列移除该病人。

---

#### 2. 更新医生 `doctor`

单独更新某个诊室的医生信息，不影响当前叫号。

```json
→ {"cmd":"doctor","index":0,"name":"赵医生","title":"副主任医师","room":"1诊室"}
← {"status":"ok"}
```

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| cmd | string | 是 | 固定值 `"doctor"` |
| index | int | 是 | 医生索引，0=1诊室，1=2诊室 |
| name | string | 是 | 医生姓名 |
| title | string | 否 | 职称 |
| room | string | 否 | 诊室名称 |

---

#### 3. 推送排队队列 `wait`

整批替换候诊队列。用于护士站统一管理排队列表。

```json
→ {"cmd":"wait","queue":[
      {"number":"013","name":"李四","room":"1诊室"},
      {"number":"014","name":"王五","room":"2诊室"}
    ]}
← {"status":"ok"}
```

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| cmd | string | 是 | 固定值 `"wait"` |
| queue | array | 是 | 病人对象数组 |
| queue[n].number | int | 是 | 排队号码 |
| queue[n].name | string | 是 | 姓名 |
| queue[n].room | string | 是 | 诊室 |

最多 10 人，超出截断。

---

#### 4. 紧急公告 `notice`

弹出紧急公告弹窗。

```json
→ {"cmd":"notice","text":"系统维护中，请稍候","duration":10}
← {"status":"ok"}
```

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| cmd | string | 是 | 固定值 `"notice"` |
| text | string | 是 | 公告文本内容 |
| duration | int | 否 | 显示时长（秒），0=手动关闭 |

---

#### 5. 关闭公告 `clear_notice`

```json
→ {"cmd":"clear_notice"}
← {"status":"ok"}
```

---

#### 6. 心跳检测 `ping`

```json
→ {"cmd":"ping"}
← {"status":"pong"}
```

---

### 错误应答

```json
← {"status":"error","msg":"JSON 格式错误"}
← {"status":"error","msg":"call 命令缺少 name 或 room 字段"}
← {"status":"error","msg":"未知命令"}
```

---

## 护士站测试客户端

`client/nurse_client.c` 是命令行测试工具，自动将简短的文本命令转为上述 JSON 协议发送。

### 编译

```bash
cd client
gcc nurse_client.c -o nurse_client
```

### 使用

```bash
./nurse_client                    # 连接本机 127.0.0.1:8888
./nurse_client 192.168.1.100      # 连接指定 IP
./nurse_client 192.168.1.100 9999 # 指定 IP 和端口
```

### 命令

```
nurse> call 012 张三 1          叫号：号码 姓名 诊室(1或2)
nurse> doctor 1 赵医生          换医生：诊室(1或2) 姓名
nurse> notice 请排队等候        发公告
nurse> clear                   关公告
nurse> ping                    心跳
nurse> help                    帮助
nurse> quit                    退出
```

`call` 和 `doctor` 的诊室写 `1` 或 `2` 即可，自动拼成 `1诊室`/`2诊室`。

---

## UDP 设备发现

护士站可通过 UDP 广播自动发现叫号屏，无需手动输入 IP。

| 项目 | 说明 |
|---|---|
| 协议 | UDP |
| 端口 | 9999 |
| 广播内容 | `WHO_IS_CALL_SCREEN` |
| 应答内容 | `CALL_SCREEN,room=1,port=8888` |

```bash
# 用 netcat 测试
echo "WHO_IS_CALL_SCREEN" | nc -u -b 255.255.255.255 9999
# 收到：CALL_SCREEN,room=1,port=8888
```

---

## 编译运行

### 主程序（叫号屏）

```bash
cd ubuntu_demo
rm -rf build && mkdir build && cd build
cmake ..
make
cd ..
./bin/main
```

### 依赖

- CMake >= 3.10
- GCC (C99)
- LVGL 8.x (源码包含在 `lvgl/`)
- FreeType 2.13.3 (源码包含在 `freetype-2.13.3/`)
- SDL2 (PC 调试) 或 Linux Framebuffer (RK3568 部署)
- pthread

---

## 项目结构

```
ubuntu_demo/
├── main.c                     # 入口：初始化+启动网络+LVGL主循环
├── CMakeLists.txt             # 构建
├── lv_conf.h                  # LVGL 配置
├── mouse_cursor_icon.c        # SDL 鼠标光标
├── STSONG.TTF / STLITI.TTF   # 中文字体
│
├── ui/                        # UI 层
│   ├── app_ui.c/.h           #   登录页 + 主菜单
│   ├── medical.c/.h          #   医疗叫号屏 + poll_timer 轮询
│   └── ui_font.c/.h          #   FreeType 中文字体加载
│
├── net/                       # 网络层
│   ├── net_server.c/.h       #   TCP 服务端 (select多路复用+粘包处理)
│   ├── protocol.c/.h         #   JSON 命令解析 → app_state 写入
│   ├── udp_discover.c/.h     #   UDP 设备发现
│   └── cJSON.c/.h            #   JSON 解析库
│
├── core/                      # 数据层
│   ├── app_state.c/.h        #   全局共享状态 + pthread_mutex_t
│   ├── user_data.c/.h        #   用户登录/注册/排行榜
│   └── logger.c/.h           #   日志记录 (call_screen.log)
│
├── client/                     # 测试工具
│   ├── nurse_client.c        #   护士站命令行客户端
│   └── CMakeLists.txt        #   独立编译
│
├── other/                     # 暂不参与编译
│   ├── 2048.c/.h             #   2048 小游戏
│   ├── album.c/.h            #   电子相册
│   └── player.c/.h           #   视频播放器
│
├── lvgl/                      # LVGL 图形库
├── freetype-2.13.3/          # FreeType 字体引擎
└── bin/main                   # 编译输出
```

---

## 数据流架构

```
护士站 (nurse_client)               叫号屏 (main)
      │                                  │
      │  TCP {"cmd":"call",...}          │
      ├─────────────────────────────────→│
      │                                  ├→ net_server 线程
      │                                  │   ├→ 粘包处理(按\n分行)
      │                                  │   └→ protocol_parse()
      │                                  │       ├→ cJSON 解析
      │                                  │       ├→ app_state_set_xxx()
      │                                  │       └→ logger_write()
      │  ← {"status":"ok"}               │
      │                                  │
      │                                  ├→ poll_timer (200ms)
      │                                  │   ├→ app_state_has_new_call()
      │                                  │   ├→ update_current_call()
      │                                  │   ├→ start_blink()
      │                                  │   └→ tts_speak()
      │                                  │
      │                                  └→ lv_timer_handler() 渲染
```

**关键规则**：网络线程不直接操作 LVGL 控件。网络线程只写 `app_state` 并置标志位，LVGL 的 `poll_timer` 每 200ms 检测标志位，在主线程中刷新界面。

---

## 日志

程序运行后自动生成 `call_screen.log`：

```
[2026-05-06 20:10:20] CONNECT 客户端连接 IP=127.0.0.1 Port=54321
[2026-05-06 20:10:22] CALL number=12 name=张三 room=1诊室 doctor=王医生
[2026-05-06 20:10:23] NOTICE text=系统维护中，请稍候 duration=10
[2026-05-06 20:15:00] CONNECT 客户端断开 fd=5
```

```bash
tail -f call_screen.log   # 实时查看
```
