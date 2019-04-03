 /*
  * This file is for 2HSOC hda test.
  */
#if 0
#include "hda.h"
#include "pcie.h"

//#undef HDA_DEBUG
#define HDA_DEBUG
#ifdef HDA_DEBUG
#define hda_dbg(format, arg...) tgt_printf(format "\n", ## arg)
#else
#define hda_dbg(fmt, arg)
#endif

#define ls_writeb(addr,val) *(volatile unsigned char*)(addr) = (val)
#define ls_readb(addr) *(volatile unsigned char*)(addr)
void sd_entry_prepare(int *sd_entry_addr,int *data)
{
	unsigned char i;
	for (i = 0;i < 0x10;i++) {
		hda_dbg("\rsd_entry_addr = 0x%lx\n",sd_entry_addr + i *4);
		hda_ls2h_readl(sd_entry_addr + i * 4) = ((unsigned int)data & 0xfffffff) + i * 0x100;
		hda_ls2h_readl(sd_entry_addr + i * 4 + 2) = 0x1000;
		hda_ls2h_readl(sd_entry_addr + i * 4 + 3) = 0x1;
	}
}

void sd_data_prepare(int * sd_data)
{
	unsigned char i = 0,j;
	unsigned int sd_data_buf[] = {
		0xf0c7ec18, 0xf174ec8b, 0xf2aeed71, 0xf4e8ef61,
		0xf7d7f20c, 0xf99bf3c1, 0xf907f433, 0xf79ef3a3,
		0xf5fcf293, 0xf3fbf03b, 0xf15eed6b, 0xefe3eb72,
		0xefa6eae0, 0xf053eaad, 0xf117eb44, 0xf209eca3,
		0xf1ffedf7, 0xf10fed80, 0xf004ecc3, 0xf051ed27,
		0xf187eea8, 0xf315ef89, 0xf510f0ce, 0xf7bcf32c,
		0xfa86f6ad, 0xfc70fa2b, 0xfd63fd37, 0xfea00022,
		0x013a0406, 0x054808c7, 0x0a740e09, 0x0d2c0faf,
		0x0aa10b30, 0x07980712, 0x061905d2, 0x038e0384,
		0x00a20075, 0xff87ff14, 0x0033ff6e, 0x012dffe9,
		0x01f00052, 0x02cf0154, 0x04cb0338, 0x07f305c2,
		0x0c2609b0, 0x0e730d80, 0x0c2a0ce7, 0x06890874,
		0x028b04f8, 0x00720357, 0xfd810059, 0xf9fdfc2d,
		0xf7d2f895, 0xf6d6f680, 0xf607f605, 0xf5f9f6a6,
		0xf731f7bc, 0xf832f823, 0xf8acf7d1, 0xf8bef72b,
		0xf946f712, 0xf927f73d, 0xf91bf7c5, 0xf9d4f984
	};
	for(i = 0;i < 0x40;i++) {
		hda_dbg("\rsd_data = %x\n",sd_data_buf[i]);
		for(j = 0;j < 0x10;j++)
				hda_ls2h_writel(sd_data + (j * 0x40) + i, sd_data_buf[i]);
	}

}
void hda_test(void)
{
	int *corb_p = (int *)(unsigned int*)malloc(sizeof(int) * 0x1200);
	unsigned int corb_p_tmp = (unsigned int)corb_p | 0xa0000000;
	corb_p = (int *)((unsigned int)corb_p + (0x80 - ((unsigned int)corb_p & 0x7f)));
	int *rirb_p = (int *)(unsigned int)corb_p + 0x400;
	int *sd_bdpl = (int *)(unsigned int)rirb_p + 0x800;

	hda_dbg("\rcorb_bp_tmp = 0x%lx\n",corb_p_tmp);
	hda_dbg("\rcorb_p = 0x%lx\n",corb_p);
	hda_dbg("\rrirb_p = 0x%lx\n",rirb_p);

	//this buffer is used to control the codec.
	static unsigned int corb_buf[] = {
		0x205000a, 0x205000a, 0x2044000, 0x205000a,
		0x205000a, 0x2044000, 0x171601 , 0x171701 ,
		0x171501 , 0x870100 , 0x837000 , 0x970100 ,
		0x937000 , 0xc3b000 , 0xd3b000 , 0xe3b000 ,
		0xf3b000 , 0xc37000 , 0xc37100 , 0xd37000 ,
		0xd37100 , 0xe37000 , 0xe37100 , 0xf37000 ,
		0xf37100 , 0x2637000, 0x2637100, 0x2337000,
		0x2237000, 0x1470740, 0x143b000, 0x1470100,
		0x1670740, 0x163b000, 0x1670102, 0x1570740,
		0x153b000, 0x1570101, 0x1b707c0, 0x1b3b000,
		0x1b70100, 0x1870721, 0x183b080, 0x1970724,
		0x193b080, 0x1a70720, 0x1a3b080, 0x1c70720,
		0x2337000, 0x2337180, 0x2337280, 0x2337380,
		0x2337480, 0x2337580, 0x2337680, 0x2337780,
		0x2337880, 0x2337980, 0x2337a80, 0x2237000,
		0x2237180, 0x2237280, 0x2237380, 0x2237480,
		0x2237580, 0x2237680, 0x2237780, 0x2237880,
		0x2237980, 0x2237a80, 0x1e70740, 0x1f70720,
		0x670650 , 0x624011 , 0x270650 , 0x224011 ,
		0x470650 , 0x424011 , 0x370650 , 0x324011 ,
		0x870610 , 0x824011 , 0xc3a01d , 0xc3901d ,
		0xe3a01d , 0xe3901d , 0xd3a01d , 0xd3901d ,
		0x2336080, 0x2335080, 0x2336100, 0x2335100
	}; 
	hda_dbg("\rcorb_buf = 0x%lx\n",corb_buf);

	tgt_printf("-%x\n",ls_readb(0xbbe20008));
	ls_readb(0xbbe20008) &= (~ICH6_GCTL_RESET);
	tgt_printf("-%x\n",ls_readb(0xbbe20008));
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	tgt_printf("-%lx\n",*(volatile unsigned char*)(0xbbe20008));
        hda_writel(GCTL, hda_readb(GCTL) & ~ICH6_GCTL_RESET);
	delay(0x100000);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
//	hda_ls2h_writel(LS2H_CHIP_CFG0_REG,0x4006030);
        hda_writel(GCTL, hda_readb( GCTL) | ICH6_GCTL_RESET);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writew(STATESTS,0x1);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writew(GSTS,0x2);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writel(INTCTL,0xc0000010);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writel(CORBLBASE,((unsigned int)corb_p & 0xfffffff));
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writeb(CORBCTL,ICH6_CORBCTL_RUN);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writeb(CORBSIZE,0x40);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writel(RIRBLBASE,((unsigned int)rirb_p & 0xfffffff));
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writeb(RIRBCTL,ICH6_RBCTL_DMA_EN);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writeb(RIRBSTS,ICH6_RBSTS_OVERRUN | ICH6_RBSTS_IRQ);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	hda_writeb(RIRBSIZE,0x2);
	tgt_printf("-%s-%d-\n",__func__,__LINE__);
	
	
	//set the sdo registers
	hda_sd_writeb(SD_STS,0x4);
	hda_sd_writel(SD_CBL, 0xffc0);
	hda_sd_writeb(SD_LVI,0xf);
	hda_sd_writew(SD_FORMAT,0x4011);
	hda_sd_writel(SD_BDLPL, ((unsigned int)sd_bdpl & 0xfffffff));
	hda_sd_writel(SD_CTL, 0x50001c);
	int i;
	tgt_printf("------------------------------------------\n");
	for(i = 1;i < 0x5d;i++ ) {
		ls2h_readl(new_addr((unsigned int)(corb_p + i)))= corb_buf[i];
		hda_writew(CORBWP,i);
#ifdef HDA_DEBUG
		tgt_printf("\rcorb %lx\n",corb_buf[i]);
		tgt_printf("\rcorb %lx\n",(unsigned int)(corb_p + i));
//		tgt_printf("\rcorb %x\n",ls2h_readl((corb_p + i)));
		tgt_printf("\rcorb wp %lx\n",hda_readw(CORBWP));
		tgt_printf("\rrirb wp %lx\n",hda_readw(RIRBWP));
#endif
	}

	int *sd_data = (int *)(unsigned int)sd_bdpl + 0x400;

	sd_entry_prepare((int *)new_addr((unsigned int)sd_bdpl),(int *)new_addr((unsigned int)sd_data));

	sd_data_prepare((int *)new_addr((unsigned int)sd_data));

	hda_sd_writel(SD_CTL, 0x50001e);

	tgt_printf("\rHDA TEST IS BEGIN!\n");
	int tmp = 2;
	while(tmp--){
		delay(0x2000000);
		delay(0x2000000);
	}

	hda_sd_writel(SD_CTL, 0x50001c);
	tgt_printf("\rHDA TEST IS END!\n");
	free((int *)corb_p_tmp);
}
#endif

 /*
  * This file is for 2HSOC hda test.
  */
