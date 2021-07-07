/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* find_pci_path.h */


#ifndef FIND_PCI_PATH_H
#define FIND_PCI_PATH_H

int
find_pci_path(const unsigned int target_vendor_id, const unsigned int target_device_id, char** *paths_p_ret);

char*
get_copper_plx9054_path(void);

#endif

