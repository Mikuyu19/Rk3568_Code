#include <stdio.h>
#include <sys/utsname.h>

#include "add.h"
#include "sub.h"

static void print_board_info(void)
{
    struct utsname u;
    if (uname(&u) == 0) {
        printf("system : %s\n", u.sysname);
        printf("kernel : %s\n", u.release);
        printf("arch   : %s\n", u.machine);
    }

    FILE *fp = fopen("/proc/device-tree/model", "r");
    if (fp == NULL) {
        fp = fopen("/sys/firmware/devicetree/base/model", "r");
    }

    if (fp != NULL) {
        char model[128] = {0};
        size_t n = fread(model, 1, sizeof(model) - 1, fp);
        fclose(fp);
        if (n > 0) {
            model[n] = '\0';
            printf("board  : %s\n", model);
        }
    } else {
        printf("board  : unknown\n");
    }
}

int main(void)
{
    print_board_info();
    printf("3 + 4 = %d\n", add(3, 4));
    printf("10 - 4 = %d\n", sub(10, 4));
    return 0;
}
