#include <pmon.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/malloc.h>
#include <machine/pio.h>
#include <sys/device.h>
#include <target/ls2k.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <pmon/dev/pcibrvar.h>

#define DC_FB0 1	//mtf modify

#define DIS_WIDTH  FB_XSIZE
#define DIS_HEIGHT FB_YSIZE
#define EXTRA_PIXEL  0

#define DC0_BASE_ADDR_OFF	0x1240
#define DC1_BASE_ADDR_OFF	0x1250

#define RANDOM_HEIGHT_Z 37

static char *ADDR_CURSOR = 0xaeff0000;
static char *MEM_ptr = 0xae800000;	/* frame buffer address register on ls2h mem */

static int MEM_ADDR = 0;

struct vga_struc {
	float pclk;
	int hr, hss, hse, hfl;
	int vr, vss, vse, vfl;
} vgamode[] = {
	{	28.56,	640,	664,	728,	816,	480,	481,	484,	500,	},	/*"640x480_70.00" */	
	{	33.10,	640,	672,	736,	832,	640,	641,	644,	663,	},	/*"640x640_60.00" */	
	{	39.69,	640,	672,	736,	832,	768,	769,	772,	795,	},	/*"640x768_60.00" */	
	{	42.13,	640,	680,	744,	848,	800,	801,	804,	828,	},	/*"640x800_60.00" */	
	{	35.84,	800,	832,	912,	1024,	480,	481,	484,	500,	},	/*"800x480_70.00" */	
	{	38.22,	800,	832,	912,	1024,	600,	601,	604,	622,	},	/*"800x600_60.00" */	
	{	40.73,	800,	832,	912,	1024,	640,	641,	644,	663,	},	/*"800x640_60.00" */	
	{	40.01,	832,	864,	952,	1072,	600,	601,	604,	622,	},	/*"832x600_60.00" */	
	{	40.52,	832,	864,	952,	1072,	608,	609,	612,	630,	},	/*"832x608_60.00" */	
	{	45.98,	960,	864,	952,	1072,	600,	609,	612,	630,	},	/*"960x600_60.00" */	
	{	38.17,	1024,	1048,	1152,	1280,	480,	481,	484,	497,	},	/*"1024x480_60.00" */	
	{	48.96,	1024,	1064,	1168,	1312,	600,	601,	604,	622,	},	/*"1024x600_60.00" */	
	{	52.83,	1024,	1072,	1176,	1328,	640,	641,	644,	663,	},	/*"1024x640_60.00" */	
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */	
	{	71.38,	1152,	1208,	1328,	1504,	764,	765,	768,	791,	},	/*"1152x764_60.00" */	
	{	83.46,	1280,	1344,	1480,	1680,	800,	801,	804,	828,	},	/*"1280x800_60.00" */	
	{	108.88,	1280,	1360,	1496,	1712,	1024,	1025,	1028,	1060,	},	/*"1280x1024_60.00" */	
	{	85.86,	1368,	1440,	1584,	1800,	768,	769,	772,	795,	},	/*"1368x768_60.00" */	
	{	93.80,	1440,	1512,	1664,	1888,	800,	801,	804,	828,	},	/*"1440x800_60.00" */	
	{	120.28,	1440,	1528,	1680,	1920,	900,	901,	904,	935,	},	/*"1440x900_67.00" */
};

enum {
	OF_BUF_CONFIG = 0,
	OF_BUF_ADDR = 0x20,
	OF_BUF_STRIDE = 0x40,
	OF_BUF_ORIG = 0x60,
	OF_DITHER_CONFIG = 0x120,
	OF_DITHER_TABLE_LOW = 0x140,
	OF_DITHER_TABLE_HIGH = 0x160,
	OF_PAN_CONFIG = 0x180,
	OF_PAN_TIMING = 0x1a0,
	OF_HDISPLAY = 0x1c0,
	OF_HSYNC = 0x1e0,
	OF_VDISPLAY = 0x240,
	OF_VSYNC = 0x260,
	OF_DBLBUF = 0x340,
};

