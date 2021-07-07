/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_ONSENBinData_h
#define _Belle2_ONSENBinData_h

#include "daq/storage/BinData.h"

namespace Belle2 {

  class ONSENBinData : public BinData {

  private:
    struct sose_frame_t {
      unsigned short start; /* should be 0x5800 (no error), 0xD800 (error) */
      unsigned short trigger_lo;
      unsigned short trigger_hi;
      unsigned short time_tag_lo_and_type;
      unsigned short time_tag_mid;
      unsigned short time_tag_hi;
      unsigned short frame_counter;
      unsigned int crc32;
    };

  public:
    static const unsigned int MAX_PXD_FRAMES  = 256;
    static const unsigned int MAGIC = 0xBEBAFECA;

  public:
    ONSENBinData(void* buf);
    ~ONSENBinData() throw();

  public:
    unsigned int getTrigger() const;
    unsigned int getTimetag() const;
    unsigned int getTriggerType() const;
    unsigned int getEventNumber() const;
    unsigned int getFrameNumber() const;
    unsigned int getFrameByteSize() const;
    unsigned int getONSENMagic() const;

  private:
    unsigned int* m_magic;
    unsigned int* m_framenr;
    unsigned int* m_length_of_frame;
    mutable sose_frame_t* m_start_frame;

  };

}

#endif
