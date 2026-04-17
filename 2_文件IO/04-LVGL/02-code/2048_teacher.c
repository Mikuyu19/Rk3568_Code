#include "2048.h"
#include <stdio.h>
#include "main_interface.h"
#include <time.h>

#define TILE_SIZE 60

//定义2048游戏数组
static int game_grid[4][4] = {
	0,0,2,4,
	0,0,0,2,
	2,2,0,0,
	0,0,0,0
};

//保存图片控件的数组
static lv_obj_t *tile_image[4][4];
//计算棋盘中0的个数
int get_arr_zero_count()
{
    int count = 0;
    for(int i = 0;i<4;i++)
    {
        for(int j = 0;j<4;j++)
        {
            if(game_grid[i][j] == 0)
                count++;
        }
    }
    return count;
}

//产生一个随机数
void rand_num()
{
	//计算棋盘中0的个数
    int count = get_arr_zero_count();
	//随机生成1-count之间的随机数k k表示生成2或4是第几个0的位置
    int k = rand()%count+1;
    int q = 0;
	//找k的位置 放置2或4
    for(int i = 0;i<4;i++)
    {
        for(int j = 0;j<4;j++)
        {
            if(game_grid[i][j] == 0)
            {
                q++;
                if(q == k)
                {
                
                    game_grid[i][j] = rand()%10>2?2:4;
                    break;
                }
            }
        }
    }
}

//初始化2048游戏棋盘数字的值
static void init_2048_game()
{
	//设置随机种子
    srand(time(NULL));
	//把游戏棋盘数字的值变成0
    memset(game_grid,0,sizeof(game_grid));
	//生成两个随机数
    rand_num();
    rand_num();
}

static void to_select_app_screen_cb(lv_event_t *e)
{
    //选择界面不存在则创建
    if(select_app_screen == NULL)
    {
        //初始化选择界面
        select_app_screen = ui_select_app_screen_init();
    }

    //加载选择界面并删除2048界面
    lv_screen_load(select_app_screen);
    if(game_2048_screen!=NULL)
    {
        lv_obj_delete(game_2048_screen);
        game_2048_screen = NULL;
    }
}

/*
	去二维数组中一行的0 把其他数字放左边，0放右边
	eg:
        2 0 4 0 ->2 4 0 0 
    temp_arr：传入一维数组的名字，有4个int
*/
void rm_zero(int temp_arr[],int *flag)
{
	int k = 0;//待放入有效数字的下标
    for(int i = 0;i<4;i++)
    {
        if(temp_arr[i]!=0)//找非0数字
        {
            temp_arr[k] = temp_arr[i];
            if(k!=i)
            {
                temp_arr[i] = 0;
                *flag = 1;
            }
            k++;
        }
    }
}

/*
	把二维数组中一行的数组，相邻且相同进行合并
	eg:
        2 2 0 0 ->4 0 0 0
    temp_arr：传入一维数组的名字，有4个int
*/
void hebing(int temp_arr[],int *flag)
{
    for(int i = 0;i<3;i++)
    {
        if(temp_arr[i] == temp_arr[i+1] && temp_arr[i]!=0)
        {
            temp_arr[i]*=2;
            temp_arr[i+1] = 0;
            *flag = 1;
        }
    }	
}

void slide_left()
{
    int temp_arr[4];
    int flag = 0;//0 数组没有发送变化 1 数组发送变化
    for(int i = 0;i<4;i++)
    {
        //获取一行数据
        for(int j = 0;j<4;j++)
        {
            temp_arr[j] = game_grid[i][j];
        }

        //去0
        rm_zero(temp_arr,&flag);
        //合并
        hebing(temp_arr,&flag);
        //去0
        rm_zero(temp_arr,&flag);

        //再把数据放回到游戏数组当中
        for(int j = 0;j<4;j++)
        {
             game_grid[i][j] = temp_arr[j];
        }
    }

    if(flag == 1)
    {
        rand_num();
    }
}

void slide_right()
{
    int temp_arr[4];
    int flag = 0;//0 数组没有发送变化 1 数组发送变化
    for(int i = 0;i<4;i++)
    {
        //获取一行数据
        for(int j = 0;j<4;j++)
        {
            temp_arr[3-j] = game_grid[i][j];
        }

        //去0
        rm_zero(temp_arr,&flag);
        //合并
        hebing(temp_arr,&flag);
        //去0
        rm_zero(temp_arr,&flag);

        //再把数据放回到游戏数组当中
        for(int j = 0;j<4;j++)
        {
             game_grid[i][j] = temp_arr[3-j];
        }
    }

    if(flag == 1)
    {
        rand_num();
    }
}


