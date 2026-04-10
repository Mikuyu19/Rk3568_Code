#include <stdio.h>
#include <unistd.h>

#include "gy39.h"
#include "mq2.h"

int main(void)
{
    struct gy39_data gy39_buf;
    struct mq2_data mq2_buf;

    printf("sensor monitor start\n");

    int gy39_fd = gy39_open();
    if (gy39_fd == -1)
    {
        printf("open gy39 uart fail\n");
        return 1;
    }

    int mq2_fd = mq2_open();
    if (mq2_fd == -1)
    {
        printf("open mq2 uart fail\n");
        close(gy39_fd);
        return 1;
    }

    while (1)
    {
        if (gy39_read_data(gy39_fd, &gy39_buf) == 0 &&
            mq2_read_data(mq2_fd, &mq2_buf) == 0)
        {
            printf("-------------------------\n");
            gy39_print_data(gy39_buf);
            mq2_print_data(mq2_buf);
            printf("-------------------------\n");
        }
        else
        {
            printf("read sensor data fail\n");
        }

        sleep(1);
    }

    close(gy39_fd);
    close(mq2_fd);
    return 0;
}
