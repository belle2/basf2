#ifndef _Belle2_TCPSocketWriter_hh
#define _Belle2_TCPSocketWriter_hh

#include "daq/slc/system/FileWriter.h"
#include "daq/slc/system/TCPSocket.h"

namespace Belle2 {

  typedef FileWriter<TCPSocket> TCPSocketWriter;

}

#endif
