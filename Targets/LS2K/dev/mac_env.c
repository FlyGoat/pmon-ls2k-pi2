// SPDX-License-Identifier: GPL-2.0+
/*
* Get MAC address from NVRAM env
* Copyright (C) Jiaxun Yang <jiaxun.yang@flygoat.com> 2019
*
*/
#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls2k.h"
#include "target/board.h"
#include "target/eeprom.h"
#include "target/bonito.h"
#include "generate_mac_val.c"

#include "pflash.h"
#include "dev/pflash_tgt.h"

extern struct fl_map *tgt_flashmap();

char *nvram;
unsigned char mac_syn0[6];
unsigned char mac_syn1[6];


void i2c_init(void) {
    printf("Target have no EEPROM, getting MAC address from flash\n");
    nvram = (char *)(tgt_flashmap())->fl_map_base + NVRAM_OFFS;

    /* Copy MAC from NVRAM to buffer, and check if it's valid. */
    bcopy(&nvram + SYN0_MAC_OFFS, &mac_syn0, 6);
    if(!is_valid_ether_addr_linux(&mac_syn0)){
        unsigned char syn0_def[6] = {0x00, 0x50, 0xC2, 0x1D, 0x30, 0x0F};
        printf("Invalid syn0 mac, fallback to default\n");
        mac_write(0x0, &syn0_def, 6);
    }

    bcopy(&nvram + SYN1_MAC_OFFS, &mac_syn1, 6);
    if(!is_valid_ether_addr_linux(&mac_syn1)){
        unsigned char syn1_def[6] = {0x00, 0x50, 0xC2, 0x1D, 0x30, 0x10};
        printf("Invalid syn1 mac, fallback to default\n");
        mac_write(0x6, &syn1_def, 6);
    }

    printf("syn0 mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_syn0[0], mac_syn0[1],
		mac_syn0[2], mac_syn0[3], mac_syn0[4], mac_syn0[5]);

    printf("syn1 mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_syn1[0], mac_syn1[1],
		mac_syn1[2], mac_syn1[3], mac_syn1[4], mac_syn1[5]);
    return;
}

int mac_read(unsigned char data_addr, unsigned char *buf, int count){
    if(count <= 0 || count > 6)
        return -1;

    /* data_addr is hardcoded, syn0 is 0x0, syn1 is 0x6 */
    if(data_addr == 0x0){
        /* bcopy(source, dest, size) not memcpy */
        bcopy(&mac_syn0, buf, count);
        return 0;
    } else if(data_addr == 0x6) {
        bcopy(&mac_syn1, buf, count);
        return 0;
    }
    return -1;
}

int mac_write(unsigned char data_addr, unsigned char *buf, int count){
    if(count <= 0 || count > 6)
        return -1;

    /* data_addr is hardcoded, syn0 is 0x0, syn1 is 0x6 */
    if(data_addr == 0x0){
        /* bcopy(source, dest, size) not memcpy */
        bcopy(buf, &mac_syn0, count);
        tgt_flashprogram(nvram + SYN0_MAC_OFFS, 6, (char *)&mac_syn0, 0);
        return count;
    } else if(data_addr == 0x6) {
        bcopy(buf, &mac_syn1, count);
        tgt_flashprogram(nvram + SYN1_MAC_OFFS, 6, (char *)&mac_syn1, 0);
        return count;
    }
    return -1;
}

int cmd_setmac(int ac, unsigned char *av[])
{
	int i, j, v, count, param = 0;
	unsigned char *s = NULL;
	unsigned char data_addr;
	unsigned char buf[32] = {0};

	switch (ac) {
		case 1:
		case 2:
			param = 1;
			break;
		case 3:
			break;
		default:
			goto warning;
	}

	if (param == 1) {
		for (i = 0; i < 2; i++) {	
			if (mac_read(i * 6, buf, 6) == 6) {
				if (!is_valid_ether_addr_linux(buf)){
					printf("syn%d Mac is invalid, now get a new mac\n", i);
					generate_mac_val(buf);
					if (mac_write((i * 6), buf, 6) == 6) {
						printf("set syn%d  Mac address: ",i);
						for (v = 0;v < 6;v++)
							printf("%2x%s",*(buf + v) & 0xff,(5-v)?":":" ");
						printf("\n");
						printf("The machine should be restarted to make the new mac change to take effect!!\n");
						} else
							printf("eeprom write error!\n");
					printf("you can set it by youself\n");
				} else {
					printf("syn%d Mac address: ", i);
					for (j = 0; j < 6; j++)
						printf("%02x%s", buf[j], (5-j)?":":" ");
					printf("\n");
				}
			} else {
				printf("eeprom write error!\n");
				return 0;
			}
		}
		goto warning;
	}

	if (av[2])
        s = av[2];
	else
        goto warning; 

	count = strlen(s) / 3 + 1;
	if (count - 6) goto warning;

	for (i = 0; i < count; i++) {
		gethex(&v, s, 2); 
		buf[i] = v;
		s += 3;
	}

	data_addr = strtoul(av[1] + 3, NULL, 0);
	data_addr *= 6;

	if (mac_write(data_addr, buf, count) == count) {
		printf("set syn%d  Mac address: %s\n",data_addr / 6, av[2]);
		printf("The machine should be restarted to make the mac change to take effect!!\n");
	} else 
		printf("eeprom write error!\n");
	return 0;
warning:
	printf("Please accord to correct format.\nFor example:\n");
	printf("\tsetmac syn1 \"00:11:22:33:44:55\"\n");
	printf("\tThis means set syn1's Mac address 00:11:22:33:44:55\n");
	return 0;
}

static const Cmd Cmds[] = {
	{"Misc"},
	{"setmac", "", NULL, "set the Mac address of LS2K syn0 and syn1", cmd_setmac, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
