#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/workqueue.h>

#include <asm/io.h>
#include <asm/div64.h>
#include <asm/uaccess.h>

#include <asm/mach/map.h>
//#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <linux/fb.h>

#define VSPW       9   //4
#define VBPD       13  //17
#define LINEVAL    479  
#define VFPD       21  //26

#define HSPW       19    //4
#define HBPD       25   //40
#define HOZVAL     799   
#define HFPD       209   //214

#define LeftTopX     0
#define LeftTopY     0
#define RightBotX   799
#define RightBotY   479

static struct fb_info *lcd_test_info;
//static long unsigned long *gpbcon;
static long unsigned long *gpf0con;
static long unsigned long *gpf1con;
static long unsigned long *gpf2con;
static long unsigned long *gpf3con;
static long unsigned long *gpd0con;
static long unsigned long *gpd0dat;
static long unsigned long *display_control;

/*lcd registers*/
static long unsigned long *vidcon0;
static long unsigned long *vidcon1;
static long unsigned long *vidtcon2;
static long unsigned long *vidtcon3;
static long unsigned long *wincon0;
static long unsigned long *wincon2;
static long unsigned long *shadowcon;
static long unsigned long *vidosd0a;
static long unsigned long *vidosd0b;
static long unsigned long *vidosd0c;
static long unsigned long *vidw00add0b0;
static long unsigned long *vidw00add1b0;
static long unsigned long *vidw00add2;
static long unsigned long *vidtcon0;
static long unsigned long *vidtcon1;

static u32  pseudo_palette[16];

struct clk		*simon_clk;
//static struct lcd_con_regs *lcd_con;

/* from pxafb.c */
static  unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int lcd_test_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	unsigned int val;
	
	if (regno > 16) 
		return 1;
	/* 用red,green,blue三原色构造出val */
	val  = chan_to_field(red,   &info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,  &info->var.blue);

	pseudo_palette[regno] = val;
		
	return 0;	
}

static struct fb_ops lcd_test_fbops = 
{
	.owner		= THIS_MODULE,
	
