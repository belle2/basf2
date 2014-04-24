#ifndef _Belle2_TCPSocketReader_hh
#define _Belle2_TCPSocketReader_hh

#include "daq/slc/system/FileReader.h"
#include "daq/slc/system/TCPSocket.h"

namespace Belle2 {

  typedef FileReader<TCPSocket> TCPSocketReader;

}

#endif
