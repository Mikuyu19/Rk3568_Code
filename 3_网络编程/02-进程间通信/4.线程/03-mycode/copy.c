
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

void *cp(void *arg)
{
    struct cp_file cp_info = *(struct cp_file *)arg;

    // 用只读的方式打开源文件
    int fd_src = open(cp_info.src, O_RDONLY);
    if (fd_src < 0)
    {
        perror("open src");
        pthread_exit(NULL);
    }

    // 用只写的方式打开目的文件
    int fd_dest = open(cp_info.dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dest < 0)
    {
        perror("open dest");
        close(fd_src);
        pthread_exit(NULL);
    }

    // 循环读取源文件的数据写入到目的文件
    char buf[4096];
    while (1)
    {
        int ret = read(fd_src, buf, sizeof(buf));
        if (ret == 0)
            break;
        if (ret < 0)
        {
            perror("read");
            break;
        }
        write(fd_dest, buf, ret);
    }

    // 关闭两个文件
    close(fd_src);
    close(fd_dest);

    pthread_exit(NULL);
}

/*
    把src_dir拷贝到dest_dir目录下
    src_dir:源目录路径
    dest_dir：目的目录路径
*/
void cp_dir(char *src_dir, char *dest_dir)
{
    /*打开源目录(从哪个目录拷贝)*/
    DIR *dir = opendir(src_dir);
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }
    /*分别获取源目录和目的目录的绝对路径(后面接你需要拼接带路径的文件名)*/
    // 保存当前目录的工作路径
    char src_path[1024];
    // 保存要拷贝的源目录的工作路径
    char dest_path[1024];
    // 保存要拷贝的目的目录的工作路径
    char new_dest_dir[1024];

    /*让源目录成为目的目录的子目录名，获取这个子目录的名字*/
    char *dir_name = basename(src_dir);
    /*获取这个子目录的名字与目的目录进行拼接，并创建这个目录*/
    snprintf(new_dest_dir, sizeof(new_dest_dir), "%s%s", dest_dir, dir_name);
    /*更新目的目录*/
    if (mkdir(new_dest_dir, 0777) < 0)
    {
        perror("mkdir");
    }
    /*读取源目录的目录项*/
    struct dirent *p;
    while ((p = readdir(dir)) != NULL)
    {
        // 先跳过两个隐藏目录. ..
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
        {
            continue;
        }
        // 拼接源路径
        snprintf(src_path, sizeof(src_path), "%s%s", src_dir, p->d_name);
        // 获取文件属性
        struct stat st;
        if (stat(src_path, &st) < 0)
        {
            perror("stat");
            continue;
        }
        // 如果是目录
        if (S_ISDIR(st.st_mode))
        {
            cp_dir(src_path, dest_path);
        }
        // 如果是普通文件
        else if (S_ISREG(st.st_mode))
        {

            // 拼接目标路径
            snprintf(dest_path, sizeof(dest_path), "%s%s", dest_dir, p->d_name);

            // 保存源路径与目标路径
            struct cp_file cp_filename;
            strcpy(cp_filename.src, src_path);
            strcpy(cp_filename.dest, dest_path);

            // 创建一个线程，让线程执行一个cp函数->拷贝文件的内容
            pthread_t id;
            pthread_create(&id, NULL, cp, (void *)&cp_filename);

            // 等待线程结束
            pthread_join(id, NULL);
        }
    }
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