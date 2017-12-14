#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Process.h>
#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace Belle2 {

  class ProcessMonitor {
  public:
    void run()
    {
      int status;
      wait(&status);
    }
  };

}


using namespace Belle2;

int main(int argc, char** argv)
{
  int port = (argc > 1) ? atoi(argv[1]) : -1;
  std::string hostname = "";
  if (port <= 0) {
    LogFile::debug("usage: %s <port> [-d]", argv[0]);
    return 1;
  }
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0) {
      i++;
      if (i < argc) hostname = argv[i];
    }
    if (strcmp(argv[i], "-d") == 0) {
      daemon(0, 0);
    }
  }
  LogFile::open("nsm2socket/nsm2scoket");
  if (hostname.size() == 0) {
    TCPServerSocket server_socket("0.0.0.0", port);
    try {
      server_socket.open();
      while (true) {
        TCPSocket socket = server_socket.accept();
        Process(new NSM2SocketBridge(socket));
        socket.close();
        PThread(new ProcessMonitor());
      }
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
    LogFile::debug("Terminate process");
  } else {
    TCPSocket socket(hostname, port);
    socket.connect();
    NSM2SocketBridge(socket).run();
  }
  return 0;
}
