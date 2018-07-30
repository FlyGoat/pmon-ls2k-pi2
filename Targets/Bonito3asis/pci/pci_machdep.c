/*	$Id: pci_machdep.c,v 1.1.1.1 2006/09/14 01:59:09 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>

#include <sys/malloc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>

#include <machine/bus.h>

#include "include/bonito.h"

#include <pmon.h>

extern void *pmalloc __P((size_t ));
#if  (PCI_IDSEL_CS5536 != 0)
#include <include/cs5536_pci.h>
extern pcireg_t cs5536_pci_conf_readn(int function, int reg, int width);
extern int cs5536_pci_conf_writen(int function, int reg, int width, pcireg_t value);
#endif
extern void *pmalloc __P((size_t ));

extern int _pciverbose;

extern char hwethadr[6];
struct pci_device *_pci_bus[16];
int _max_pci_bus = 0;


/* PCI mem regions in PCI space */

/* soft versions of above */
static pcireg_t pci_local_mem_pci_base;


/****************************/
/*initial PCI               */
/****************************/

int
_pci_hwinit(initialise, iot, memt)
	int initialise;
	bus_space_tag_t iot;
	bus_space_tag_t memt;
{
	/*pcireg_t stat;*/
	struct pci_device *pd;
	struct pci_bus *pb;
	int newcfg=0;
	if(getenv("newcfg"))newcfg=1;

	if (!initialise) {
		return(0);
	}

	/*
	 *  Allocate and initialize PCI bus heads.
	 */

	/*
	 * PCI Bus 0
	 */
	pd = pmalloc(sizeof(struct pci_device));
	pb = pmalloc(sizeof(struct pci_bus));
	if(pd == NULL || pb == NULL) {
		printf("pci: can't alloc memory. pci not initialized\n");
		return(-1);
	}

	pd->pa.pa_flags = PCI_FLAGS_IO_ENABLED | PCI_FLAGS_MEM_ENABLED;
	pd->pa.pa_iot = pmalloc(sizeof(bus_space_tag_t));
	pd->pa.pa_iot->bus_reverse = 1;
	pd->pa.pa_iot->bus_base = BONITO_PCIIO_BASE_VA;
	//printf("pd->pa.pa_iot=%p,bus_base=0x%x\n",pd->pa.pa_iot,pd->pa.pa_iot->bus_base);
	pd->pa.pa_memt = pmalloc(sizeof(bus_space_tag_t));
	pd->pa.pa_memt->bus_reverse = 1;
	//pd->pa.pa_memt->bus_base = PCI_LOCAL_MEM_PCI_BASE;
	pd->pa.pa_memt->bus_base = 0xb0000000;
	pd->pa.pa_dmat = &bus_dmamap_tag;
	pd->bridge.secbus = pb;
	_pci_head = pd;

#ifdef LS3_HT /* whd */
	pb->minpcimemaddr  = BONITO_PCILO1_BASE;
	pb->nextpcimemaddr = BONITO_PCILO1_BASE+BONITO_PCILO_SIZE;
	pb->minpciioaddr   = PCI_IO_SPACE_BASE+0x000b000;
	pb->nextpciioaddr  = PCI_IO_SPACE_BASE+ BONITO_PCIIO_SIZE;
	pb->pci_mem_base   = BONITO_PCILO_BASE_VA;
	pb->pci_io_base    = BONITO_PCIIO_BASE_VA;
#else
	if(newcfg)
	{
	pb->minpcimemaddr  = BONITO_PCILO1_BASE;//??????,???ܺ?linux??????ʼ??ַһ??,????xwindow????ʾ??????
	pb->nextpcimemaddr = BONITO_PCIHI_BASE; 
	pd->pa.pa_memt->bus_base = 0xa0000000;
	BONITO_PCIMAP =
	    BONITO_PCIMAP_WIN(0, PCI_MEM_SPACE_PCI_BASE+0x00000000) |	
	    BONITO_PCIMAP_WIN(1, PCI_MEM_SPACE_PCI_BASE+0x14000000) |
	    BONITO_PCIMAP_WIN(2, PCI_MEM_SPACE_PCI_BASE+0x18000000) |
	    BONITO_PCIMAP_PCIMAP_2;
	}
	else
	{
	/*pci??ַ??cpu??ַ????,????pci memʱҪioreamapת??,ֱ??or 0xb0000000*/
	pb->minpcimemaddr  = 0x04000000;
	pb->nextpcimemaddr = 0x08000000; 
	pd->pa.pa_memt->bus_base = 0xb0000000;
	BONITO_PCIMAP =
	    BONITO_PCIMAP_WIN(0, PCI_MEM_SPACE_PCI_BASE+0x00000000) |	
	    BONITO_PCIMAP_WIN(1, PCI_MEM_SPACE_PCI_BASE+0x04000000) |
	    BONITO_PCIMAP_WIN(2, PCI_MEM_SPACE_PCI_BASE+0x08000000) |
	    BONITO_PCIMAP_PCIMAP_2;
	}
	pb->minpciioaddr  = 0x0004000;
	pb->nextpciioaddr = BONITO_PCIIO_SIZE;
	pb->pci_mem_base   = BONITO_PCILO_BASE_VA;
	pb->pci_io_base    = BONITO_PCIIO_BASE_VA;
#endif

	pb->max_lat = 255;
	pb->fast_b2b = 1;
	pb->prefetch = 1;
	pb->bandwidth = 4000000;
	pb->ndev = 1;
	_pci_bushead = pb;
	_pci_bus[_max_pci_bus++] = pd;

	
	bus_dmamap_tag._dmamap_offs = 0x80000000;

/*set pci base0 address and window size*/
	pci_local_mem_pci_base = 0x80000000;
#ifdef LS3_HT
#else
	BONITO_PCIBASE0 = 0x80000000;
#endif
#if 0
	BONITO_PCIBASE1 = 0;
	BONITO(BONITO_REGBASE + 0x50) = 0x8000000c;
	BONITO(BONITO_REGBASE + 0x54) = 0xffffffff;
   /*set master1's window0 to map pci 2G->DDR 0 */
	  asm(".set mips3;dli $2,0x900000003ff00000;li $3,0x80000000;sd $3,0x60($2);sd $0,0xa0($2);dli $3,0xffffffff80000000;sd $3,0x80($2);.set mips0" :::"$2","$3");

	/* 
	 * PCI to local mapping: [8M,16M] -> [8M,16M]
	 */
	BONITO_PCI_REG(0x18) = 0x00800000; 
	BONITO_PCI_REG(0x1c) = 0x0;
	BONITO(BONITO_REGBASE + 0x58) = 0xff80000c;
	BONITO(BONITO_REGBASE + 0x5c) = 0xffffffff;
	/*set pci 8-16M -> DDR 8-16M ,window size 8M,can not map 0-8M to pci,because ddr pci address will cover vga mem.*/
	  asm(".set mips3;dli $2,0x900000003ff00000;li $3,0x800000;sd $3,0x68($2);sd $3,0xa8($2);dli $3,0xffffffffff800000;sd $3,0x88($2);.set mips0" :::"$2","$3");

	{
//can not change gnt to break pci transfer when device's gnt not deassert for some sb like 82371,via686b.
	volatile int *p=0xffffffffbfe00168;
#ifdef CONFIG_SLOW_PCI_FOR_BROKENDEV
	*p=0x00fe0115; //make default pci device not cpu itselt,this will make two cpu accesses to pci has about 5 pci clocks inteval for broken device like sundance net adaptor,otherwise maybe only one pci clock inteval.
#else
	*p=0x00fe0105;
#endif
//make pci retry max 32.
    //p=0xffffffffbfe00058; *p=*p|0x2000;
	}
#endif
	return(1);
}

