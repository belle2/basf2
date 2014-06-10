#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
namespace Belle2 {

  class ProcessMonitor {
  public:
    void run() {
      int status;
      wait(&status);
    }
  };

}


using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: %s <port> [<hostname2nsm> <port2nsm>]", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  LogFile::open("nsm2socket");
  const std::string host = "0.0.0.0";
  const int port = atoi(argv[1]);
  const std::string host2nsm = (argc > 3) ? argv[2] : "";
  const int port2nsm = (argc > 3) ? atoi(argv[3]) : -1;
  TCPServerSocket server_socket(host, port);
  try {
    server_socket.open();
  } catch (const IOException& e) {
    LogFile::error("Faield to open server socket.");
    return 1;
  }
  while (true) {
    TCPSocket socket;
    try {
      socket = server_socket.accept();
    } catch (const IOException& e) {
      LogFile::error("Faield to accept connection.");
      return 1;
    }
    NSMNode node;
    try {
      node = NSMNode(TCPSocketReader(socket).readString());
    } catch (const IOException& e) {
      LogFile::error("IO error : %s ", e.what());
      return 1;
    }
    pid_t pid = fork();
    if (pid < 0) {
      LogFile::error("Faield to fork proces.");
      return 1;
    } else if (pid == 0) {
      LogFile::close();
      LogFile::open("nsm2socket." + node.getName());
      NSM2SocketCallback* callback = new NSM2SocketCallback(node);
      PThread(new NSMNodeDaemon(callback, host2nsm, port2nsm));
      NSM2SocketBridge(socket, callback, db).run();
      break;
    } else {
      PThread(new ProcessMonitor());
      socket.close();
    }
  }
  LogFile::debug("Terminate process");
  return 1;
}