//#include <sys/linux/types.h>
//#include <stdio.h>
//#include <machine/pio.h>
//#include "target/ls2h.h"
//#include "target/board.h"
//#include "target/eeprom.h"

/*
 * registers
 */
#include <pmon.h>
#include "hda.h"
static void sd_entry_prepare(int *sd_entry_addr,int data)
{
	unsigned char i;
	for (i = 0;i < 0x10;i++) {
		//printf("sd_entry_addr = 0x%lx\n",sd_entry_addr + i *4);
	//	ls2h_readl(sd_entry_addr + i * 4) = ((unsigned int)data + i * 0x1000);
		ls2h_readl(sd_entry_addr + i * 4) = data + i * 0x100;
		ls2h_readl(sd_entry_addr + i * 4 + 2) = 0x1000;
		ls2h_readl(sd_entry_addr + i * 4 + 3) = 0x1;
	}
}

static void sd_data_prepare(int * sd_data)
{
	unsigned char i = 0,j,k;

	unsigned int sd_data_buf[] = {
		0xf0c7ec18, 0xf174ec8b, 0xf2aeed71, 0xf4e8ef61,
		0xf7d7f20c, 0xf99bf3c1, 0xf907f433, 0xf79ef3a3,
		0xf5fcf293, 0xf3fbf03b, 0xf15eed6b, 0xefe3eb72,
		0xefa6eae0, 0xf053eaad, 0xf117eb44, 0xf209eca3,
		0xf1ffedf7, 0xf10fed80, 0xf004ecc3, 0xf051ed27,
		0xf187eea8, 0xf315ef89, 0xf510f0ce, 0xf7bcf32c,
		0xfa86f6ad, 0xfc70fa2b, 0xfd63fd37, 0xfea00022,
		0x013a0406, 0x054808c7, 0x0a740e09, 0x0d2c0faf,
		0x0aa10b30, 0x07980712, 0x061905d2, 0x038e0384,
		0x00a20075, 0xff87ff14, 0x0033ff6e, 0x012dffe9,
		0x01f00052, 0x02cf0154, 0x04cb0338, 0x07f305c2,
		0x0c2609b0, 0x0e730d80, 0x0c2a0ce7, 0x06890874,
		0x028b04f8, 0x00720357, 0xfd810059, 0xf9fdfc2d,
		0xf7d2f895, 0xf6d6f680, 0xf607f605, 0xf5f9f6a6,
		0xf731f7bc, 0xf832f823, 0xf8acf7d1, 0xf8bef72b,
		0xf946f712, 0xf927f73d, 0xf91bf7c5, 0xf9d4f984
	};
	
	for(i = 0;i < 0x40;i++) {
		//printf("sd_data = %x\n",sd_data_buf[i]);
		for(j = 0;j < 0x10;j++)
			for(k = 0;k < 0x10;k++)
				ls2h_writel(sd_data + (j * 0x400) + (0x40 * k) + i, sd_data_buf[i]);
	}

}
void hda_test(void)
{
	int *corb_p = (unsigned int*)pmalloc(sizeof(int) * 0x1000);
	corb_p = (unsigned int)corb_p + (0x80 - ((unsigned int)corb_p & 0x7f));
	int *rirb_p = (unsigned int)corb_p + 0x400;
	int *sd_bdpl = (unsigned int)rirb_p + 0x800;
	printf("corb_p = 0x%lx\n",corb_p);
	printf("rirb_p = 0x%lx\n",rirb_p);
	
	//this buffer is used to control the codec.
	static unsigned int corb_buf[] = {
		0xf0000,   0xf0002  , 0xf0000  ,
		0x205000a, 0x205000a, 0x2044000, 0x205000a,
		0x205000a, 0x2044000, 0x171601 , 0x171701 ,
		0x171501 , 0x870100 , 0x837000 , 0x970100 ,
		0x937000 , 0xc3b000 , 0xd3b000 , 0xe3b000 ,
		0xf3b000 , 0xc37000 , 0xc37100 , 0xd37000 ,
		0xd37100 , 0xe37000 , 0xe37100 , 0xf37000 ,
		0xf37100 , 0x2637000, 0x2637100, 0x2337000,
		0x2237000, 0x1470740, 0x143b000, 0x1470100,
		0x1670740, 0x163b000, 0x1670102, 0x1570740,
		0x153b000, 0x1570101, 0x1b707c0, 0x1b3b000,
		0x1b70100, 0x1870721, 0x183b080, 0x1970724,
		0x193b080, 0x1a70720, 0x1a3b080, 0x1c70720,
		0x2337000, 0x2337180, 0x2337280, 0x2337380,
		0x2337480, 0x2337580, 0x2337680, 0x2337780,
		0x2337880, 0x2337980, 0x2337a80, 0x2237000,
		0x2237180, 0x2237280, 0x2237380, 0x2237480,
		0x2237580, 0x2237680, 0x2237780, 0x2237880,
		0x2237980, 0x2237a80, 0x1e70740, 0x1f70720,
		0x670650 , 0x624011 , 0x270650 , 0x224011 ,
		0x470650 , 0x424011 , 0x370650 , 0x324011 ,
		0x870610 , 0x824011 , 0xc3a01d , 0xc3901d ,
		0xe3a01d , 0xe3901d , 0xd3a01d , 0xd3901d ,
		0x2336080, 0x2335080, 0x2336100, 0x2335100
	}; 

        hda_writel(GCTL, hda_readb(GCTL) & ~ICH6_GCTL_RESET);
	delay(0x100000);
	//ls2h_writel(0xbbd00200,0x4006030);
        hda_writel(GCTL, hda_readb( GCTL) | ICH6_GCTL_RESET);
	hda_writew(STATESTS,0x1);
	hda_writew(GSTS,0x2);
	hda_writel(INTCTL,0xc0000010);
	hda_writel(CORBLBASE,(unsigned int)corb_p & 0xfffffff);
	hda_writeb(CORBCTL,ICH6_CORBCTL_RUN);
	hda_writeb(CORBSIZE,0x40);
	hda_writel(RIRBLBASE,(unsigned int)rirb_p & 0xfffffff);
	hda_writeb(RIRBCTL,ICH6_RBCTL_DMA_EN);
	hda_writeb(RIRBSTS,ICH6_RBSTS_OVERRUN | ICH6_RBSTS_IRQ);
	hda_writeb(RIRBSIZE,0x2);
	
	
	//set the sdo registers
	hda_sd_writeb(SD_STS,0x4);
	hda_sd_writel(SD_CBL, 0xffc0);
	hda_sd_writeb(SD_LVI,0xf);
	hda_sd_writew(SD_FORMAT,0x4011);
	hda_sd_writel(SD_BDLPL, (unsigned int)sd_bdpl & 0xfffffff);
	hda_sd_writel(SD_CTL, 0x50001c);
	int i;
	for(i = 1;i < 0x5d;i++ ) {
//		printf("corb %x\n",corb_buf[i]);
		*(volatile unsigned int*)(corb_p + i) = corb_buf[i];
		hda_writew(CORBWP,i);
//		printf("corb %x\n",*(volatile unsigned int*)(corb_p + i));
//		printf("corb wp %x\n",hda_readw(CORBWP));
	}

	int *sd_data = (unsigned int*)pmalloc(sizeof(int) * 0x4020);
	sd_data = (unsigned int)sd_data + (0x80 - ((unsigned int)sd_data & 0x7f));
	printf("sd_data = 0x%lx\n",sd_data);

	sd_entry_prepare(sd_bdpl,(unsigned int)sd_data & 0xfffffff);

	sd_data_prepare(sd_data);
	
	hda_sd_writel(SD_CTL, 0x50001e);

	tgt_printf("\rHDA TEST IS BEGIN!\n");
	int tmp = 2;
	while(tmp--){
		delay(0x200000);
	}

	hda_sd_writel(SD_CTL, 0x50001c);
	tgt_printf("\rHDA TEST IS END!\n");
	free((int *)corb_p);
}

static const Cmd Cmds[] = {
	{"ls2h hda"},
	{"hda", "", 0, "test the hda function", hda_test, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
