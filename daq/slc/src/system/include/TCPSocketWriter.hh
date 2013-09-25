#ifndef _B2DAQ_TCPSocketWriter_hh
#define _B2DAQ_TCPSocketWriter_hh

#include "FileWriter.hh"
#include "TCPSocket.hh"

namespace B2DAQ {

  typedef FileWriter<TCPSocket> TCPSocketWriter;

}

#endif
