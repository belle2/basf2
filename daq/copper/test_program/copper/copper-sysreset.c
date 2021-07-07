/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* sysreset.c */


#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>

#define PCIVME_PATH  "/proc/bus/pci/01/0d.0"


int
main(int argc, char *argv[])
{
	int devfd, memfd;
	int ret;
	struct {
		unsigned long m_word[4];
		unsigned long m_bar[4];
	} pcichip;
	int pagesize;
	unsigned char *raw_p, *map_p, *map_csr_p;
	unsigned long off;


	devfd = open(PCIVME_PATH, O_RDONLY);
	if( devfd==-1 ){
		perror("scan pcivme");
		exit(1);
	}
	ret = read(devfd, &pcichip, sizeof(pcichip));
	if( ret!=sizeof(pcichip) ){
		perror("scan PCI BARs");
		exit(1);
	}
	close(devfd);


	pagesize = getpagesize();


	raw_p = (unsigned char*)(pcichip.m_bar[1] / pagesize * pagesize);
	off   = pcichip.m_bar[1] - pcichip.m_bar[1] / pagesize * pagesize;


	memfd = open("/dev/mem", O_RDWR);
	if( memfd==-1 ){
		perror("open /dev/mem");
		exit(1);
	}
	map_p = (unsigned char*)mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, (off_t)raw_p);
	if( map_p==(unsigned char*)-1 ){
		perror("mmap");
		exit(1);
	}


	map_csr_p = map_p + off;
	

	*(map_csr_p + 0x10) = 0xff;


	return 0;
}

