/* find_pci_path.h */

#ifndef FIND_PCI_PATH_H
#define FIND_PCI_PATH_H

#if defined(__cplusplus)
extern "C" {
#endif

int find_pci_path(const unsigned int target_vendor_id,
                  const unsigned int target_device_id,
                  char** * paths_p_ret);

char* get_copper_plx9054_path(void);

#if defined(__cplusplus)
}
#endif
#endif

