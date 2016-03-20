#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() {
    int ret = 0;
    ret = mkdir("./testdir", 0777);
    printf("ret is: %d.\n", ret);
    return 0;
}

