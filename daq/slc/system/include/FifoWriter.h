#ifndef _Belle2_FifoWriter_hh
#define _Belle2_FifoWriter_hh

#include "daq/slc/system/FileWriter.h"
#include "daq/slc/system/Fifo.h"

namespace Belle2 {

  typedef FileWriter<Fifo> FifoWriter;

}

#endif
