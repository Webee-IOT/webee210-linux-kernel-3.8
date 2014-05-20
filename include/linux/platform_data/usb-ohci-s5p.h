/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __MACH_S5P_OHCI_H
#define __MACH_S5P_OHCI_H

/**************Add by Webee*******************/

#ifdef CONFIG_S5P_DEV_USB_EHCI
static struct s5p_ehci_platdata s5p_ehci_platdata;
#endif
//#ifdef CONFIG_S5P_DEV_USB_OHCI
static struct s5p_ohci_platdata s5p_ohci_platdata;
//#endif

/**************Add by Webee*******************/

struct s5p_ohci_platdata {
	int (*phy_init)(struct platform_device *pdev, int type);
	int (*phy_exit)(struct platform_device *pdev, int type);
};

extern void s5p_ohci_set_platdata(struct s5p_ohci_platdata *pd);

#endif /* __MACH_S5P_OHCI_H */
