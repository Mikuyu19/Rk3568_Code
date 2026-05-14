// ========================
// protocol.c —— JSON 协议解析
// ========================
//
// 本模块解析护士站发来的 JSON 命令，调用 app_state 更新共享状态，
// 并生成 JSON 应答让 net_server 回传给客户端。
//
// 命令路由流程：
//   JSON 字符串 → cJSON_Parse()
//              → 提取 "cmd" 字段
//              → cmd == "call"   → handle_call()
//              → cmd == "wait"   → handle_wait()
//              → cmd == "doctor" → handle_doctor()
//              → cmd == "notice" → handle_notice()
//              → cmd == "clear_notice" → handle_clear_notice()
//              → cmd == "ping"   → handle_ping()
//              → 未知命令        → 返回 {"status":"error","msg":"unknown cmd"}

#include "protocol.h"
#include "cJSON.h"
#include "../core/app_state.h"
#include "../core/logger.h" // 日志记录

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ======================== 工具函数 ========================

// 生成成功应答：{"status":"ok"}
static char *make_ok(void)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "status", "ok");
    char *raw = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return raw;
}

// 生成错误应答：{"status":"error","msg":"..."}
static char *make_error(const char *msg)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "status", "error");
    cJSON_AddStringToObject(obj, "msg", msg);
    char *raw = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return raw;
}

// 安全取出 JSON 对象中的字符串字段，字段不存在返回 ""
static const char *json_get_str(cJSON *obj, const char *key)
{
    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (item && item->type == cJSON_String)
    {
        return item->valuestring;
    }
    return "";
}

// 安全取出 JSON 对象中的整数字段，字段不存在返回 0
// 同时兼容数字类型和字符串类型（如 "013"）
static int json_get_int(cJSON *obj, const char *key)
{
    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (!item) return 0;

    if (item->type == cJSON_Number) {
        return item->valueint;
    }
    if (item->type == cJSON_String) {
        return atoi(item->valuestring);
    }
    return 0;
}

// ======================== 命令处理函数 ========================

// @call 命令示例：
// {"cmd":"call","number":"012","name":"张三","room":"1诊室",
//  "doctor":"王医生","title":"主任医师"}
static char *handle_call(cJSON *root)
{
    const char *number_str = json_get_str(root, "number");
    const char *name = json_get_str(root, "name");
    const char *room = json_get_str(root, "room");
    const char *doctor = json_get_str(root, "doctor");
    const char *title = json_get_str(root, "title");

    if (name[0] == '\0' || room[0] == '\0')
    {
        return make_error("call 命令缺少 name 或 room 字段");
    }

    int number = atoi(number_str);
    if (number <= 0)
    {
        // 没有传号码就自动分配
        number = app_state_get_next_number();
    }

    // 写入共享状态 → LVGL 定时器检测到标志后会刷新界面
    app_state_set_call(number, name, room, doctor, title);

    logger_write(LOG_CALL, "number=%d name=%s room=%s doctor=%s",
                 number, name, room, doctor[0] ? doctor : "(未指定)");

    return make_ok();
}

// @wait 命令示例：
// {"cmd":"wait","queue":[
//   {"number":"013","name":"李四","room":"1诊室"},
//   {"number":"014","name":"王五","room":"2诊室"}
// ]}
static char *handle_wait(cJSON *root)
{
    cJSON *queue_array = cJSON_GetObjectItem(root, "queue");
    if (!queue_array || queue_array->type != cJSON_Array)
    {
        return make_error("wait 命令缺少 queue 数组");
    }

    patient_info_t queue[QUEUE_MAX_COUNT];
    int count = cJSON_GetArraySize(queue_array);
    if (count > QUEUE_MAX_COUNT)
    {
        count = QUEUE_MAX_COUNT; // 超过容量就截断
    }

    for (int i = 0; i < count; i++)
    {
        cJSON *item = cJSON_GetArrayItem(queue_array, i);
        if (!item)
            continue;

        queue[i].number = json_get_int(item, "number");
        snprintf(queue[i].name, sizeof(queue[i].name), "%s",
                 json_get_str(item, "name"));
        snprintf(queue[i].room, sizeof(queue[i].room), "%s",
                 json_get_str(item, "room"));
    }

    app_state_set_queue(queue, count);

    logger_write(LOG_CALL, "WAIT 队列更新 count=%d", count);

    return make_ok();
}

