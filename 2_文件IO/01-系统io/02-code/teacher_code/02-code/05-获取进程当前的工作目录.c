#include <stdio.h>
#include <unistd.h>
int main()
{
    #if 0
    char buf[100] = {0};
    char *p = getcwd(buf,sizeof(buf)-1);
    if(p == NULL)
    {
        perror("getcwd fail");
        return -1;
    }
    printf("buf:%s\n",buf);
    printf("p:%s\n",p);
    #endif

    #if 0
    char buf[100] = {0};
    char *p = getwd(buf);
    if(p == NULL)
    {
        perror("getwd fail");
        return -1;
    }
    printf("buf:%s\n",buf);
    printf("p:%s\n",p);
    #endif

    #if 1
    char *get_current_dir_name(void);//一定要加声明
    char *p = get_current_dir_name();
    if(p == NULL)
    {
        perror("get_current_dir_name fail");
        return -1;
    }
    printf("p:%s\n",p);
    free(p);//释放空间
    #endif
}