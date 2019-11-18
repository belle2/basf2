#ifndef ARICHRAWDATAHEADER_H
#define ARICHRAWDATAHEADER_H

#include <iostream>
#include <vector>


namespace Belle2 {

#define ARICHFEB_HEADER_SIZE 10 // FEB header size in bytes
#define ARICHRAW_HEADER_SIZE 12 // Raw header size in bytes

  struct ARICHRawHeader {
    uint8_t type = 0;
    uint8_t version = 0;
    uint8_t mergerID = 0;
    uint8_t FEBSlot = 0;
    uint32_t length = 0;
    uint32_t trigger = 0;
    std::vector<bool> SEU_FEB = std::vector<bool>(6, false);

    void print()
    {
      std::cout << "Type: " << unsigned(type) << " version: " <<  unsigned(version) << std::endl;
      std::cout << "mergerID: " <<  unsigned(mergerID) << " FEBSlot: " <<  unsigned(FEBSlot) << std::endl;
      std::cout << "data length: " << length << " trigger No.: " << trigger << std::endl;
      std::cout << "SEU of FEB: " << SEU_FEB[5] << " " << SEU_FEB[4] << " " << SEU_FEB[3] << " " << SEU_FEB[2] << " " << SEU_FEB[1] << " "
                << SEU_FEB[0] << std::endl;
    }

  };
}


#endif
