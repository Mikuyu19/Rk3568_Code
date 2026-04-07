1.加载led的内核驱动

```
把kobject_led.ko下载到开发板，然后输入命令
	insmod kobject_led.ko(每次开机都要做这步)
加载成功，在开发板目录下面/sys/kernel/gec_ctrl/
	生成
		led_d7
		led_d8
		led_d9
		led_d10
		led_all
		beep
```

2.对led 和 蜂鸣器

```
1.打开led_d7
	int led_fd = open("/sys/kernel/gec_ctrl/led_d7",O_RDWR);
2.操作
	写1	亮
	写0	灭
	int on_or_off = 1;
	write(led_fd,&on_or_off,4);
	on_or_off = 0;
	write(led_fd,&on_or_off,4);
3.关闭文件
```

led.c

```
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "led.h"


//led控制函数
void lcd_ctrl(char *led_name,int onoff)
{
	int led_fd = open(led_name,O_RDWR);
	
	write(led_fd,&onoff,4);
	close(led_fd);
}
```

led.h

```c
#ifndef _LED_H_
#define _LED_H_

#define LED7	"/sys/kernel/gec_ctrl/led_d7"
#define LED8	"/sys/kernel/gec_ctrl/led_d8"
#define LED9	"/sys/kernel/gec_ctrl/led_d9"
#define LED10	"/sys/kernel/gec_ctrl/led_d10"
#define LEDALL	"/sys/kernel/gec_ctrl/led_all"
#define BEEP	"/sys/kernel/gec_ctrl/beep"

#define ON 1
#define OFF 0

//led控制函数
void lcd_ctrl(char *led_name,int onoff);

#endif
```