/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible. We also set up the interrupt controller
 * routing and level control registers.
 */
void
_pci_hwreinit (void)
{
}

void
_pci_flush (void)
{
}

/*
 *  Map the CPU virtual address of an area of local memory to a PCI
 *  address that can be used by a PCI bus master to access it.
 */
vm_offset_t
_pci_dmamap(va, len)
	vm_offset_t va;
	unsigned int len;
{
#if 0
	return(VA_TO_PA(va) + bus_dmamap_tag._dmamap_offs);
#endif
	return(pci_local_mem_pci_base + VA_TO_PA (va));
}


#if 1
/*
 *  Map the PCI address of an area of local memory to a CPU physical
 *  address.
 */
vm_offset_t
_pci_cpumap(pcia, len)
	vm_offset_t pcia;
	unsigned int len;
{
	return PA_TO_VA(pcia - pci_local_mem_pci_base);
}
#endif


/*
 *  Make pci tag from bus, device and function data.
 */
pcitag_t
_pci_make_tag(bus, device, function)
	int bus;
	int device;
	int function;
{
	pcitag_t tag;

	tag = (bus << 16) | (device << 11) | (function << 8);
	return(tag);
}

/*
 *  Break up a pci tag to bus, device function components.
 */
void
_pci_break_tag(tag, busp, devicep, functionp)
	pcitag_t tag;
	int *busp;
	int *devicep;
	int *functionp;
{
	if (busp) {
		*busp = (tag >> 16) & 255;
	}
	if (devicep) {
		*devicep = (tag >> 11) & 31;
	}
	if (functionp) {
		*functionp = (tag >> 8) & 7;
	}
}

