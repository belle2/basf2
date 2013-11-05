#ifndef _Belle2_TCPSocketReader_hh
#define _Belle2_TCPSocketReader_hh

#include "system/FileReader.h"
#include "system/TCPSocket.h"

namespace Belle2 {

  typedef FileReader<TCPSocket> TCPSocketReader;

}

#endif
