#include <linux/module.h>  
#include <linux/fb.h>  
#include <linux/dma-mapping.h>  
#include <linux/clk.h>  

static struct fb_info *lcd_info;  
unsigned long pseudo_palette[16];
unsigned long *display_control;
volatile unsigned long* gpf0con;
volatile unsigned long* gpf1con;  
volatile unsigned long* gpf2con;  
volatile unsigned long* gpf3con;  
volatile unsigned long* gpd0con;  
volatile unsigned long* gpd0dat;  
volatile unsigned long* vidcon0;  
volatile unsigned long* vidcon1;  
volatile unsigned long* vidtcon0;  
volatile unsigned long* vidtcon1;  
volatile unsigned long* vidtcon2;
volatile unsigned long* wincon0;  
volatile unsigned long* vidosd0a;  
volatile unsigned long* vidosd0b;  
volatile unsigned long* vidosd0c;  
volatile unsigned long* vidw00add0b0;  
volatile unsigned long* vidw00add1b0;  
volatile unsigned long* shodowcon;  
  
struct clk *lcd_clk;  
  
  
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)  
{  
    chan &= 0xffff;  
    chan >>= 16 - bf->length;  
    return chan << bf->offset;  
}  
  
static int lcdfb_setcolreg(unsigned int regno, unsigned int red,  
                 unsigned int green, unsigned int blue,  
                 unsigned int transp, struct fb_info *info)  
{  
    unsigned int val;  
      
    if (regno > 16)  
        return 1;  
  
    /* 用red,green,blue三原色构造出val */  
    val  = chan_to_field(red,   &info->var.red);  
    val |= chan_to_field(green, &info->var.green);  
    val |= chan_to_field(blue,  &info->var.blue);  
      
    //((u32 *)(info->pseudo_palette))[regno] = val;  
    pseudo_palette[regno] = val;  
    return 0;  
}  
  
static struct fb_ops lcd_fbops = {  
    .owner      = THIS_MODULE,  
    .fb_setcolreg   = lcdfb_setcolreg,  
    .fb_fillrect    = cfb_fillrect,  
    .fb_copyarea    = cfb_copyarea,  
    .fb_imageblit   = cfb_imageblit,  
};  
  
