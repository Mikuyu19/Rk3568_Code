# mplayer项目

键盘控制方式

```
 <-  or  ->       seek backward/forward 10 seconds
 down or up       seek backward/forward  1 minute
 pgdown or pgup   seek backward/forward 10 minutes
 < or >           step backward/forward in playlist
 p or SPACE       pause movie (press any key to continue)
 q or ESC         stop playing and quit program
 + or -           adjust audio delay by +/- 0.1 second
 o                cycle OSD mode:  none / seekbar / seekbar + timer
 * or /           increase or decrease PCM volume
 x or z           adjust subtitle delay by +/- 0.1 second
 r or t           adjust subtitle position up/down, also see -vf expand

```

slave命令后台处理模式

1）查看slave模式命令列表

```bash
china@ubuntu:/mnt/hgfs/cs2520F/02-第二阶段/02-并发/2025.8.1-进程间通信/03-资料$ mplayer -input cmdlist
radio_step_channel   Integer
radio_set_channel    String
radio_set_freq       Float
radio_step_freq      Float
seek                 Float [Integer]
edl_loadfile         String
edl_mark            
audio_delay          Float [Integer]
speed_incr           Float
speed_mult           Float
speed_set            Float
quit                 [Integer]
stop                
pause               
frame_step          
pt_step              Integer [Integer]
pt_up_step           Integer [Integer]
alt_src_step         Integer
loop                 Integer [Integer]
sub_delay            Float [Integer]
sub_step             Integer [Integer]
```

常用的命令

```
loadfile   string   //参数string 为 歌曲名字。 
 
volume 100 1  //设置音量 中间的为音量的大小。
 
mute  1/0 //静音开关
 
pause  //暂停/取消暂停
 
get_time_length  //返回值是播放文件的长度，以秒为单位。
 
seek value   //向前查找到文件的位置播放 参数value为秒数。
 
get_percent_pos //返回文件的百分比（0--100）
 
get_time_pos //打印出在文件的当前位置用秒表示，采用浮点数
 
volume  [abs] //增大/减小音量，或将其设置为，如果[abs]不为零

 quit [value]
 退出MPlayer。可选的整数[value]的值作为返回代码
 为mplayer的进程（默认值：0）。
 
get_file_name //打印出当前文件名
 
get_meta_album //打印出当前文件的'专辑'的元数据
 
get_meta_artist //打印出当前文件的'艺术家'的元数据
 
get_meta_comment //打印出当前文件的'评论'的元数据
 
get_meta_genre //打印出当前文件的'流派'的元数据
 
get_meta_title //打印出当前文件的'标题'的元数据
 
get_meta_year //打印出当前文件的'年份'的元数据
```

2)使用slave模式的命令从控制台输入控制命令控制mplayer播放器

```
运行mplayer -slave -quiet <movie>,并在控制台窗口输入slave命令
-quiet 不输出冗余的信息

注意：
	rk3568开发板和6818的开发板都没有mplayer,需要下载mplayer的包，进行交叉编译，再把交叉编译好的mplayer的程序放到开发板的/usr/bin目录下
```

```bash
china@ubuntu:/mnt/hgfs/cs2520F/02-第二阶段/02-并发/2025.8.1-进程间通信/03-资料$ mplayer -slave -quiet 1.mp4
MPlayer 1.3.0 (Debian), built with gcc-7 (C) 2000-2016 MPlayer Team
do_connect: could not connect to socket
connect: No such file or directory
Failed to open LIRC support. You will not be able to use your remote control.

Playing 1.mp4.
libavformat version 57.83.100 (external)
libavformat file format detected.
[mov,mp4,m4a,3gp,3g2,mj2 @ 0x7fe2d09ce2a0]Protocol name not provided, cannot determine if input is local or a network protocol, buffers and access patterns cannot be configured optimally without knowing the protocol
[lavf] stream 0: video (hevc), -vid 0
[lavf] stream 1: audio (aac), -aid 0, -alang eng
VIDEO:  [HEVC]  640x360  24bpp  25.000 fps  303.7 kbps (37.1 kbyte/s)
Failed to open VDPAU backend libvdpau_nvidia.so: cannot open shared object file: No such file or directory
[vdpau] Error when calling vdp_device_create_x11: 1
==========================================================================
Opening video decoder: [ffmpeg] FFmpeg's libavcodec codec family
libavcodec version 57.107.100 (external)
Selected video codec: [ffhevc] vfm: ffmpeg (FFmpeg HEVC / H.265)
==========================================================================
Clip info:
 major_brand: isom
 minor_version: 512
 compatible_brands: isomiso2mp41
 encoder: Lavf58.20.100
 description: Tencent CAPD MTS 
Load subtitles in ./
==========================================================================
Opening audio decoder: [ffmpeg] FFmpeg/libavcodec audio decoders
AUDIO: 44100 Hz, 2 ch, floatle, 96.0 kbit/3.40% (ratio: 12000->352800)
Selected audio codec: [ffaac] afm: ffmpeg (FFmpeg AAC (MPEG-2/MPEG-4 Audio))
==========================================================================
AO: [pulse] 44100Hz 2ch floatle (4 bytes per sample)
Starting playback...
Movie-Aspect is undefined - no prescaling applied.
VO: [xv] 640x360 => 640x360 Planar YV12 
mute 1
mute 0
pause
pause
get_time_length
ANS_LENGTH=208.08
^C

MPlayer interrupted by signal 2 in module: sleep_timer

Exiting... (Quit)
china@ubuntu:/mnt/hgfs/cs2520F/02-第二阶段/02-并发/2025.8.1-进程间通信/03-资料$ 
```

小项目：基于mplayer的简易的音视频播放器

```
sudo apt install mplayer
```

要求：

```
要能够实现手动播放
手动播放要求：快进/快退/暂停/继续播放/上一首/下一首/调节音量/静音/解除静音....
```

拓展：

```
歌词，收藏夹....
```

```c
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#define FIFOPATH "/home/china/1.fifo"
char *video[] = {"1.mp4", "2.mp4", "3.mp4"};
int volume_value = 100;
// 取一首歌
int i = 0;    // i为歌曲的下标
void video_play()
{

    // 打开有名管道
    int fdfifo = open(FIFOPATH, O_RDWR);
    if (fdfifo == -1)
    {
        perror("open FIFOPATH fail");
        exit(0);
    }

    while (1)
    {
        pid_t pid = fork();
        if (pid == 0) // 子
        {
            char fifoname[128] = {0};
            sprintf(fifoname, "file=%s", FIFOPATH);
            execlp("mplayer", "mplayer", "-slave", "-quiet", "-input", fifoname, "-zoom", "-x", "800", "-y", "480", video[i], NULL);
        }
        else if (pid > 0) // 父进程
        {
            char cmd[50] = {0};
            while (1)
            {
                char c = getchar();
                switch (c)
                {
                case 'a': // 快进
                    write(fdfifo, "seek +5\n", strlen("seek +5\n"));
                case 'q': // 退出
                    write(fdfifo, "quit\n", strlen("quit\n"));
                    wait(NULL); // 回收子进程的资源
                    exit(0);    // 退出程序
                }
            }
        }
    }
}

int main()
{
    // 音视频开始播放
    video_play();
}
```

