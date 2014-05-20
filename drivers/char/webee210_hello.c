/*
 * Name:Webee210_hello.c
 * Copyright (C) 2014 Webee.JY  (2483053468@qq.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>


/* �����������ں��� */            
static int __init Webee210_hello_init(void)
{
    printk(KERN_WARNING "Webee210_hello_init!\n");
	printk(KERN_WARNING "Hello,world!\n");
    return 0;
}

/* ��������ĳ��ں��� */            
static void __exit Webee210_hello_exit(void)
{
   printk(KERN_WARNING "Webee210_hello_exit!\n");
   printk(KERN_WARNING "Goodbye,world!\n");
}                                    

/* �����������/���ں��������仰˵���൱��
 * �����ں�������������/���ں���������
 */
module_init(Webee210_hello_init);
module_exit(Webee210_hello_exit);

/* ������֧�ֵ�Э�顢���ߡ����� */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("webee");
MODULE_DESCRIPTION("Webee210 Board First module test");