static int lcd_init(void){  
    int ret;  
  
    /*分配fb_info */  
    lcd_info = framebuffer_alloc(0, NULL);  
    if(lcd_info == NULL){  
        printk(KERN_ERR "alloc framebuffer failed!\n");  
        return -ENOMEM;  
    }  
  
    /* 配置fb_info各成员*/  
    /* fix */  
    strcpy(lcd_info->fix.id, "s5pv210_lcd");  
    lcd_info->fix.smem_len = 800*480*4;  
    lcd_info->fix.type = FB_TYPE_PACKED_PIXELS;  
    lcd_info->fix.visual = FB_VISUAL_TRUECOLOR;  
    lcd_info->fix.line_length = 800*4;  
  
    /* var */  
    lcd_info->var.xres = 800;  
    lcd_info->var.yres = 480;  
    lcd_info->var.xres_virtual = 800;  
    lcd_info->var.yres_virtual = 480;  
    lcd_info->var.bits_per_pixel = 32;  
  
    lcd_info->var.red.offset = 16;  
    lcd_info->var.red.length = 8;  
    lcd_info->var.green.offset = 8;  
    lcd_info->var.green.length = 8;  
    lcd_info->var.blue.offset = 0;  
    lcd_info->var.blue.length = 8;  
    lcd_info->var.activate = FB_ACTIVATE_NOW;  
  
    lcd_info->screen_size = 800*480*4;  
    lcd_info->pseudo_palette = pseudo_palette;  
  
    lcd_info->fbops = &lcd_fbops;  
    /* 配置硬件资源*/  
    /* 映射内存*/  
    display_control = ioremap(0xe0107008,4);  
    gpf0con      = ioremap(0xE0200120, 4);  
    gpf1con      = ioremap(0xE0200140, 4);  
    gpf2con      = ioremap(0xE0200160, 4);  
    gpf3con      = ioremap(0xE0200180, 4);  
      
    gpd0con      = ioremap(0xE02000A0, 4);  
    gpd0dat      = ioremap(0xE02000A4, 4);  
      
    vidcon0      = ioremap(0xF8000000, 4);  
    vidcon1      = ioremap(0xF8000004, 4);  
    vidtcon0     = ioremap(0xF8000010, 4);  
    vidtcon1     = ioremap(0xF8000014, 4);  
    vidtcon2     = ioremap(0xF8000018, 4);  
    wincon0      = ioremap(0xF8000020, 4);  
    vidosd0a     = ioremap(0xF8000040, 4);  
    vidosd0b     = ioremap(0xF8000044, 4);  
    vidosd0c     = ioremap(0xF8000048, 4);  
    vidw00add0b0 = ioremap(0xF80000A0, 4);  
    vidw00add1b0 = ioremap(0xF80000D0, 4);  
    shodowcon    = ioremap(0xF8000034, 4);  
      
    /* 配置GPIO*/  
    *gpf0con = 0x22222222;  
    *gpf1con = 0x22222222;  
    *gpf2con = 0x22222222;  
    *gpf3con = 0x22222222;  
    *gpd0con &= ~0xf;  
    *gpd0con |= 0x1;  
    *gpd0dat |= 1<<0;  
    *display_control = 2<<0;  
    /* 使能时钟*/  
    lcd_clk = clk_get(NULL, "lcd");  
    if (!lcd_clk || IS_ERR(lcd_clk)) {  
        printk(KERN_INFO "failed to get lcd clock source\n");  
    }  
    clk_enable(lcd_clk);  
      
    /* 配置LCD控制器*/  
    *vidcon0 = (4<<6)|(1<<4);  
    *vidcon1 = (1<<6)|(1<<5)|(1<<4);  
  
    *vidtcon0 = (17<<16)|(26<<8)|(4<<0);  
    *vidtcon1 = (40<<16)|(214<<8)|(4<<0);  
    *vidtcon2 = (479<<11)|(799<<0);  
  
    *wincon0 &= ~(0xf<<2);  
    *wincon0 |= (0xb<<2);  
  
    *vidosd0a = (0<<11)|(0<<0);  
    *vidosd0b = (799<<11)|(479<<0);  
    *vidosd0c = 480*800;  
    //物理地址  
    lcd_info->screen_base = dma_alloc_writecombine(NULL,   
        lcd_info->fix.smem_len, (dma_addr_t *)&(lcd_info->fix.smem_start), GFP_KERNEL);  
      
    *vidw00add0b0 = lcd_info->fix.smem_start;  
    *vidw00add1b0 = lcd_info->fix.smem_start + lcd_info->fix.smem_len;  
  
    *shodowcon = 0x1;  
  
    //开启状态  
    *wincon0 |= 1;  
    *vidcon0 |= 3;  
    /* 注册fb_info */  
    ret = register_framebuffer(lcd_info);  
    return ret;  
}  
  
static void lcd_exit(void){  
    unregister_framebuffer(lcd_info);  
    dma_free_writecombine(NULL, lcd_info->fix.smem_len,   
        (void*)lcd_info->screen_base, (dma_addr_t)lcd_info->fix.smem_start);  
  
    iounmap(shodowcon);  
    iounmap(vidw00add1b0);  
    iounmap(vidw00add0b0);  
    iounmap(vidosd0c);  
    iounmap(vidosd0b);  
    iounmap(vidosd0a);  
    iounmap(wincon0);  
    iounmap(vidtcon2);  
    iounmap(vidtcon1);  
    iounmap(vidtcon0);  
    iounmap(vidcon1);  
    iounmap(vidcon0);  
    iounmap(gpd0dat);  
    iounmap(gpd0con);  
    iounmap(gpf3con);  
    iounmap(gpf2con);  
    iounmap(gpf1con);  
    iounmap(gpf0con);  
    framebuffer_release(lcd_info);  
}  
  
module_init(lcd_init);  
module_exit(lcd_exit);  
MODULE_LICENSE("GPL");
