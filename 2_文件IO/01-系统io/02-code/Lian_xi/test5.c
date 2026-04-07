#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// 1. 定义双向循环链表节点
typedef struct Node
{
    char path[1024];   // 数据域：保存 .c 文件的完整路径
    struct Node *prev; // 指向前驱节点
    struct Node *next; // 指向后继节点
} Node;

// 初始化链表头节点
Node *init_list()
{
    Node *head = (Node *)malloc(sizeof(Node));
    strcpy(head->path, "HEAD");
    head->prev = head; // 循环指向自己
    head->next = head; // 循环指向自己
    return head;
}

// 尾插法插入新节点
void insert_tail(Node *head, const char *filepath)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    strcpy(new_node->path, filepath);

    Node *tail = head->prev; // 头节点的前驱就是当前的尾节点

    // 更新指针连接
    tail->next = new_node;
    new_node->prev = tail;
    new_node->next = head;
    head->prev = new_node;
}
// 2. 递归查找 .c 文件并入链表
void find_c_files(const char *base_path, Node *head)
{
    DIR *dp = opendir(base_path);
    if (!dp)
        return;

    struct dirent *ep;
    char next_path[1024];
    struct stat st; // 定义 stat 结构体

    // 遍历目录项
    while ((ep = readdir(dp)) != NULL)
    {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;

        snprintf(next_path, sizeof(next_path), "%s/%s", base_path, ep->d_name);

        // 使用 stat 检查文件类型
        if (stat(next_path, &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                find_c_files(next_path, head); // 递归子目录
            }
            else if (S_ISREG(st.st_mode))
            {
                // 检查文件名是否以 .c 结尾
                int len = strlen(ep->d_name);
                if (len >= 2 && ep->d_name[len - 2] == '.' && ep->d_name[len - 1] == 'c')
                {
                    insert_tail(head, next_path); // 是 .c 文件，插入链表
                }
            }
        }
    }
    closedir(dp);
}

// 打印并释放链表
void print_and_free_list(Node *head)
{
    Node *curr = head->next;
    int count = 0;

    while (curr != head)
    { // 没转回头部说明没遍历完
        printf("[%d] %s\n", ++count, curr->path);

        Node *temp = curr;
        curr = curr->next;
        free(temp); // 顺便释放内存
    }
    free(head); // 释放头节点
    printf("共找到 %d 个 .c 文件\n", count);
}

int main()
{
    Node *head = init_list();

    printf("正在搜索当前目录及其子目录下的 .c 文件...\n");
    find_c_files(".", head);

    print_and_free_list(head);

    return 0;
}