// @doctor 命令示例：
// {"cmd":"doctor","index":0,"name":"李医生","title":"副主任医师","room":"2诊室"}
static char *handle_doctor(cJSON *root)
{
    int index = json_get_int(root, "index");
    const char *name = json_get_str(root, "name");
    const char *title = json_get_str(root, "title");
    const char *room = json_get_str(root, "room");

    if (index < 0 || index > 1)
    {
        return make_error("doctor index 必须是 0 或 1");
    }

    app_state_set_doctor(index, name[0] ? name : NULL,
                         title[0] ? title : NULL,
                         room[0] ? room : NULL);

    printf("[protocol] DOCTOR index=%d name=%s title=%s room=%s\n",
           index, name, title, room);

    return make_ok();
}

// @notice 命令示例：
// {"cmd":"notice","text":"系统维护中，请稍候...","duration":10}
static char *handle_notice(cJSON *root)
{
    const char *text = json_get_str(root, "text");
    int duration = json_get_int(root, "duration");

    if (text[0] == '\0')
    {
        return make_error("notice 命令缺少 text 字段");
    }

    app_state_set_notice(text, duration);

    logger_write(LOG_NOTICE, "text=%s duration=%d", text, duration);

    return make_ok();
}

// @clear_notice 命令示例：
// {"cmd":"clear_notice"}
static char *handle_clear_notice(cJSON *root)
{
    (void)root; // 不需要参数
    app_state_clear_notice();

    printf("[protocol] CLEAR_NOTICE\n");

    return make_ok();
}

// @ping 命令示例：
// {"cmd":"ping"}
// 应答：{"status":"pong"}
static char *handle_ping(cJSON *root)
{
    (void)root;

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "status", "pong");
    char *raw = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);

    printf("[protocol] PING → pong\n");

    return raw;
}

// ======================== 公开入口 ========================

char *protocol_parse(const char *line)
{
    if (line == NULL || line[0] == '\0')
    {
        return NULL;
    }

    // 1. 解析 JSON
    cJSON *root = cJSON_Parse(line);
    if (root == NULL)
    {
        // JSON 解析失败
        logger_write(LOG_ERROR, "JSON 解析失败: %s", line);
        return make_error("JSON 格式错误");
    }

    // 2. 提取 "cmd" 字段
    cJSON *cmd_item = cJSON_GetObjectItem(root, "cmd");
    if (!cmd_item || cmd_item->type != cJSON_String)
    {
        cJSON_Delete(root);
        return make_error("缺少 cmd 字段");
    }

    const char *cmd = cmd_item->valuestring;

    // 3. 按 cmd 路由到对应处理函数
    char *response = NULL;

    if (strcmp(cmd, "call") == 0)
    {
        response = handle_call(root);
    }
    else if (strcmp(cmd, "wait") == 0)
    {
        response = handle_wait(root);
    }
    else if (strcmp(cmd, "doctor") == 0)
    {
        response = handle_doctor(root);
    }
    else if (strcmp(cmd, "notice") == 0)
    {
        response = handle_notice(root);
    }
    else if (strcmp(cmd, "clear_notice") == 0)
    {
        response = handle_clear_notice(root);
    }
    else if (strcmp(cmd, "ping") == 0)
    {
        response = handle_ping(root);
    }
    else
    {
        // 未知命令
        logger_write(LOG_ERROR, "未知命令: %s", cmd);
        response = make_error("未知命令");
    }

    // 4. 释放 JSON 树，返回应答字符串
    cJSON_Delete(root);
    return response;
}
