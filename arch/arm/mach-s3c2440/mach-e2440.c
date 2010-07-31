/* linux/arch/arm/mach-s3c2440/mach-e2440.c
 *
 * Copyright (c) 2008 Ramax Lo <ramaxlo@gmail.com>
 *      Based on mach-anubis.c by Ben Dooks <ben@simtec.co.uk>
 *      and modifications by SBZ <sbz@spgui.org> and
 *      Weibing <http://weibing.blogbus.com> and
 *      Michel Pollet <buserror@gmail.com>
 *
 * For product information, visit http://code.google.com/p/e2440/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/mmc/host.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/hardware.h>
#include <mach/fb.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <mach/leds-gpio.h>
#include <mach/regs-mem.h>
#include <mach/regs-lcd.h>
#include <mach/irqs.h>
#include <plat/nand.h>
#include <plat/iic.h>
#include <plat/mci.h>
#include <plat/udc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>

#include <sound/s3c24xx_uda134x.h>
#include <mach/e2440-map.h>

static struct map_desc e2440_iodesc[] __initdata = {
	{
		.virtual	= (u32)E2440_VA_CS8900A_BASE,
		.pfn		= __phys_to_pfn(E2440_PA_CS8900A_BASE),
		.length		= SZ_1M,
		.type		= MT_DEVICE,
	}
};

#define UCON S3C2410_UCON_DEFAULT
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE


static struct s3c2410_uartcfg e2440_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
};

/* USB device UDC support */

static void e2440_udc_pullup(enum s3c2410_udc_cmd_e cmd)
{
	pr_debug("udc: pullup(%d)\n", cmd);

	switch (cmd) {
		case S3C2410_UDC_P_ENABLE :
			s3c2410_gpio_setpin(S3C2410_GPC(5), 1);
			break;
		case S3C2410_UDC_P_DISABLE :
			s3c2410_gpio_setpin(S3C2410_GPC(5), 0);
			break;
		case S3C2410_UDC_P_RESET :
			break;
		default:
			break;
	}
}

static struct s3c2410_udc_mach_info e2440_udc_cfg __initdata = {
	.udc_command		= e2440_udc_pullup,
};



/* NAND Flash on E2440 board */

