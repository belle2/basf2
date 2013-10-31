#ifndef _Belle2_TCPSocketReader_hh
#define _Belle2_TCPSocketReader_hh

#include "FileReader.h"
#include "TCPSocket.h"

namespace Belle2 {

  typedef FileReader<TCPSocket> TCPSocketReader;

}

#endif
