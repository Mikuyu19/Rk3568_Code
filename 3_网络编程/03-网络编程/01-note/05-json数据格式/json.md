

- json：[JSON](https://www.json.org/) 全称 JavaScript Object Notation，即 JS对象简谱，是一种轻量级的数据格式。它采用完全独立于编程语言的文本格式来存储和表示数据，语法简洁、层次结构清晰，易于人阅读和编写，同时也易于机器解析和生成，有效的提升了网络传输效率。

- cjson:基于C语言的json库，有两个源码文件 cJSON.c 和cJSON.h

    ```
    gcc xxx.c cJSON.c
    ./a.out
    ```

-   语法

    ```c
    json对象是一个无序的“名称/值”键值对的集合
    -以{开始 以}结束,允许嵌套使用
    -每个名称和值成对出现，名称和值之前使用:分隔
    -键值对之间用，分隔
    -这些字符前后允许出现无意义的空白符
    
    对于键值，可以有如下的值
    -一个是新的json的对象
    -数组：使用[和]表示
    -数字：直接表示，可以是整数，也可以是浮点数
    -字符串：使用""表示
    ```

- cjson结构体

  ```c
  /* The cJSON structure: */
  typedef struct cJSON {
  	struct cJSON *next,*prev;	 //next指针：指向下一个键值对  prev指针指向上一个键值对
  	struct cJSON *child;		//在键值对结构体中，当该键值对的值是一个嵌套的JSON数据或者一个数组时，由child指针指向该条新链表。
  
  	int type;					//键值对 中   值的数据类型  cJSON Types
  
  	char *valuestring;			//如果 键值对中的值，是字符串类型，该变量就是字符串的地址
  	int valueint;				//如果 键值对中的值，是int类型，该变量就是值的数据
  	double valuedouble;			//如果 键值对中的值，是double类型，该变量就是值的数据
  
  	char *string;				//存放 键值对 中 的 键（键都是字符串类型）
  } cJSON;
  
  eg:
  	{
  		"name":"lixiang",
  		"age":18,
  		"score":[80,90,100]
  	}
  
  	CJSON*object;//假设让它指向{}
  	object->child;//"name":"lixiang"  里面链表中的第一个结点的地址
  	object->child->string;//name
  	object->child->valuestring;//lixiang
  	object->child->next;//"age":18
  	object->child->next->valueint;//18
  	object->child->next->next;//"score":[80,90,100]
  	object->child->next->next->child;//[80,90,100]指向80这个元素的地址
  	object->child->next->next->child->valueint;//80
  	object->child->next->next->child->next->valueint;//90
  ```



# 1.cjson数据的封装

1.cjson对象或数组的创建和销毁

```c
//创建一个空值json对象
cJSON * cJSON_CreateNull(void);
//创建一个true值json对象
cJSON * cJSON_CreateTrue(void);
//创建一个false值json对象
cJSON * cJSON_CreateFalse(void);
//创建一个布尔值json对象
cJSON * cJSON_CreateBool(cJSON_bool boolean);
//创建一个整型json对象
cJSON * cJSON_CreateNumber(double num);
//创建一个字符串json对象
cJSON * cJSON_CreateString(const char *string);
//创建一个数组json对象
cJSON * cJSON_CreateArray(void);
//创建一个json对象
cJSON * cJSON_CreateObject(void);
//销毁json对象
void cJSON_Delete(cJSON *item);
```

**2.cjson对象数据添加**

```c
//在对象上添加NULL
cJSON* cJSON_AddNullToObject(cJSON * const object, const char * const name);
//在对象上添加true
cJSON* cJSON_AddTrueToObject(cJSON * const object, const char * const name);
//在对象上添加false
cJSON* cJSON_AddFalseToObject(cJSON * const object, const char * const name);
//在对象上添加布尔值
cJSON* cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
//在对象上添加整数
cJSON* cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number);
//在对象上添加字符串
cJSON* cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
//在一个对象里面添加另外一个cjson的对象,key叫做name
cJSON*) cJSON_AddObjectToObject(cJSON * const object, const char * const name);
//在一个对象里面添加一个数组，数组的key叫做name
cJSON* cJSON_AddArrayToObject(cJSON * const object, const char * const name);
//在数组json对象array上添加一个item的json对象
cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item);
//在json对象object上添加一个item的json对象，并且key的名字为string
cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
```

**3.将cjson格式的对象转化成字符串**

输出JSON数据，可以将整条链表中存放的JSON信息输出到一个字符串中

```c
char * cJSON_Print(const cJSON *item);//格式
char * cJSON_PrintUnformatted(const cJSON *item);//无格式

记得释放空间
```

例子：

```c
{
    "name":"lixiang",
    "age":18,
    "score":[80,90,100]
}
	
#include <stdio.h>
#include "cJSON.h"

int main()
{
    //创建一个cjson对象
    cJSON *cjson_obj =  cJSON_CreateObject();//{}
    cJSON_AddStringToObject(cjson_obj,"name","lixiang");
    cJSON_AddNumberToObject(cjson_obj,"age",18);
    //添加数组到cjson_obj
    cJSON *array = cJSON_AddArrayToObject(cjson_obj, "score");
    //添加元素到数组中
    cJSON_AddItemToArray(array, cJSON_CreateNumber(80));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(90));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(100));
    
    //把cjson转化为字符串
    char *buf = cJSON_Print(cjson_obj);
    printf("%s\n",buf);
    free(buf);

    char *buf2 = cJSON_PrintUnformatted(cjson_obj);
    printf("%s\n",buf2);
    free(buf2);

    //销毁cjson对象
    cJSON_Delete(cjson_obj);//会连同子对象也会一起销毁
}
```

练习：

```c
{
	"name":"luffy",
	"sex":"man",
	"age":19
}
```

```c
["hello world",10,33]
```

```C
{"person":[{"name":"luffy","age":19}]}
```

# 2. CJSON字符串的解析

1.解析json对象

需要删除

```c
//将json字符串转换为 cJSON 结构体
cJSON * cJSON_Parse(const char *value);
```

2.根据键值对的名称从JSON数据中取出对应的值，返回该键值对(链表节点)的地址

```c
//获取对象object中键值为string的json对象
cJSON * cJSON_GetObjectItem(const cJSON * const object, const char * const string);
```

3.如果JSON数据的值是数组，使用下面的两个API提取数据：

```c
//获取数组array下标为index的json对象
cJSON * cJSON_GetArrayItem(const cJSON *array, int index);
//获取数组中元素的个数
int cJSON_GetArraySize(const cJSON *array);
```

4.判断键值类型并打印

```c
cJSON *object;//假设有一个cjson对象object
if(object->type == cJSON_String)//字符串
    printf("%s:%s\n",object->string,object->valuestring);
else if(object->type == cJSON_Number)//数字
    printf("%s:%d\n",object->string,object->valueint);
else if(object->type == cJSON_Array)//数组
{
    printf("%s:",object->string);
    //获取数组中元素的个数
    int num = cJSON_GetArraySize(object);
    for(int i = 0;i<num;i++)
    {
        cJSON *temp = cJSON_GetArrayItem(object,i);//获取数组元素
        if(temp->type == cJSON_String)
            printf("%s ",temp->valuestring);
        else if(temp->type == cJSON_Number)
            printf("%d ",temp->valueint);
    }
    printf("\n");
} 
```

示例

```c
#include <stdio.h>
#include "cJSON.h"

void print(cJSON *object)
{
    if(object->type == cJSON_String)//字符串
        printf("%s:%s\n",object->string,object->valuestring);
    else if(object->type == cJSON_Number)//数字
        printf("%s:%d\n",object->string,object->valueint);
    else if(object->type == cJSON_Array)//数组
    {
        printf("%s:",object->string);
        //获取数组中元素的个数
        int num = cJSON_GetArraySize(object);
        for(int i = 0;i<num;i++)
        {
            cJSON *temp = cJSON_GetArrayItem(object,i);//获取数组元素
            if(temp->type == cJSON_String)
                printf("%s ",temp->valuestring);
            else if(temp->type == cJSON_Number)
                printf("%d ",temp->valueint);
        }
        printf("\n");
    }     
}
int main()
{
    char buf[] = "{\"name\":\"lixiang\",\"age\":18,\"score\":[80,90,100]}";
    //把字符串转成cjson格式
    cJSON *cjson_obj = cJSON_Parse(buf);

    //获取对象object中键值为string的json对象
    cJSON *cjosn_name = cJSON_GetObjectItem(cjson_obj,"name");    
    print(cjosn_name);

    cJSON *cjosn_age = cJSON_GetObjectItem(cjson_obj,"age");    
    print(cjosn_age);

    cJSON *cjosn_score = cJSON_GetObjectItem(cjson_obj,"score");    
    print(cjosn_score);
}
```

练习：

格式：

```
{"name":"luffy","sex":"man","age":19}
```

格式：

```
{"list":{"name":"luffy","age":19},"other":{"name":"ace"}}
```

格式：

```
{"names":["luffy","robin"]}
```

