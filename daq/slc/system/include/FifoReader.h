#ifndef _Belle2_FifoReader_hh
#define _Belle2_FifoReader_hh

#include "daq/slc/system/FileReader.h"
#include "daq/slc/system/Fifo.h"

namespace Belle2 {

  typedef FileReader<Fifo> FifoReader;

}

#endif
