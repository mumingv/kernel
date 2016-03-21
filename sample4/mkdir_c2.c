#include <stdio.h>
#include <sys/syscall.h>

int main() {
    int ret = 0;
    // 下面两种写法是完全一样的，SYS_mkdir是一个宏定义，表示系统调用号39
    if (1) {
        ret = syscall(SYS_mkdir, "./testdir", 0777);
    } else {
        ret = syscall(39, "./testdir", 0777);
    }
    printf("ret is: %d.\n", ret);
    return 0;
}

