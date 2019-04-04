/* PLL Settings For Loongson-2K1000 */

#define SOFT_CLKSEL
#ifdef SOFT_CLKSEL

#ifndef DDR_FREQ
/* Default MEM @ 400Mhz */
#define DDR_FREQ   400
#endif

#ifndef CORE_FREQ
/* Default CPU @ 800Mhz */
#define CORE_FREQ   800
#endif

#define L1_REFC     4
#define L1_DIV      1
#define L2_DIV      2
#define L1_LOOPC  (CORE_FREQ*L1_DIV*L2_DIV*L1_REFC/100)


#define DDR_REFC   4
#define DDR_DIV    1
#define DDR_DIV_L2    4
#define DDR_LOOPC  (DDR_FREQ*DDR_REFC*DDR_DIV*DDR_DIV_L2/100)
/* GPU @ 400Mhz, HDA @ 24Mhz */
#define GPU_DIV_L2    (400*DDR_REFC*DDR_DIV/DDR_LOOPC/100)
#define HDA_DIV_L2    (24*DDR_REFC*DDR_DIV/DDR_LOOPC/100)

/* DC @ 250Mhz, GMAC @ 125Mhz */
#define DC_LOOPC    100
#define DC_REFC     4
#define DC_DIV      1
#define DC_DIV_L2   10
#define GMAC_DIV    20

#define PIX0_LOOPC	109
#define PIX0_REFC	5
#define PIX0_DIV	1
#define PIX0_DIV_L2	20

#define PIX1_LOOPC	109
#define PIX1_REFC	5
#define PIX1_DIV	1
#define PIX1_DIV_L2	20
#define BYPASS_CORE 0x0
#define BYPASS_NODE 0x0
#define BYPASS_L1   0x0

#define PLL_CHANG_COMMIT 0x1

#define BYPASS_REFIN 		(0x1 << 0)

#define PLL_L1_LOCKED 		(0x1 << 16)
#define PLL_L1_ENA		(0x1 << 2)

#define PLL_MEM_ENA		(0x1 << 2)
#define PLL_MEM_LOCKED 		(01 << 16)

	TTYDBG ("LS2K Soft CLK SEL adjust begin\r\n")

	li     	t0, 0xbfe10480
	li	t1, (0x1 << 19) 	//power down pll L1 first
	sd	t1, 0x0(t0)
	dli	t1, (L1_LOOPC << 32) | (L1_DIV << 42) | (L1_REFC << 26) | (0x3 << 10) | (0x1 << 7)
	dli	t2, L2_DIV
	sd	t1, 0(t0)
	sd	t2, 8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)
	
wait_core_pll_lock:
	ld      a0, 0x0(t0)
	li      a1, PLL_L1_LOCKED
	and     a0, a1, a0
	beqz    a0, wait_core_pll_lock
	nop

	ld      a0, 0x0(t0)
	ori     a0, a0, PLL_CHANG_COMMIT
	sd      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nMEM        :")

	li      t0, 0xbfe10490
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (DDR_DIV << 42) | (DDR_REFC << 26) | (DDR_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, (GPU_DIV_L2 << 22) | (DDR_DIV_L2) | (HDA_DIV_L2 << 44)
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

wait_ddr_pll_lock:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, wait_ddr_pll_lock
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x3
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nDC        :")

	li      t0, 0xbfe104a0
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (DC_DIV << 42) | (DC_REFC << 26) | (DC_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, (GMAC_DIV << 22) | (DC_DIV_L2)
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

wait_dc_pll_lock:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, wait_dc_pll_lock
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x3
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nPIX0        :")

	li      t0, 0xbfe104b0		//pll_pix0
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (PIX0_DIV << 42) | (PIX0_REFC << 26) | (PIX0_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, PIX0_DIV_L2
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

wait_pix0_pll_lock:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, wait_pix0_pll_lock
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x1
	sw      a0, 0x0(t0)

	bal     hexserial
	nop

	TTYDBG ("\r\nPIX1        :")

	li      t0, 0xbfe104c0		//pll_pix1
	li	t1, (0x1 << 19) 	//power down pll  first
	sd	t1, 0x0(t0)
	dli     t1, (PIX1_DIV << 42) | (PIX1_REFC << 26) | (PIX1_LOOPC << 32) | (0x3 << 10) | (0x1 << 7)
	dli	t2, PIX1_DIV_L2
	sd	t1, 0x0(t0)
	sd	t2, 0x8(t0)
	ori	t1, PLL_L1_ENA
	sd      t1, 0x0(t0)

wait_pix1_pll_lock:
	lw      a0, 0x0(t0)
	li      a1, PLL_MEM_LOCKED
	and     a0, a0, a1
	beqz    a0, wait_pix1_pll_lock
	nop

	lw      a0, 0x0(t0)
	ori     a0, a0, 0x1
	sw      a0, 0x0(t0)

	bal     hexserial
	nop


#endif
