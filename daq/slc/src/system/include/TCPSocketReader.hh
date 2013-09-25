#ifndef _B2DAQ_TCPSocketReader_hh
#define _B2DAQ_TCPSocketReader_hh

#include "FileReader.hh"
#include "TCPSocket.hh"

namespace B2DAQ {

  typedef FileReader<TCPSocket> TCPSocketReader;

}

#endif