int
_pci_canscan (pcitag_t tag)
{
	int bus, device, function;

	_pci_break_tag (tag, &bus, &device, &function); 
	if((bus == 0 ) && device == 0) {
		return(0);		/* Ignore the Discovery itself */
	}
	return (1);
}


#if 0
pcireg_t
_pci_conf_read(pcitag_t tag,int reg)
{
	return _pci_conf_readn(tag,reg,4);
}

pcireg_t
_pci_conf_readn(pcitag_t tag, int reg, int width)
{
    u_int32_t addr, type;
    unsigned long ht_conf_rd_addr;
    pcireg_t data;
    int bus, device, function;

    if ((reg & (width-1)) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: bad reg 0x%x\n", reg);
	return ~0;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return ~0;		/* device out of range */
	addr = (1 << (device+11)) | (function << 8) | reg;
//#define LS3_HT
#ifdef LS3_HT
	addr = ((device << 11)) | (function << 8) | reg;
#endif
	type = 0x00000;
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return ~0;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	type = 0x10000;
    }
#ifdef LS3_HT
    if(bus == 0)    
       ht_conf_rd_addr = 0xba000000;
    else
       ht_conf_rd_addr = 0xbb000000;


    ht_conf_rd_addr = ht_conf_rd_addr | addr;
    data = *(volatile pcireg_t *)(ht_conf_rd_addr);
    //if(bus != 0)  printf("ht_conf_rd_addr[%X]! : %8x \n",ht_conf_rd_addr,data);

    return data;
#endif

#if  (PCI_IDSEL_CS5536 != 0)
    if( (bus == 0) && (device == PCI_IDSEL_CS5536) && (reg < 0xf0) ){
     	return cs5536_pci_conf_readn(function, reg, width);
     }
#endif
 
    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16) | type;

    data = *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));

    /* move data to correct position */
    data = data >> ((addr & 3) << 3);

    if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
#if 0
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: reg=%x master abort\n", reg);
#endif
	return ~0;
    }

    if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: target abort\n");
	return ~0;
    }

    return data;
}
void
_pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
	return _pci_conf_writen(tag,reg,data,4);
}

void
_pci_conf_writen(pcitag_t tag, int reg, pcireg_t data,int width)
{
    u_int32_t addr, type;
    unsigned long ht_conf_rd_addr;
    int bus, device, function;

    if ((reg &(width-1)) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: bad reg %x\n", reg);
	return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return;		/* device out of range */
	addr = (1 << (device+11)) | (function << 8) | reg;
#ifdef LS3_HT
	addr = (device << 11) | (function << 8) | reg;
#endif
	type = 0x00000;
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	type = 0x10000;
    }

#ifdef LS3_HT
    if(bus == 0)    
       ht_conf_rd_addr = 0xba000000;
    else
       ht_conf_rd_addr = 0xbb000000;

    ht_conf_rd_addr = ht_conf_rd_addr | addr;
    //printf("ht_conf_rd_addr[%X]!\n",ht_conf_rd_addr);

    *(volatile pcireg_t *)(ht_conf_rd_addr) = data;

    return;
#endif

#if  (PCI_IDSEL_CS5536 != 0)
    if( (bus == 0) && (device == PCI_IDSEL_CS5536) & (reg < 0xf0)){
    	if(cs5536_pci_conf_writen(function, reg, width, data)){
		printf("cs5536 write error.\n");
 	}
 	return;
     }
#endif

    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16)|type;

#if 0
    *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc)) = data;
#else
    {
      pcireg_t ori = *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));
      pcireg_t mask = 0x0;

      if (width == 2) {
	if (addr & 3) mask = 0xffff; 
	else mask = 0xffff0000;
      }else if (width == 1) {
	if ((addr & 3) == 1) {
	  mask = 0xffff00ff;
	}else if ((addr & 3) == 2) {
	  mask = 0xff00ffff;
	}else if ((addr & 3) == 3) {
	  mask = 0x00ffffff;
	}else{
	  mask = 0xffffff00;
	}
      }

      data = data << ((addr & 3) << 3);
      data = (ori & mask) | data;
      *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc)) = data;
    }
#endif

    if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: master abort\n");
    }

    if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: target abort\n");
    }
}
#endif


void
pci_sync_cache(p, adr, size, rw)
	void *p;
	vm_offset_t adr;
	size_t size;
	int rw;
{
	CPU_IOFlushDCache(adr, size, rw);
}

