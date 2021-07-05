/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* find_pci_path.c */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>


int
find_pci_path(const unsigned int target_vendor_id, const unsigned int target_device_id, char ***paths_p_ret)
{
#define MAX_SEARCH (32)
#define PCI_PATH ("/proc/bus/pci")
#define PCI_DEVICE_FILE ("/proc/bus/pci/devices")

	int i;
	FILE *fp;
	char buf[512];
	int nfound = 0;
	static char paths[MAX_SEARCH][64];
	static char *paths_p[MAX_SEARCH];


	for( i=0; i<MAX_SEARCH; i++ ){
		paths_p[i] = &paths[i][0];
	}
	*paths_p_ret = paths_p;


	fp = fopen(PCI_DEVICE_FILE, "r");
	if( !fp ){
		fprintf(stderr, "cannot open %s: %s\n", PCI_DEVICE_FILE, strerror(errno));
		return 0;
	}


	while( fgets(buf,sizeof(buf),fp) ){
		unsigned int dfn, ven;
		unsigned int pci_bus, pci_slot, pci_func;
		unsigned int vendor_id, device_id;

		sscanf(buf, "%x %x", &dfn, &ven);

		pci_bus  = dfn >> 8;
		pci_slot = (dfn & 0xff) >> 3 & 0x1f;
		pci_func = dfn & 0x07;

		vendor_id = ven >> 16;
		device_id = ven & 0xffff;

		if( vendor_id!=target_vendor_id ) continue;
		if( device_id!=target_device_id ) continue;

		sprintf(paths_p[nfound++], "%s/%02x/%02x.%1x", PCI_PATH, pci_bus, pci_slot, pci_func);
		if( nfound>=MAX_SEARCH ) break;
	}


	fclose(fp);


	return nfound;
}


char *
get_copper_plx9054_path(void)
{
	int nfound;

	/* PLX9054 */
	const unsigned int vendor_id = 0x10b5; 
	const unsigned int device_id = 0x9054; 
	char **paths_p;

	nfound = find_pci_path(vendor_id, device_id, &paths_p);

	switch( nfound ){
		case 0:
			fprintf(stderr, "get_copper_plx9054_path: cannot find PLX9054\n");
			exit(1);

		case 1:
			/* not very expected */
			return paths_p[0];

		case 2:
		{
			/* both COPPER and TTRX has PLX9054 -> assume COPPER's PLX9054 resides on smaller id bus */
			char path0[64], path1[64];
			char bus0, bus1;

			strcpy(path0, paths_p[0]);
			strcpy(path1, paths_p[1]);

			path0[16] = 0; bus0 = strtoul(&path0[14], NULL, 16);
			path1[16] = 0; bus1 = strtoul(&path1[14], NULL, 16);

			return bus0<bus1 ? paths_p[0] : paths_p[1];
		}

		default:
			fprintf(stderr, "get_copper_plx9054_path: more than 2 PLX9054s found (%d)\n", nfound);
			exit(1);
	}
}

