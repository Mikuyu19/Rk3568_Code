# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个Linux进程间通信(IPC)学习项目，包含多种IPC机制的示例代码和练习。项目使用C语言编写，面向RK3568嵌入式开发板。

## 编译命令

### 基本编译
```bash
gcc filename.c -o output_name
```

### 线程相关程序
```bash
gcc filename.c -o output_name -pthread
```

### POSIX信号量程序
```bash
gcc filename.c -o output_name -pthread
```

### 实时库(clock_gettime等)
```bash
gcc filename.c -o output_name -lrt -pthread
```

## 代码结构

### 1.管道 (Pipes)
- **无名管道(pipe)**: 用于有亲缘关系的进程间通信，通过`pipe()`创建
- **有名管道(FIFO)**: 通过`mkfifo()`创建，可用于无亲缘关系的进程
- **mplayer项目**: 基于有名管道的音视频播放器，使用slave模式控制mplayer

关键API:
- `pipe(int pipefd[2])` - 创建无名管道
- `mkfifo(const char *pathname, mode_t mode)` - 创建有名管道

### 2.信号 (Signals)
信号是异步通信机制，不能传输数据，只传递信号值。

关键API:
- `signal(int signum, sighandler_t handler)` - 捕捉信号
- 注意: SIGKILL和SIGSTOP不能被捕捉、阻塞或忽略

### 3.System V IPC
包含共享内存和信号量，使用key作为许可证机制。

#### 共享内存流程:
1. `ftok()` - 生成IPC key
2. `shmget()` - 创建/打开共享内存
3. `shmat()` - 映射到进程地址空间
4. 读写操作
5. `shmdt()` - 解除映射
6. `shmctl(IPC_RMID)` - 删除共享内存

#### 信号量流程:
1. `ftok()` - 生成IPC key
2. `semget()` - 创建/打开信号量集
3. `semctl(SETVAL/SETALL)` - 初始化信号量值
4. `semop()` - P/V操作(加锁/解锁)
5. `semctl(IPC_RMID)` - 删除信号量

命令行工具:
```bash
ipcs        # 查看所有IPC对象
ipcs -m     # 查看共享内存
ipcs -s     # 查看信号量
ipcrm -m id # 删除共享内存
ipcrm -s id # 删除信号量
```

### 4.线程 (Threads)
使用pthread库实现多线程编程。

关键API:
- `pthread_create()` - 创建线程
- `pthread_join()` - 等待线程结束
- `pthread_exit()` - 线程退出并返回值

### 5.POSIX信号量
提供有名和无名两种信号量。

#### 有名信号量:
- `sem_open()` - 创建/打开
- `sem_wait()/sem_post()` - P/V操作
- `sem_close()` - 关闭
- `sem_unlink()` - 删除

#### 无名信号量:
- `sem_init(sem, 1, value)` - 初始化(第二个参数为1表示进程间共享)
- `sem_wait()/sem_post()` - P/V操作
- `sem_destroy()` - 销毁

## 常见模式

### 共享内存+信号量保护
```c
// 创建共享内存和信号量
key_t key = ftok(".", 1);
int shm_id = shmget(key, size, IPC_CREAT | IPC_EXCL | 0777);
int sem_id = semget(key, 1, IPC_CREAT | IPC_EXCL | 0777);

// 初始化信号量为1(互斥锁)
semctl(sem_id, 0, SETVAL, 1);

// 使用
sem_p(sem_id, 0);  // 加锁
// 临界区代码
sem_v(sem_id, 0);  // 解锁
```

### 父子进程通信
```c
pipe(pipefd);
pid_t pid = fork();
if (pid == 0) {
    // 子进程: 关闭不用的端，执行操作
    close(pipefd[0]);
    write(pipefd[1], data, len);
    close(pipefd[1]);
} else {
    // 父进程: 关闭不用的端，执行操作
    close(pipefd[1]);
    read(pipefd[0], buf, len);
    close(pipefd[0]);
    wait(NULL);
}
```

## 重要注意事项

1. **有名管道路径**: 不要在共享文件夹中创建FIFO，会有权限问题。使用Linux本地路径如`/home/china/1.fifo`

2. **ftok路径**: 必须是存在的且有读权限的路径，建议使用`"."`或`"/home/china"`

3. **信号量初值**: 
   - 互斥锁(mutex): 初值为1
   - 生产者-消费者: 读信号量初值0，写信号量初值1

4. **竞态条件**: 多进程/线程访问共享资源时必须使用信号量保护，否则会出现数据不一致(如20万问题)

5. **死锁预防**: 多个资源时，所有进程必须按相同顺序获取锁

6. **资源清理**: 
   - 共享内存和信号量不会自动删除，必须显式调用`shmctl()`和`semctl()`
   - 使用`ipcs`和`ipcrm`命令管理残留的IPC对象

7. **mplayer项目**: 需要在RK3568开发板上交叉编译mplayer并放到`/usr/bin`目录

## 目标平台

- 开发环境: Ubuntu (虚拟机或WSL)
- 目标平台: RK3568嵌入式开发板
- 交叉编译: 某些程序需要交叉编译后部署到开发板