int caclulatefreq(float PCLK)
{
	int pstdiv, ODF, LDF, idf, inta, intb;
	int mpd, modf, mldf, rodf;
	int out;
	float a, b, c, min;

	min = 100;
	for (pstdiv = 1; pstdiv < 32; pstdiv++)
		for (ODF = 1; ODF <= 8; ODF *= 2) {
			a = PCLK * pstdiv;
			b = a * ODF;
			LDF = ((int)(b / (50 * ODF))) * ODF;
			intb = 50 * LDF;
			inta = intb / ODF;
			c = b - intb;
			if (inta < 75 || inta > 1800)
				continue;
			if (intb < 600 || intb > 1800)
				continue;
			if (LDF < 8 || LDF > 225)
				continue;
			if (c < min) {
				min = c;
				mpd = pstdiv;
				modf = ODF;
				mldf = LDF;
			}
		}
	rodf = modf == 8 ? 3 : modf == 4 ? 2 : modf == 2 ? 1 : 0;
	out = (mpd << 24) + (mldf << 16) + (rodf << 5) + (5 << 2) + 1;
	printf("ODF=%d, LDF=%d, IDF=5, pstdiv=%d, prediv=1\n", rodf, mldf, mpd);
	return out;
}

int config_cursor(unsigned long base)
{
	/* framebuffer Cursor Configuration */
	outl(base + LS2H_FB_CUR_CFG_REG, 0x00020200);
	/* framebuffer Cursor Address */
	outl(base + LS2H_FB_CUR_ADDR_REG, ADDR_CURSOR);
	/* framebuffer Cursor Location */
	outl(base + LS2H_FB_CUR_LOC_ADDR_REG, 0x00060122);
	/* framebuffer Cursor Background */
	outl(base + LS2H_FB_CUR_BACK_REG, 0x00eeeeee);
	outl(base + LS2H_FB_CUR_FORE_REG, 0x00aaaaaa);
}

int config_fb(unsigned long base)
{
	int i, mode = -1;
	int j;
	unsigned int chip_reg;

	for (i = 0; i < sizeof(vgamode) / sizeof(struct vga_struc); i++) {
		int out;
		if (vgamode[i].hr == FB_XSIZE && vgamode[i].vr == FB_YSIZE) {
			mode = i;
			out = caclulatefreq(vgamode[i].pclk);
			printf("out=%x\n", out);
		}//if
	}

	if (mode < 0) {
		printf("\n\n\nunsupported framebuffer resolution\n\n\n");
		return;
	}

	/* Disable the panel 0 */
	outl((base + OF_BUF_CONFIG), 0x00000000);
	/* framebuffer configuration RGB565 */
	outl((base + OF_BUF_CONFIG), 0x00000003);
	outl((base + OF_BUF_ADDR), MEM_ADDR);
	outl(base + OF_DBLBUF, MEM_ADDR);
	outl((base + OF_DITHER_CONFIG), 0x00000000);
	outl((base + OF_DITHER_TABLE_LOW), 0x00000000);
	outl((base + OF_DITHER_TABLE_HIGH), 0x00000000);
	outl((base + OF_PAN_CONFIG), 0x80001311);
	outl((base + OF_PAN_TIMING), 0x00000000);

	outl((base + OF_HDISPLAY),
		  (vgamode[mode].hfl << 16) | vgamode[mode].hr);
	outl((base + OF_HSYNC),
		  0x40000000 | (vgamode[mode].hse << 16) | vgamode[mode].hss);
	outl((base + OF_VDISPLAY),
		  (vgamode[mode].vfl << 16) | vgamode[mode].vr);
	outl((base + OF_VSYNC),
		  0x40000000 | (vgamode[mode].vse << 16) | vgamode[mode].vss);

#if defined(CONFIG_VIDEO_32BPP)
	outl((base + OF_BUF_CONFIG), 0x00100104);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 4 + 255) & ~255);
#elif defined(CONFIG_VIDEO_16BPP)
	outl((base + OF_BUF_CONFIG), 0x00100103);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#elif defined(CONFIG_VIDEO_15BPP)
	outl((base + OF_BUF_CONFIG), 0x00100102);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#elif defined(CONFIG_VIDEO_12BPP)
	outl((base + OF_BUF_CONFIG), 0x00100101);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#else /* 640x480-32Bits */
	outl((base + OF_BUF_CONFIG), 0x00100104);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 4 + 255) & ~255);
