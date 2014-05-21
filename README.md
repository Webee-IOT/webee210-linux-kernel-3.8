###参与webee210-linux-kernel-3.8的开发

如果你想参与本__webee210-linux-kernel-3.8__的开发,请用力hit下面的链接:

[webee210-rootfs-qt4.8](https://github.com/iZobs/webee210-rootfs-qt4.8/blob/master/Develop-doc.md)

###关于
这是网蜂webee210的linux 内核，仅适合于webee210v2开发板的使用。该内核基于`kernel-org`的linux-kernel-3.8.0进行修改的。
###UPDATE
__2014-5-18__
- 在make menuconfig时可以选择开启LCD驱动
- 启动linux时显示LOGO 
- 添加了对UVC摄像的驱动的支持

###TODO
- 添加s5pv210的camera-interface驱动
- 添加3G模块的驱动支持
- 添加i2s声卡驱动的支持
- 添加MFC硬件编码的驱动支持

###编译
目录下有不同的配置文件，拿这个'webee_uvc_lcd_defconfig'配置文件来举个例子:

	  @ cp webee_uvc_lcd_defconfig .config
	  @	make uImage -j2
或者你可以在`.config`的基础上，添加自己的配置:

      @ cp webee_uvc_lcd_defconfig .config
	  @	make menuconfig
      @	make uImage -j2

不同的配置文件配置不同，其中`webee_defconfig`的内核配置开启的东西最少，适合用于测试和开发.

