#ifndef _Belle2_TCPSocketWriter_hh
#define _Belle2_TCPSocketWriter_hh

#include "system/FileWriter.h"
#include "system/TCPSocket.h"

namespace Belle2 {

  typedef FileWriter<TCPSocket> TCPSocketWriter;

}

#endif