static struct mtd_partition e2440_default_nand_part[] __initdata = {
	[0] = {
		.name	= "u-boot",
		.size	= SZ_256K,
		.offset	= 0,
	},
	[1] = {
		.name	= "u-boot-env",
		.size	= SZ_128K,
		.offset	= SZ_256K,
	},
	[2] = {
		.name	= "kernel",
		/* 5 megabytes, for a kernel with no modules
		 * or a uImage with a ramdisk attached */
		.size	= 0x00500000,
		.offset	= SZ_256K + SZ_128K,
	},
	[3] = {
		.name	= "root",
		.offset	= SZ_256K + SZ_128K + 0x00500000,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct s3c2410_nand_set e2440_nand_sets[] __initdata = {
	[0] = {
		.name		= "e2440-nand",
		.nr_chips	= 1,
//		.nr_partitions	= ARRAY_SIZE(e2440_default_nand_part),
//		.partitions	= e2440_default_nand_part,
		.flash_bbt 	= 1, /* we use u-boot to create a BBT */
	},
};

static struct s3c2410_platform_nand e2440_nand_info __initdata = {
	.tacls		= 0,
	.twrph0		= 25,
	.twrph1		= 15,
	.nr_sets	= ARRAY_SIZE(e2440_nand_sets),
	.sets		= e2440_nand_sets,
	.ignore_unset_ecc = 1,
};

static struct platform_device *e2440_devices[] __initdata = {
	&s3c_device_nand,
	&s3c_device_sdi,
};

static void __init e2440_map_io(void)
{
	s3c24xx_init_io(e2440_iodesc, ARRAY_SIZE(e2440_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(e2440_uartcfgs, ARRAY_SIZE(e2440_uartcfgs));
}

/*
 * e2440_features string
 *
 * t = Touchscreen present
 * b = backlight control
 * c = camera [TODO]
 * 0-9 LCD configuration
 *
 */
static char e2440_features_str[12] __initdata = "0tb";

static int __init e2440_features_setup(char *str)
{
	if (str)
		strlcpy(e2440_features_str, str, sizeof(e2440_features_str));
	return 1;
}

__setup("e2440=", e2440_features_setup);

#define FEATURE_SCREEN (1 << 0)
#define FEATURE_BACKLIGHT (1 << 1)
#define FEATURE_TOUCH (1 << 2)
#define FEATURE_CAMERA (1 << 3)

struct e2440_features_t {
	int count;
	int done;
	int lcd_index;
	struct platform_device *optional[8];
};


static void __init e2440_init(void)
{
//	struct e2440_features_t features = { 0 };
	int i;

//	printk(KERN_INFO "MINI2440: Option string e2440=%s\n",
//			e2440_features_str);

	/* Parse the feature string */
//	e2440_parse_features(&features, e2440_features_str);

	/* turn LCD on */
//	s3c2410_gpio_cfgpin(S3C2410_GPC(0), S3C2410_GPC0_LEND);

	/* Turn the backlight early on */
//	s3c2410_gpio_setpin(S3C2410_GPG(4), 1);
//	s3c2410_gpio_cfgpin(S3C2410_GPG(4), S3C2410_GPIO_OUTPUT);

	/* remove pullup on optional PWM backlight -- unused on 3.5 and 7"s */
//	s3c2410_gpio_pullup(S3C2410_GPB(1), 0);
//	s3c2410_gpio_setpin(S3C2410_GPB(1), 0);
//	s3c2410_gpio_cfgpin(S3C2410_GPB(1), S3C2410_GPIO_INPUT);

	/* Make sure the D+ pullup pin is output */
//	s3c2410_gpio_cfgpin(S3C2410_GPC(5), S3C2410_GPIO_OUTPUT);

	/* mark the key as input, without pullups (there is one on the board) */
//	for (i = 0; i < ARRAY_SIZE(e2440_buttons); i++) {
//		s3c2410_gpio_pullup(e2440_buttons[i].gpio, 0);
//		s3c2410_gpio_cfgpin(e2440_buttons[i].gpio,
//					S3C2410_GPIO_INPUT);
//	}
//	if (features.lcd_index != -1) {
//		int li;
//
//		e2440_fb_info.displays =
//			&e2440_lcd_cfg[features.lcd_index];
//
//		printk(KERN_INFO "MINI2440: LCD");
//		for (li = 0; li < ARRAY_SIZE(e2440_lcd_cfg); li++)
//			if (li == features.lcd_index)
//				printk(" [%d:%dx%d]", li,
//					e2440_lcd_cfg[li].width,
//					e2440_lcd_cfg[li].height);
//			else
//				printk(" %d:%dx%d", li,
//					e2440_lcd_cfg[li].width,
//					e2440_lcd_cfg[li].height);
//		printk("\n");
//		s3c24xx_fb_set_platdata(&e2440_fb_info);
//	}

//	s3c24xx_udc_set_platdata(&e2440_udc_cfg);
//	s3c24xx_mci_set_platdata(&e2440_mmc_cfg);
	s3c_nand_set_platdata(&e2440_nand_info);
	s3c_i2c0_set_platdata(NULL);

//	i2c_register_board_info(0, e2440_i2c_devs,
//				ARRAY_SIZE(e2440_i2c_devs));

	platform_add_devices(e2440_devices, ARRAY_SIZE(e2440_devices));

//	if (features.count)	/* the optional features */
//		platform_add_devices(features.optional, features.count);

}


MACHINE_START(E2440, "E2440")
	/* Maintainer: Michel Pollet <buserror@gmail.com> */
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,
	.map_io		= e2440_map_io,
	.init_machine	= e2440_init,
	.init_irq	= s3c24xx_init_irq,
	.timer		= &s3c24xx_timer,
MACHINE_END
