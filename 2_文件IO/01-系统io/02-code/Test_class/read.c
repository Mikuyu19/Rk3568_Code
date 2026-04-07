// 练习：先打开文件2.txt，接着往文件中写入数据，去把写入的数据读取出来
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main()
{
    int fd;
    char *msg = "Hello, Linux File IO! This is 2.txt\n";
    char buf[100] = {0}; // 初始化读取缓冲区

    fd = open("2.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("打开/创建 2.txt 失败");
        return -1;
    }

    int write_ret = write(fd, msg, strlen(msg));
    if (write_ret == -1)
    {
        perror("写入失败");
        close(fd);
        return -1;
    }
    printf("--> 成功写入了 %d 个字节的数据。\n", write_ret);

    // ========================================================
    //
    // 此时光标在文件末尾，直接 read 会返回 0。
    // 解决方法：使用 lseek 将光标重新移动到文件开头 (SEEK_SET 代表文件头部)
    //
    // 备用方案（如果你还没学 lseek）：
    // close(fd);
    // fd = open("2.txt", O_RDONLY); // 重新打开，光标默认会在开头
    // ========================================================
    lseek(fd, 0, SEEK_SET);
    printf("已将文件光标移回文件开头。\n");

    // 3. 把写入的数据读取出来
    // 我们尝试读取 sizeof(buf)-1 个字节，留一个字节给字符串结束符 '\0'
    int read_ret = read(fd, buf, sizeof(buf) - 1);

    if (read_ret == -1)
    {
        perror("读取失败");
    }
    else if (read_ret == 0)
    {
        printf("读到了 0 个字节，说明已经到了文件末尾(EOF)。\n");
    }
    else
    {
        printf("成功读取了 %d 个字节，内容如下:\n%s", read_ret, buf);
    }

    // 4. 关闭文件
    close(fd);

    return 0;
}