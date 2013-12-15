#ifndef _Belle2_ONSENBinData_h
#define _Belle2_ONSENBinData_h

#include "daq/storage/BinData.h"

namespace Belle2 {

  class ONSENBinData : public BinData {

  private:
    struct StartFrame {
      unsigned short start;
      unsigned short trigger_lo;
      unsigned short trigger_hi;
      unsigned short timetag_lo;
      unsigned short timetag_hi;
      unsigned int checksum;
    };

  public:
    static const unsigned int MAX_PXD_FRAMES  = 256;
    static const unsigned int MAGIC = 0xCAFEBABE;

  public:
    ONSENBinData(void* buf);
    ~ONSENBinData() throw();

  public:
    unsigned int getTrigger() const;
    unsigned int getTimetag() const;
    unsigned int getEventNumber() const;
    unsigned int getFrameNumber() const;
    unsigned int getFrameByteSize() const;
    unsigned int getONSENMagic() const;

  private:
    unsigned int* _magic;
    unsigned int* _framenr;
    unsigned int* _length_of_frame;
    mutable StartFrame* _start_frame;

  };

}

#endif
