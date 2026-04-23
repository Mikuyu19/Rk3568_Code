
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

// 文件拷贝信息结构体
struct cp_file
{
    char src[2048];  // 源文件路径
    char dest[2048]; // 目标文件路径
};

void *cp(void *arg)
{
    struct cp_file = *(struct cp_file *)arg;

    // 用只读的方式打开源文件
    // 用只写的方式打开目的文件

    // 循环读取源文件的数据写入到目的文件
    while (1)
    {
        int ret = read(fd_src, ....);
        if (ret == 0)
            break;
        write(fd_dest, ....);
    }

    // 关闭两个文件
}

/*
    把src_dir拷贝到dest_dir目录下
    src_dir:源目录路径
    dest_dir：目的目录路径
*/
void cp_dir(char *src_dir, char *dest_dir)
{
    /*打开源目录(从哪个目录拷贝)*/

    /*分别获取源目录和目的目录的绝对路径(后面接你需要拼接带路径的文件名)*/
    // 保存当前目录的工作路径
    // 保存要拷贝的源目录的工作路径
    // 保存要拷贝的目的目录的工作路径

    /*让源目录成为目的目录的子目录名，获取这个子目录的名字*/

    /*获取这个子目录的名字与目的目录进行拼接，并创建这个目录*/

    /*更新目的目录*/

    /*读取源目录的目录项*/
    struct dirent *p;
    while (p = readdir())
    {
        // 先跳过两个隐藏目录. ..

        // 拼接源路径

        // 获取文件属性

        // 如果是目录
        cp_dir(....);

        // 如果是普通文件
        if (S_ISREG(st.st_mode))
        {
            // 拼接目标路径

            // 保存源路径与目标路径
            struct cp_file cp_filename;
            strcpy(cp_filename.src, "/xxxxx/xx.c");
            strcpy(cp_filename.dest, "./xx.c");
            // 创建一个线程，让线程执行一个cp函数->拷贝文件的内容
            pthread_t id;
            pthread_create(&id, NULL, cp, (void *)cp_filename);

            // 等待线程结束
            pthread_join(id, NULL);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage:%s <src_dir> <dest_dir>\n", argv[0]);
        return -1;
    }
    cp_dir(argv[1], argv[2]);
}