#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

// 文件拷贝信息结构体
struct cp_file
{
    char src[2048];  // 源文件路径
    char dest[2048]; // 目标文件路径
};

// 线程函数：拷贝单个文件
void *cp(void *arg)
{
    struct cp_file file = *(struct cp_file *)arg;

    // 以只读方式打开源文件
    int fd_src = open(file.src, O_RDONLY);
    if (fd_src < 0)
    {
        perror("open src");
        pthread_exit(NULL);
    }

    // 以只写方式打开目标文件，不存在则创建
    int fd_dest = open(file.dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dest < 0)
    {
        perror("open dest");
        close(fd_src);
        pthread_exit(NULL);
    }

    // 循环读取源文件并写入目标文件
    char buf[1024];
    int ret;
    while ((ret = read(fd_src, buf, sizeof(buf))) > 0)
    {
        write(fd_dest, buf, ret);
    }

    // 关闭文件描述符
    close(fd_src);
    close(fd_dest);

    pthread_exit(NULL);
}

// 递归拷贝目录函数
void cp_dir(char *src_dir, char *dest_dir)
{
    // 打开源目录
    DIR *dir = opendir(src_dir);
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    char src_path[2048];
    char dest_path[2048];
    char new_dest_dir[2048];

    // 获取源目录的名字（去掉路径）
    char *dir_name = basename(src_dir);
    // 在目标目录下创建同名子目录
    snprintf(new_dest_dir, sizeof(new_dest_dir), "%s/%s", dest_dir, dir_name);

    // 创建目标子目录
    if (mkdir(new_dest_dir, 0755) < 0)
    {
        perror("mkdir");
    }

    // 读取源目录的所有目录项
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // 跳过 . 和 .. 目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        
        // 拼接源文件的完整路径
        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);

        // 获取文件属性
        struct stat st;
        if (stat(src_path, &st) < 0)
        {
            perror("stat");
            continue;
        }

        // 如果是目录，递归调用
        if (S_ISDIR(st.st_mode))
        {
            cp_dir(src_path, new_dest_dir);
        }
        // 如果是普通文件，创建线程拷贝
        else if (S_ISREG(st.st_mode))
        {
            // 拼接目标文件的完整路径
            int len = snprintf(dest_path, sizeof(dest_path), "%s/%s", new_dest_dir, entry->d_name);
            if (len >= sizeof(dest_path))
            {
                fprintf(stderr, "路径过长，跳过: %s/%s\n", new_dest_dir, entry->d_name);
                continue;
            }

            // 准备文件拷贝信息
            struct cp_file cp_filename;
            strcpy(cp_filename.src, src_path);
            strcpy(cp_filename.dest, dest_path);

            // 创建线程执行文件拷贝
            pthread_t id;
            pthread_create(&id, NULL, cp, (void *)&cp_filename);
            // 等待线程完成
            pthread_join(id, NULL);
        }
    }

    // 关闭目录
    closedir(dir);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage:%s <src_dir> <dest_dir>\n", argv[0]);
        return -1;
    }

    cp_dir(argv[1], argv[2]);

    return 0;
}
