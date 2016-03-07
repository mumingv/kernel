/*
 *  linux/mykernel/mypcb.h
 *
 *  Kernel internal PCB types
 *
 *  Copyright (C) 2013  Mengning
 *
 */

#define MAX_TASK_NUM        4  //最大进程数
#define KERNEL_STACK_SIZE   1024*8  //进程堆栈大小

/* CPU-specific state of this task */
struct Thread {
    unsigned long		ip;  //ip，存放下一条待执行指令的地址
    unsigned long		sp;  //sp，堆栈指针
};

// 进程控制块
typedef struct PCB{
    int pid;  //进程ID
    volatile long state;  //进程状态, 取值: -1 unrunnable, 0 runnable, >0 stopped
    char stack[KERNEL_STACK_SIZE];  //进程堆栈
    /* CPU-specific state of this task */
    struct Thread thread;  //进程的IP和SP指针
    unsigned long	task_entry;  //任务入口函数地址
    struct PCB *next;  //进程控制块指针，将所有的进程控制块连接起来
}tPCB;

// 待调度的函数，其地址赋给PCB中的task_entry字段
void my_schedule(void);

