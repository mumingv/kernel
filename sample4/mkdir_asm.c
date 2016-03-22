#include <stdio.h>

int main() {
    int ret = 0;
    char *dir = "./testdir";
    int mode = 0777;
    asm volatile(
        "movl %1, %%ebx\n\t"
        "movl %2, %%ecx\n\t"
        "movl $39, %%eax\n\t"
        "int $0x80\n\t"  //触发软中断，进行系统调用
        "movl %%eax, %0\n\t"
        : "=m"(ret)  //对应%0，=m表示可写内存
        : "m"(dir), "m"(mode)  //对应%1和%2，m表示内存
    );
    printf("ret is: %d.\n", ret);
    return 0;
}

