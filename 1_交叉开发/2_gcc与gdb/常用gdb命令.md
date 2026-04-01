b main：在 main 下断点
b swap：在 swap 下断点
b 6：在第 6 行下断点
r：运行
n：单步，不进入函数
s：单步，进入函数
c：继续运行
finish：执行完当前函数
p a：打印变量
p *x：打印指针指向的值
p &a：打印地址
p/x a：十六进制打印
info locals：看局部变量
info args：看函数参数
bt：看调用栈
q：退出
n 和 s 的区别，你一定要尽快形成感觉：

n：把函数调用当成一步
s：进入函数内部