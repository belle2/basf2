#ifndef _Belle2_TCPSocketWriter_hh
#define _Belle2_TCPSocketWriter_hh

#include "FileWriter.h"
#include "TCPSocket.h"

namespace Belle2 {

  typedef FileWriter<TCPSocket> TCPSocketWriter;

}

#endif