#endif /* 32Bits */

}

int dc_init()
{
	int print_count;
	int i;
	int PIXEL_COUNT = DIS_WIDTH * DIS_HEIGHT + EXTRA_PIXEL;
	int MEM_SIZE;
	int init_R = 0;
	int init_G = 0;
	int init_B = 0;
	int j;
	int ii = 0, tmp = 0;
	int MEM_SIZE_3 = MEM_SIZE / 6;

	int line_length = 0;

	int print_addr;
	int print_data;
	unsigned int val;
	printf("enter dc_init...\n");

#if defined(CONFIG_VIDEO_32BPP)
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = FB_XSIZE * 4;
#elif defined(CONFIG_VIDEO_16BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_15BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_12BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#else
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = FB_XSIZE * 4;
#endif

	MEM_ADDR = (long)MEM_ptr & 0x0fffffff;
	if (MEM_ptr == NULL) {
		printf("frame buffer memory malloc failed!\n ");
		exit(0);
	}

	for (ii = 0; ii < 0x1000; ii += 4)
		outl(ADDR_CURSOR + ii, 0x88f31f4f);

	ADDR_CURSOR = (long)ADDR_CURSOR & 0x0fffffff;
	printf("frame buffer addr: %x \n", MEM_ADDR);
	/* Improve the DC DMA's priority */
//mtf	outb(LS2H_QOS_CFG6_REG + 6, 0x36);
	/* Make DVO from panel1, it's the same with VGA*/

	val = pci_read_type0_config32(6, 0, 16);

	val &= 0xffff0000;
	val |= _pci_bus[0]->pa.pa_memt->bus_base;
	printf("val %x\n", val);
	config_fb(val + DC0_BASE_ADDR_OFF);	//for dvo_0 1240
	config_fb(val + DC1_BASE_ADDR_OFF);	//for dvo_1 1250
	config_cursor(val);

	printf("display controller reg config complete!\n");

	return MEM_ptr;
}

static int cmd_dc_freq(int argc, char **argv)
{
	int out;
	long sysclk;
	float pclk;
	int j;
	unsigned int chip_reg;
	if (argc < 2)
		return -1;
	pclk = strtoul(argv[1], 0, 0);
	if (argc > 2)
		sysclk = strtoul(argv[2], 0, 0);
	else
		sysclk = 33333;
	out = caclulatefreq(pclk);
	printf("out=%x\n", out);
	/* change to refclk */
#ifdef DC_FB0
	outl(LS2H_PIXCLK0_CTRL1_REG, 1);
#else
	outl(LS2H_PIXCLK1_CTRL1_REG, 1);
#endif
	/* pll_powerdown set pstdiv */
#ifdef DC_FB0
	outl(LS2H_PIXCLK0_CTRL0_REG, out | 0x80000080);
#else
	outl(LS2H_PIXCLK1_CTRL0_REG, out | 0x80000080);
#endif
	/* wait 10000ns */
	for (j = 1; j <= 200; j++)
		chip_reg = inl(LS2H_CHIP_SAMP0_REG);
	/* pll_powerup unset pstdiv */
#ifdef DC_FB0
	outl(LS2H_PIXCLK0_CTRL0_REG, out);
#else
	outl(LS2H_PIXCLK1_CTRL0_REG, out);
#endif
	/* wait pll_lock */
	while (inb(LS2H_CHIP_SAMP0_REG) & 0x00001800 != 0x00001800); 

	/* change to pllclk */
#ifdef DC_FB0
	outl(LS2H_PIXCLK0_CTRL1_REG, 0);
#else
	outl(LS2H_PIXCLK1_CTRL1_REG, 0);
#endif
	return 0;
}

static const Cmd Cmds[] = {
	{"MyCmds"},
	{"dc_freq", " pclk sysclk", 0, "config dc clk(khz)", cmd_dc_freq, 0, 99,
	 CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
