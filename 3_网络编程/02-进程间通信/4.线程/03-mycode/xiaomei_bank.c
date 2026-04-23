#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 银行卡余额
int balance = 0;

// 互斥锁和条件变量
pthread_mutex_t mutex;
pthread_cond_t cond;

// 小美取钱线程
void *xiaomei_withdraw(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        printf("小美: 我要取钱...\n");

        // 如果余额为0,等待父母汇款
        while (balance == 0)
        {
            printf("小美: 余额为0,等待父母汇款...\n");
            pthread_cond_wait(&cond, &mutex); // 释放锁并等待,被唤醒后重新获取锁
            printf("小美: 收到通知,检查余额...\n");
        }

        // 再次确认有钱才取
        if (balance > 0)
        {
            int amount = (balance >= 1000) ? 1000 : balance;
            balance -= amount;
            printf("小美: 成功取到 %d 元!\n", amount);
            printf("小美: 当前余额: %d 元\n\n", balance);
        }

        pthread_mutex_unlock(&mutex);

        sleep(2); // 模拟花钱的时间
    }

    return NULL;
}

// 兄弟姐妹取钱线程
void *sibling_withdraw(void *arg)
{
    int id = *(int *)arg;

    while (1)
    {
        pthread_mutex_lock(&mutex);

        // 如果余额为0,等待父母汇款
        while (balance == 0)
        {
            printf("兄弟姐妹%d: 余额为0,等待父母汇款...\n", id);
            pthread_cond_wait(&cond, &mutex); // 释放锁并等待,被唤醒后重新获取锁
            printf("兄弟姐妹%d: 收到通知,检查余额...\n", id);
        }

        // 再次确认有钱才取
        if (balance > 0)
        {
            int amount = (balance >= 1000) ? 1000 : balance;
            balance -= amount;
            printf("兄弟姐妹%d: 取走了 %d 元!\n", id, amount);
            printf("兄弟姐妹%d: 当前余额: %d 元\n\n", id, balance);
        }

        pthread_mutex_unlock(&mutex);

        sleep(2); // 模拟花钱的时间
    }

    return NULL;
}

// 父母汇款线程
void *parent_deposit(void *arg)
{
    while (1)
    {
        sleep(3); // 每3秒汇一次款

        pthread_mutex_lock(&mutex);

        if (balance == 0)
        {
            balance = 3000;
            printf("父母: 已汇款 1000 元,当前余额: %d 元\n", balance);
            printf("父母: 打电话通知孩子们...\n");

            // 唤醒所有等待的线程(小美和兄弟姐妹)
            pthread_cond_broadcast(&cond);
        }

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main()
{
    pthread_t xiaomei_tid, parent_tid;
    pthread_t sibling_tids[2]; // 2个兄弟姐妹
    int sibling_ids[2] = {1, 2};

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // 创建父母汇款线程
    pthread_create(&parent_tid, NULL, parent_deposit, NULL);

    // 创建小美取钱线程
    pthread_create(&xiaomei_tid, NULL, xiaomei_withdraw, NULL);

    // 创建兄弟姐妹取钱线程
    for (int i = 0; i < 2; i++)
    {
        pthread_create(&sibling_tids[i], NULL, sibling_withdraw, &sibling_ids[i]);
    }

    pthread_join(parent_tid, NULL);
    pthread_join(xiaomei_tid, NULL);
    for (int i = 0; i < 2; i++)
    {
        pthread_join(sibling_tids[i], NULL);
    }

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
