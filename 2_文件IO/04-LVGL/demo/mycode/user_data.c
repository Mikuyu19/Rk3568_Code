#include "user_data.h"

#include <stdio.h>
#include <string.h>

// 宏定义：用户数据存储的本地绝对路径 (焊死在 Ubuntu 共享目录)
#define USER_DATA_FILE "/yu/2048_project/user_data.txt"

/* 内部用户数据结构体，包含密码信息 */
typedef struct
{
    char username[USER_NAME_LEN];
    char password[USER_PWD_LEN];
    int best_score;
} user_item_t;

static user_item_t g_users[USER_MAX_COUNT]; // 在内存中缓存所有用户数据的数组
static int g_user_count;                    // 记录当前系统中有多少个注册用户
static int g_current_index = -1;            // 记录当前登录用户在数组中的下标，-1 表示未登录

/* 清空内存中的用户数据 */
static void clear_users(void)
{
    memset(g_users, 0, sizeof(g_users)); // 将数组内存全部置 0
    g_user_count = 0;
}

/* 根据用户名查找用户，返回该用户在数组中的下标，找不到返回 -1 */
static int find_user(const char *username)
{
    for (int i = 0; i < g_user_count; i++)
    {
        if (strcmp(g_users[i].username, username) == 0)
        { // 字符串完全匹配
            return i;
        }
    }
    return -1;
}

/* 【核心持久化】将内存中的用户数据同步写入本地 TXT 文件 */
static void save_users(void)
{
    // "w" 模式：打开文件用于写入，如果文件存在则清空内容重新写，不存在则创建
    FILE *fp = fopen(USER_DATA_FILE, "w");
    if (fp == NULL)
    {
        return; // 文件打开失败（可能是路径错误或权限不足），直接返回
    }

    // 遍历当前所有用户，格式化写入文件 (格式: 用户名,密码,最高分)
    for (int i = 0; i < g_user_count; i++)
    {
        fprintf(
            fp,
            "%s,%s,%d\n", // 逗号分隔，末尾带换行符
            g_users[i].username,
            g_users[i].password,
            g_users[i].best_score);
    }

    fclose(fp); // 记得关闭文件流释放资源
}

/* 确保系统至少有一个 admin 超级管理员账号，防止文本为空时无法登录 */
static void ensure_default_admin(void)
{
    if (find_user("admin") >= 0)
    {
        return; // 如果 admin 已经存在，则什么也不做
    }

    if (g_user_count >= USER_MAX_COUNT)
    {
        return; // 用户满了，无法再添加
    }

    // 写入默认管理员信息
    strcpy(g_users[g_user_count].username, "admin");
    strcpy(g_users[g_user_count].password, "123456");
    g_users[g_user_count].best_score = 0;
    g_user_count++;
}

/* 系统初始化：从文本中读取历史用户数据 */
void user_data_init(void)
{
    FILE *fp;
    char line[128]; // 用于存储每次读取的一行字符串
    char name[USER_NAME_LEN];
    char pwd[USER_PWD_LEN];
    int best;

    clear_users(); // 先清空内存

    // "r" 模式：以只读方式打开文件
    fp = fopen(USER_DATA_FILE, "r");
    if (fp != NULL)
    {
        // 循环按行读取文件内容，直到文件末尾 (EOF)
        while (fgets(line, sizeof(line), fp) != NULL)
        {
            if (g_user_count >= USER_MAX_COUNT)
            {
                break; // 如果文件里的用户数超过了系统上限，停止读取
            }

            /* sscanf 的正则魔法：
             * %31[^,] 表示读取最多 31 个字符，遇到逗号停止 (31留出位置给 \0)
             * 这段代码的作用是把 "name,pwd,score\n" 拆解出来
             */
            if (sscanf(line, "%31[^,],%31[^,],%d", name, pwd, &best) == 3)
            {
                strcpy(g_users[g_user_count].username, name);
                strcpy(g_users[g_user_count].password, pwd);
                g_users[g_user_count].best_score = best;
                g_user_count++;
            }
        }
        fclose(fp);
    }

    ensure_default_admin(); // 检查兜底账号
    save_users();           // 同步保存一次，确保刚生成的默认账号写进文件
    g_current_index = -1;   // 初始化状态为未登录
}

/* 验证登录 */
int user_login(const char *username, const char *password)
{
    int index = find_user(username);

    if (index < 0)
    {
        return 0; // 找不到账号
    }

    if (strcmp(g_users[index].password, password) != 0)
    {
        return 0; // 密码对不上
    }

    g_current_index = index; // 记录当前登录用户的下标
    return 1;                // 登录成功
}

/* 注册新用户 */
int user_register(const char *username, const char *password)
{
    // 非法输入拦截
    if (username == NULL || password == NULL)
        return 0;
    if (username[0] == '\0' || password[0] == '\0')
        return 0;
    if (strlen(username) >= USER_NAME_LEN || strlen(password) >= USER_PWD_LEN)
        return 0;
    if (find_user(username) >= 0)
        return 0; // 用户名已被占用
    if (g_user_count >= USER_MAX_COUNT)
        return 0; // 容量已满

    // 分配新空间写入数据
    strcpy(g_users[g_user_count].username, username);
    strcpy(g_users[g_user_count].password, password);
    g_users[g_user_count].best_score = 0;
    g_user_count++;

    save_users(); // 【关键】注册成功后立刻同步到文件，防止断电丢失
    return 1;
}

/* 登出账号 */
void user_logout(void)
{
    g_current_index = -1; // 切断登录状态
}

/* 获取当前登录用户名 */
const char *user_get_current_name(void)
{
    if (g_current_index < 0 || g_current_index >= g_user_count)
        return "";
    return g_users[g_current_index].username;
}

/* 获取当前用户的历史最高分 */
int user_get_current_best(void)
{
    if (g_current_index < 0 || g_current_index >= g_user_count)
        return 0;
    return g_users[g_current_index].best_score;
}

/* 游戏结算时调用：如果分数破纪录，则更新并写入文件 */
void user_update_current_best(int score)
{
    if (g_current_index < 0 || g_current_index >= g_user_count)
        return;

    if (score > g_users[g_current_index].best_score)
    {
        g_users[g_current_index].best_score = score;
        save_users(); // 破纪录了，写盘保存
    }
}

/* 【核心算法】获取排行榜：拷贝数据并进行降序排序 */
int user_get_rank_list(rank_item_t *list, int max_count)
{
    rank_item_t temp[USER_MAX_COUNT]; // 创建一个临时数组用来排序
    int count = g_user_count;

    if (count > max_count)
    {
        count = max_count; // 防止外部传入的数组太小装不下
    }

    // 1. 将数据从内部结构体剥离到对外的排行榜结构体中
    for (int i = 0; i < count; i++)
    {
        strcpy(temp[i].username, g_users[i].username);
        temp[i].best_score = g_users[i].best_score;
    }

    // 2. 经典的冒泡排序 (降序：分数高的排前面)
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - 1 - i; j++)
        {
            if (temp[j].best_score < temp[j + 1].best_score)
            {
                // 交换位置
                rank_item_t t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }

    // 3. 将排好序的数据拷贝给外部传入的指针数组
    for (int i = 0; i < count; i++)
    {
        list[i] = temp[i];
    }

    return count; // 返回实际排名的总人数
}