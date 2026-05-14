#include <stdio.h>
#include "cJSON.h"

// {"name":"luffy","sex":"man","age":19}
// {"list":{"name":"luffy","age":19},"other":{"name":"ace"}}
// {"names":["luffy","robin"]}

void print(cJSON *object)
{
    if (object == NULL)
    {
        printf("Error: cJSON object is NULL!\n");
        return;
    }
    if (object->type == cJSON_String) // 字符串
        printf("%s:%s\n", object->string, object->valuestring);
    else if (object->type == cJSON_Number) // 数字
        printf("%s:%d\n", object->string, object->valueint);
    else if (object->type == cJSON_Array) // 数组
    {
        printf("%s:", object->string);
        // 获取数组中元素的个数
        int num = cJSON_GetArraySize(object);
        for (int i = 0; i < num; i++)
        {
            cJSON *temp = cJSON_GetArrayItem(object, i); // 获取数组元素
            if (temp->type == cJSON_String)
                printf("%s ", temp->valuestring);
            else if (temp->type == cJSON_Number)
                printf("%d ", temp->valueint);
        }
        printf("\n");
    }
}

int main()
{
    char buf[] = "{\"name\":\"luffy\",\"sex\":\"man\",\"age\":19}";
    char buf2[] = "{ \"list\" : {\"name\" : \"luffy\", \"age\" : 19},\"other\" : {\"name\" : \"ae\"}}";
    char buf3[] = "{\" names \":[\" luffy \",\" robin \"]}";
    // 把字符串转成cjson格式
    cJSON *cjson_obj = cJSON_Parse(buf3);

    if (cjson_obj == NULL)
        return -1; // 解析失败检查

    // // 1. 先获取 "list" 对象
    // cJSON *cjson_list = cJSON_GetObjectItem(cjson_obj, "list");
    // if (cjson_list != NULL)
    // {
    //     // 2. 再从 "list" 对象中获取 "name" 和 "age"
    //     cJSON *cjson_name = cJSON_GetObjectItem(cjson_list, "name");
    //     print(cjson_name);

    //     cJSON *cjson_age = cJSON_GetObjectItem(cjson_list, "age");
    //     print(cjson_age);
    // }
    //-----------------------------------------------------------
    // 2. 获取键值对（注意：键名必须带上空格 " names "）
    cJSON *cjson_names = cJSON_GetObjectItem(cjson_obj, " names ");

    // 3. 打印数组内容（调用你自己的函数）
    print(cjson_names);

    // 销毁cjson对象
    cJSON_Delete(cjson_obj);
    return 0;
}
