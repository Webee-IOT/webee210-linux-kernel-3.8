#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <plat/regs-nand.h>
#include <plat/nand.h>
#include "../mtdcore.h"

static unsigned long *clk_gate_ip1;
static unsigned long *clk_gate_block;
static unsigned long *mp0_3con;

static struct mtd_partition webee_nand_part[] = {
	[0] = {
		.name	= "webee_uboot",
		.size	= SZ_1M,
		.offset	= 0,
	},
	[1] = {
		.name	= "webee_kernel",
		.size	= 5*SZ_1M,
		.offset	= MTDPART_OFS_APPEND,
	},
	[2] = {
		.name	= "webee_rootfs",
		.size	= MTDPART_SIZ_FULL,
		.offset	= MTDPART_OFS_APPEND,
	},
};

struct nand_regs {
	unsigned long nfconf;
	unsigned long nfcont;
	unsigned long nfcmmd;
	unsigned long nfaddr;
	unsigned long nfdata;
	unsigned long nfmeccd0;
	unsigned long nfmeccd1;
	unsigned long nfseccd;
	unsigned long nfsblk;
	unsigned long nfeblk;
	unsigned long nfstat;
	unsigned long nfeccerr0;
	unsigned long nfeccerr1;
};

static struct nand_regs *nand_regs;
static struct nand_chip *webee_nand_chip;
static struct mtd_info *webee_nand_mtd;

static void webee_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
		/*ȡ��ѡ��*/
		nand_regs->nfcont |= (1<<1);
	}
	else
	{
		/*ѡ��оƬ*/
		nand_regs->nfcont &= ~(1<<1);
	}
}

static void webee_nand_cmd_ctrl(struct mtd_info *mtd, int dat,
				unsigned int ctrl)
{

	if (ctrl & NAND_CLE)
	{
		/*������*/
		nand_regs->nfcmmd = dat;
	}
	else
	{
		/*����ַ*/
		nand_regs->nfaddr = dat;
	}
}

static int webee_nand_dev_ready(struct mtd_info *mtd)
{
	/*�ȴ�����Ĳ������*/
	return (nand_regs->nfstat & (1<<0));
}

static int webee_nand_init(void)
{
	/*1.����һ��nand_chip�ṹ��*/
	webee_nand_chip = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
	nand_regs = ioremap(0xB0E00000,sizeof(struct nand_regs));
	mp0_3con         = ioremap(0xE0200320,4);
	clk_gate_ip1     = ioremap(0xE0100464,4);
	clk_gate_block = ioremap(0xE0100480,4);

	/*2.����*/
	/*
	 * ��ʼ��nand_chip�ṹ���еĺ���ָ��
	 * �ṩѡ��оƬ�����������ַ�������ݣ�д���ݣ��ȴ��Ȳ���
	 */
	webee_nand_chip->select_chip    = webee_nand_select_chip;
	webee_nand_chip->cmd_ctrl        = webee_nand_cmd_ctrl;
	webee_nand_chip->IO_ADDR_R   = &nand_regs->nfdata;
	webee_nand_chip->IO_ADDR_W  = &nand_regs->nfdata;
	webee_nand_chip->dev_ready     = webee_nand_dev_ready;
	//webee_nand_chip->ecc.mode      = NAND_ECC_SOFT;
	webee_nand_chip->ecc.mode      = NAND_ECC_NONE;
	
	/*3.Ӳ�����*/
	/*ʹ��ʱ��*/
	*clk_gate_ip1     = 0xffffffff;
	*clk_gate_block = 0xffffffff;

	/* ������ӦGPIO�ܽ�����Nand */
	*mp0_3con = 0x22222222;

	/* ����ʱ�� */
#define TWRPH1    1
#define TWRPH0    1
#define TACLS        1
	nand_regs->nfconf |= (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);

	/*
	 * AddrCycle[1]:1 = ���͵�ַ��Ҫ5������
	 */
	nand_regs->nfconf |= 1<<1;

	/*
	 * MODE[0]:1     = ʹ��Nand Flash������
	 * Reg_nCE0[1]:1 = ȡ��Ƭѡ
	 */
	nand_regs->nfcont |= (1<<1)|(1<<0);
		
	/*4.ʹ��*/
	webee_nand_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	webee_nand_mtd->owner = THIS_MODULE;
	webee_nand_mtd->priv = webee_nand_chip;

	nand_scan(webee_nand_mtd, 1);

	/*5.��ӷ���*/
	add_mtd_partitions(webee_nand_mtd, webee_nand_part, 3);

	return 0;
}

static void webee_nand_exit(void)
{
	kfree(webee_nand_mtd);
	iounmap(nand_regs);
	kfree(webee_nand_chip);
}

module_init(webee_nand_init);
module_exit(webee_nand_exit);

MODULE_LICENSE("GPL");