	.fb_setcolreg	= lcd_test_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static int  __init lcd_test_init(void)
{
	
	/* 可变的参数 */
	lcd_test_info = framebuffer_alloc(0 , NULL);
	strcpy(lcd_test_info->fix.id, "s5pv210_lcd");
	//lcd_test_info->fix.smem_start = /*显存物理地址*/
	lcd_test_info->fix.smem_len = 800 * 480 * 32/8;
	lcd_test_info->fix.type = FB_TYPE_PACKED_PIXELS;
	lcd_test_info->fix.visual = FB_VISUAL_TRUECOLOR;
	lcd_test_info->fix.line_length = 800 * 32/8;

	
	/*固定的参数*/
	lcd_test_info->var.xres = 800;
	lcd_test_info->var.yres = 480;
	lcd_test_info->var.xres_virtual   = 800;
	lcd_test_info->var.yres_virtual   = 480;
	lcd_test_info->var.bits_per_pixel = 32;
	/*RGB:888*/
	lcd_test_info->var.red.offset = 16;
	lcd_test_info->var.red.length = 8;
	
	lcd_test_info->var.green.offset = 8;
	lcd_test_info->var.green.length = 8;
	
	lcd_test_info->var.blue.offset = 0;
	lcd_test_info->var.blue.length = 8;
	
	//lcd_test_info->var.blue.msb_right = 1;
	lcd_test_info->var.activate = FB_ACTIVATE_NOW	;

	/*设置操作参数*/
	lcd_test_info->fbops = &lcd_test_fbops;
	/*其他操作*/
	//lcd_test_info->screen_base = ;	/* Virtual address 显存物理地址*/
	lcd_test_info->screen_size =  800 * 480 * 32/8;	/* Amount of ioremapped VRAM or 0 */

	lcd_test_info->pseudo_palette = pseudo_palette;		/* Fake palette of 16 colors */ 

lcd_test_info->screen_base = dma_alloc_writecombine(NULL,  lcd_test_info->fix.smem_len, (u32*)&(lcd_test_info->fix.smem_start), GFP_KERNEL);

	/* 硬件相关操作 */
#if 1
	simon_clk = clk_get(NULL, "lcd");
	if (!simon_clk || IS_ERR(simon_clk)) {
		printk(KERN_INFO "failed to get lcd clock source\n");
	}
	clk_enable(simon_clk);
#endif
	gpf0con = ioremap(0xE0200120, 4);
	gpf1con = ioremap(0xE0200140, 4);
	gpf2con = ioremap(0xE0200160, 4);
	gpf3con = ioremap(0xE0200180, 4);
	gpd0con = ioremap(0xE02000A0, 4);
	gpd0dat = ioremap(0xE02000A4, 4);
	display_control = ioremap(0xe0107008, 4);

	vidcon0 = ioremap(0xF8000000, 4);
	vidcon1 = ioremap(0xF8000004, 4);
	vidtcon2 = ioremap(0xF8000018, 4);
	vidtcon3 = ioremap(0xF800001c, 4);
	wincon0 = ioremap(0xF8000020, 4);
	wincon2 = ioremap(0xF8000028, 4);
	shadowcon = ioremap(0xF8000034, 4);
	vidosd0a = ioremap(0xF8000040, 4);
	vidosd0b = ioremap(0xF8000044, 4);
	vidosd0c = ioremap(0xF8000048, 4);
	vidw00add0b0 = ioremap(0xF80000A0, 4);
	vidw00add1b0 = ioremap(0xF80000D0, 4);
	vidw00add2 = ioremap(0xF8000100, 4);
	vidtcon0 = ioremap(0xF8000010, 4);
	vidtcon1 = ioremap(0xF8000014, 4);
	

	*gpf0con = 0x22222222;
	*gpf1con = 0x22222222;
	*gpf2con = 0x22222222;
	*gpf3con = 0x22222222;
	*gpd0con |= 1<<4;
	*gpd0dat |= 1<<1;
	*display_control  = 2<<0;
	
	
	*vidcon0 &= ~((3<<26) | (1<<18) | (0xff<<6)  | (1<<2));     /* RGB I/F, RGB Parallel format,  */
	*vidcon0 |= ((5<<6) | (1<<4) );

	*vidcon1 &= ~(1<<7);   /* 在vclk的下降沿获取数据 */
	*vidcon1 |= ((1<<6) | (1<<5));  /* HSYNC极性反转, VSYNC极性反转 */

	*vidtcon0 = (VBPD << 16) | (VFPD << 8) | (VSPW << 0);
	*vidtcon1 = (HBPD << 16) | (HFPD << 8) | (HSPW << 0);
	*vidtcon2 = (LINEVAL << 11) | (HOZVAL << 0);
	*wincon0 &= ~(0xf << 2);
	*wincon0 |= (0xB<<2)|(1<<15);
	*vidosd0a = (LeftTopX<<11) | (LeftTopY << 0);
	*vidosd0b = (RightBotX<<11) | (RightBotY << 0);
	*vidosd0c = (LINEVAL + 1) * (HOZVAL + 1);

    *vidw00add0b0 = lcd_test_info->fix.smem_start;  
    *vidw00add1b0 = lcd_test_info->fix.smem_start + lcd_test_info->fix.smem_len;  

	*shadowcon = 0x1; /* 使能通道0 */
	*vidcon0  |= 0x3; /* 开启总控制器 */
	*wincon0 |= 1;     /* 开启窗口0 */
	
	/* bit[17:8]: VCLK = HCLK / [(CLKVAL+1) x 2], LCD手册P14
	 *            10MHz(100ns) = 100MHz / [(CLKVAL+1) x 2]
	 *            CLKVAL = 4
	 * bit[6:5]: 0b11, TFT LCD
	 * bit[4:1]: 0b1100, 16 bpp for TFT
	 * bit[0]  : 0 = Disable the video output and the LCD control signal.
	 */
	 
	/*rLCDCON1 = (2<<8)|(3<<5)|(12<<1)|0;	
	  *rLCDCON2 = (1<<24)|(271<<14)|(1<<6)|(9);
	  *rLCDCON3 = (1<<19)|(479<<8)|(1);
	  *rLCDCON4 = (40);
    	  *rLCDCON5 = (1<<11)|(1<<10)|(1<<9)|(1<<8)|(0<<7)|(0<<6)|(1<<3)|(0<<1)|(1); 
	  */

	
	/*注册*/
	register_framebuffer(lcd_test_info);

	return 0;
}
static void __exit lcd_test_exit(void)
{
	unregister_framebuffer(lcd_test_info);
	//lcd_con->lcdcon1 &= ~(1<<0); /*失能LCD 控制器*/
	//lcd_con->lcdcon5 &= ~(1<<3); /*关闭LCD 控制器*/
	dma_free_writecombine(NULL,  lcd_test_info->fix.smem_len, lcd_test_info->screen_base, lcd_test_info->fix.smem_start);
//	iounmap(gpccon);
//	iounmap(gpdcon);
//	iounmap(gpgcon);
//	iounmap(lcd_con);
	framebuffer_release(lcd_test_info);
}

module_init(lcd_test_init);
module_exit(lcd_test_exit);
MODULE_LICENSE("GPL");


