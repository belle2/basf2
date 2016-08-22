#ifndef ARICHRAWDATAHEADER_H
#define ARICHRAWDATAHEADER_H

#include <iostream>


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

    void print()
    {
      std::cout << "Type: " << unsigned(type) << " version: " <<  unsigned(version) << std::endl;
      std::cout << "mergerID: " <<  unsigned(mergerID) << " FEBSlot: " <<  unsigned(FEBSlot) << std::endl;
      std::cout << "data length: " << length << " trigger No.: " << trigger << std::endl;
    }

  };
}


#endif