static lv_point_t start_point;//按下
static lv_point_t end_point;//松开
/*
    0 点击
    1 上
    2 下
    3 左
    4 右
*/
static int get_slide(lv_point_t start_point,lv_point_t end_point)
{
    int x = end_point.x-start_point.x;
    int y = end_point.y-start_point.y;

    if(x == 0 && y == 0)
    {
        return 0;
    }
    else if(abs(x) > abs(y))//左和右
    {
        if(x>0)
        {
            return 4;
        }
        else
        {
            return 3;
        }
    }
    else //上和下
    {
        if(y>0)
        {
            return 2;
        }
        else
        {
            return 1;
        }
    }
}

static int game_fail()
{
    //还有没有空位
    if(get_arr_zero_count()!=0)
        return 0;//没有失败 继续玩
    
    //满了 并且没有相邻且相同的数字
    for(int i = 0;i<4;i++)
    {
    	for(int j = 0;j<4;j++)
    	{
            if(j<3 && game_grid[i][j] == game_grid[i][j+1] && game_grid[i][j] != 0)//右边
                return 0;////没有失败 继续玩
            if(i<3 && game_grid[i][j] == game_grid[i+1][j] && game_grid[i][j] != 0)//下面
                return 0;////没有失败 继续玩
        }
    }    

    return 1;//失败
}

void update_2048_game()
{
    //在win中创建4*4的图像控件
    for(int i = 0;i<4;i++)
    {
    	for(int j = 0;j<4;j++)
    	{
    		//获取图片的路径保存到字符数组sprintf
            char bmppathname[1024] = {0};
    		sprintf(bmppathname,"A:/mnt/hgfs/CS2612/二阶段/02-文件IO/04-LVGL/02-code/ubuntu_demo/pic/%d.bmp",game_grid[i][j]);
    		//设置图片控件显示哪张图片
            lv_image_set_src(tile_image[i][j],bmppathname);
    	}
    }    
}
static void test10_cb(lv_event_t *e)
{

    //获取事件编码
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_PRESSED)//按下
    {
        //获取一个点击坐标
        lv_indev_get_point(lv_indev_active(), &start_point);   
    }
    else if(code == LV_EVENT_RELEASED)//松开
    {
        //获取一个点击坐标
        lv_indev_get_point(lv_indev_active(), &end_point);   
        
        //获取滑动方向
        int r = get_slide(start_point,end_point);
        if(r == 0)
        {
            printf("click(%d,%d)\n",end_point.x,end_point.y);
        }
        else if(r == 1)
        {
            printf("up\n");
        }
        else if(r == 2)
        {
            printf("down\n");
        }
        else if(r == 3)
        {
            printf("left\n");
            slide_left();
        }
        else if(r == 4)
        {
            printf("right\n");
            slide_right();
        }

        //更新界面
        update_2048_game();

        //判断是否失败
        if(game_fail())
        {
            printf("游戏结束\n");
        }

        //判断是否成功
    }
}

//初始化2048游戏界面
lv_obj_t * ui_2048_init()
{
	//初始化游戏4*4格子的数据 (可以先不写，手动修改上面的数据)
    init_2048_game();

    //创建窗口win
    lv_obj_t *game_2048_screen = lv_obj_create(NULL);

    lv_obj_t *win = lv_obj_create(game_2048_screen);
    //设置大小
    lv_obj_set_size(win,300,300);
    //设置位置
    lv_obj_set_align(win,9);
    
    //给win设置内边距
    lv_obj_set_style_pad_all(win,0,0);
    
    //在win中创建4*4的图像控件
    for(int i = 0;i<4;i++)
    {
    	for(int j = 0;j<4;j++)
    	{
    		//创建图片控件
            tile_image[i][j] = lv_image_create(win);
    		
    		//设置图片控件位置lv_obj_set_pos
            lv_obj_set_pos(tile_image[i][j],10+(TILE_SIZE+10)*j,10+(TILE_SIZE+10)*i);
    		
    		//获取图片的路径保存到字符数组sprintf
            char bmppathname[1024] = {0};
    		sprintf(bmppathname,"A:/mnt/hgfs/CS2612/二阶段/02-文件IO/04-LVGL/02-code/ubuntu_demo/pic/%d.bmp",game_grid[i][j]);
    		//设置图片控件显示哪张图片
            lv_image_set_src(tile_image[i][j],bmppathname);
    	}
    }
    //将win和函数进行绑定
    lv_obj_add_event_cb(win, test10_cb, LV_EVENT_ALL, NULL);     

    //创建返回按钮
    lv_obj_t*return_btn = lv_button_create(game_2048_screen);
    lv_obj_set_align(return_btn,LV_ALIGN_TOP_RIGHT);
    lv_obj_t*return_btn_lab = lv_label_create(return_btn);
    lv_label_set_text(return_btn_lab,"return");
     lv_obj_add_event_cb(return_btn,to_select_app_screen_cb,LV_EVENT_CLICKED,NULL); 


    //加载活动屏幕
    lv_screen_load(game_2048_screen);
    
    return game_2048_screen;
}