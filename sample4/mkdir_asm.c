#include <stdio.h>

int main() {
    int ret = 0;
    char *dir = "./testdir";
    int mode = 0777;
    asm volatile(
        "movl %1, %%ebx\n\t"
        "movl %2, %%ecx\n\t"
        "movl $39, %%eax\n\t"
        "int $0x80\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(ret)
        : "m"(dir), "m"(mode)
    );
    printf("ret is: %d.\n", ret);
    return 0;
}

