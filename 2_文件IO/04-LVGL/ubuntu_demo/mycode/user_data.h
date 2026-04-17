#ifndef MYCODE_USER_DATA_H
#define MYCODE_USER_DATA_H

// 定义系统中相关字符串的最大长度和最大用户数
#define USER_NAME_LEN 32  // 用户名最大长度
#define USER_PWD_LEN 32   // 密码最大长度
#define USER_MAX_COUNT 64 // 系统最多容纳 64 个用户

/* 排行榜数据结构体
 * 仅向 UI 层暴露用户名和最高分，隐藏密码信息以保证安全
 */
typedef struct
{
    char username[USER_NAME_LEN];
    int best_score;
} rank_item_t;

// 提供给外界调用的 API 接口声明
void user_data_init(void);                                     // 初始化用户系统（读取文件）
int user_login(const char *username, const char *password);    // 用户登录
int user_register(const char *username, const char *password); // 用户注册
void user_logout(void);                                        // 用户登出
const char *user_get_current_name(void);                       // 获取当前登录的用户名
int user_get_current_best(void);                               // 获取当前用户的最高分
void user_update_current_best(int score);                      // 更新当前用户的最高分
int user_get_rank_list(rank_item_t *list, int max_count);      // 获取经过排序的排行榜列表

#endif