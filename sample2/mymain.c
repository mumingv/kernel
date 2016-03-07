/*
 *  linux/mykernel/mymain.c
 *
 *  Kernel internal my_start_kernel
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

// 进程控制块数组，共4个进程
tPCB task[MAX_TASK_NUM];
// 指向当前正在执行的进程控制块
tPCB * my_current_task = NULL;

volatile int my_need_sched = 0;

void my_process(void);

// 系统启动后调用的第一个函数
void __init my_start_kernel(void)
{
    int pid = 0;
    int i;

    /**
     * 0号进程初始化
     */
    task[pid].pid = pid;
    task[pid].state = 0;  //将0号线程置为运行状态
    // 将任务入口地址置为my_process函数
    task[pid].task_entry = (unsigned long)my_process;
    // 将eip置为my_process函数
    task[pid].thread.ip = (unsigned long)my_process;
    // 堆栈指针指向当前堆栈的栈底
    task[pid].thread.sp = (unsigned long)&task[pid].stack[KERNEL_STACK_SIZE-1];
    // 目前只有一个0号进程，next指向自己
    task[pid].next = &task[pid];

    /**
     * 初始化其他3个进程(1号、2号、3号进程)
     */
    for(i=1;i<MAX_TASK_NUM;i++)
    {
        // 先将已经初始化好的0号进程控制块拷贝过来, 再基于此进行修改
        memcpy(&task[i],&task[0],sizeof(tPCB));
        // 1号、2号、3号进程的PID分别为1、2、3
        task[i].pid = i;
        // 进程状态置为未运行（待运行）
        task[i].state = -1;
        // 堆栈指针指向自己进程堆栈的栈底
        task[i].thread.sp = (unsigned long)&task[i].stack[KERNEL_STACK_SIZE-1];
        // 链表尾部插入节点，形成循环链表: 0->1->2->3->0
        task[i].next = task[i-1].next;
        task[i-1].next = &task[i];
    }

    /**
     * 启动0号进程
     */
    pid = 0;
    my_current_task = &task[pid];

    // 下面是内嵌汇编代码，是本程序的核心, 目的是将%ebp, %esp, %eip都附上值
    // 1. asm和__asm__是等价的，asm是别名，为了书写方便;
    // 2. volatile和__volatile__是等价的，volatile是别名；volatile用于指示编译器不要优化代码，后面的指令保持原样;
	asm volatile(
        // 3. %0~%2分别对应下面冒号后面的输出部分和输入部分；
        // 4. %%esp有两个%, 前面一个%用于转义; \n\t也是转义字符；
        // 5. %esp指向栈底，即指向&task[pid].stack[KERNEL_STACK_SIZE-1]
    	"movl %1,%%esp\n\t" 	/* set task[pid].thread.sp to esp */
        // 6. 将%esp压栈，等同于: pushl %esp
    	"pushl %1\n\t" 	        /* push ebp */
        // 7. 将my_process函数入口地址入栈
    	"pushl %0\n\t" 	        /* push task[pid].thread.ip */
        // 8. ret = popl %eip，即%eip指向my_process函数入口地址
    	"ret\n\t" 	            /* pop task[pid].thread.ip to eip */
        // 9. %ebp指向&task[pid].stack[KERNEL_STACK_SIZE-1]
    	"popl %%ebp\n\t"
        // 10. 输出部分：为空
    	: 
        // 11. 输入部分：分别对应%0和%1
        // 12. 这里的前置修饰符, "c"表示寄存器%ecx, "d"表示寄存器%edx；
    	: "c" (task[pid].thread.ip),"d" (task[pid].thread.sp)	/* input c or d mean %ecx/%edx*/
	);
    // 汇编代码总结：
    // a. 此处的汇编代码主要目的就是将%ebp, %esp指向进程堆栈的栈底；将%eip指向my_process函数入口地址;
    // b. 个人理解，只要将%ebp, %esp, %eip这三个寄存器直接使用movl命令赋上值就好了，没必要这么多汇编语句。
}

/**
 * 进程执行的第一个函数
 */
void my_process(void)
{
    int i = 0;
    while(1)
    {
        i++;
        // 每10000000次，触发一次操作
        if(i%10000000 == 0)
        {
            printk(KERN_NOTICE "this is process %d -\n",my_current_task->pid);

            // 这里需要等时钟中断程序将my_need_sched的值改为1, 才能触发一次my_schedule函数
            if(my_need_sched == 1)
            {
                my_need_sched = 0;
        	    my_schedule();
        	}
        	printk(KERN_NOTICE "this is process %d +\n",my_current_task->pid);
        }     
    }
}
