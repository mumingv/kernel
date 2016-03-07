/*
 *  linux/mykernel/myinterrupt.c
 *
 *  Kernel internal my_timer_handler
 *
 *  Copyright (C) 2013  Mengning
 *
 */
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/tty.h>
#include <linux/vmalloc.h>

#include "mypcb.h"

extern tPCB task[MAX_TASK_NUM];
extern tPCB * my_current_task;
extern volatile int my_need_sched;

volatile int time_count = 0;

/*
 * Called by timer interrupt.
 * it runs in the name of current running process,
 * so it use kernel stack of current running process
 */
void my_timer_handler(void)
{
    if(time_count%1000 == 0 && my_need_sched != 1)
    {
        printk(KERN_NOTICE ">>>my_timer_handler here<<<\n");
        // 该变量置为1，my_process函数中就可以调用my_schedule函数了
        my_need_sched = 1;
    } 
    time_count ++ ;  
    return;  	
}

/**
 * 定时调度函数, 调度间隔在my_timer_handler函数中指定
 */
void my_schedule(void)
{
    tPCB * next;
    tPCB * prev;

    // my_current_task在初始时指向0号进程的进程控制块, 肯定不为空
    // my_current_task的next也不会为NULL，因为是循环链表，即使只有一个0号进程控制块，也是自己指向自己的
    if(my_current_task == NULL 
        || my_current_task->next == NULL)
    {
    	return;
    }
    printk(KERN_NOTICE ">>>my_schedule<<<\n");

    // 进程切换
    /* schedule */
    next = my_current_task->next;
    prev = my_current_task;
    // next节点的状态初始值为-1
    if(next->state == 0)/* -1 unrunnable, 0 runnable, >0 stopped */
    {
        // next进程第二次(及之后所有次)执行时走该分支, 这里的注释假设系统第二次从0号进程切换到1号进程
    	/* switch to next process */
    	asm volatile(
        	"pushl %%ebp\n\t" 	    /* save ebp */
        	"movl %%esp,%0\n\t" 	/* save esp */
        	"movl %2,%%esp\n\t"     /* restore  esp */
            // 这里与第一次调度该进程相比少了一行"movl %2,%%ebp\n\t"
            // 因为第一次调用1号进程的时候, 已经将2号进程的堆栈基址压栈了，
            // 在第二次调用1号进程的时候直接从堆栈中取出来就行了
            // 注：这里的%1f和中断没有关系，是进程上下文切换的特殊地址
        	"movl $1f,%1\n\t"       /* save eip */	
        	"pushl %3\n\t" 
        	"ret\n\t" 	            /* restore  eip */
            // 这里的"1"是标号，一般共指令跳转使用，本汇编程序未使用
        	"1:\t"                  /* next process start here */
            // 将第一次调用1号进程时压入的%ebp弹出堆栈
        	"popl %%ebp\n\t"
        	: "=m" (prev->thread.sp),"=m" (prev->thread.ip)
        	: "m" (next->thread.sp),"m" (next->thread.ip)
    	); 
    	my_current_task = next; 
    	printk(KERN_NOTICE ">>>switch %d to %d<<<\n",prev->pid,next->pid);   	
    }
    else
    {
        // next进程第一次执行时走该分支, 这里的注释假设系统第一次从0号进程切换到1号进程
        
        // 设置1号进程状态为运行中
        next->state = 0;
        // 当前指针指向1号进程
        my_current_task = next;
        printk(KERN_NOTICE ">>>switch %d to %d<<<\n",prev->pid,next->pid);
        // 将%ebp, %esp, %eip的值修改为1号进程的对应值, 即启动2号进程的执行
    	asm volatile(
            // 1. %ebp入0号进程的堆栈
        	"pushl %%ebp\n\t" 	    /* save ebp */
            // 2. %esp保存到0号进程的控制块中
        	"movl %%esp,%0\n\t" 	/* save esp */
            // 3. %esp指向1号进程的栈底
        	"movl %2,%%esp\n\t"     /* restore  esp */
            // 4. %ebp也指向1号进程的栈底
        	"movl %2,%%ebp\n\t"     /* restore  ebp */
            // 5. %eip保存到0号进程的控制块中
            // 注：这里的%1f和中断没有关系，是进程上下文切换的特殊地址
        	"movl $1f,%1\n\t"       /* save eip */	
            // 6. 将1号进程的入口函数地址(和0号进程相同)压入1号进程的栈
        	"pushl %3\n\t" 
            // 7. ret = popl %eip, 即：将%eip指向1号进程的入口函数，并开始执行
        	"ret\n\t" 	            /* restore  eip */
            // 8. 输出部分，"m"表示内存变量，"="表示操作数在指令中是只写的
        	: "=m" (prev->thread.sp),"=m" (prev->thread.ip)
            // 9. 输入部分，"m"表示内存变量
        	: "m" (next->thread.sp),"m" (next->thread.ip)
    	);          
    }   
    return;	
}

