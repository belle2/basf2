#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/LoggerObjectTable.h>
#include <daq/slc/database/RunNumberInfoTable.h>

#include "daq/slc/apps/nsm2socket/NSM2SocketBridgeThread.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include "daq/slc/runcontrol/RunSetting.h"

#include "daq/slc/nsm/NSMNode.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <unistd.h>

using namespace Belle2;

bool send(Writer& writer, const NSMMessage& msg)
{
  try {
    writer.writeObject(msg);
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for writing");
  }
  return false;
}

bool recieve(Reader& reader, NSMMessage& msg)
{
  try {
    msg.init();
    reader.readObject(msg);
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for reading");
  }
  return false;
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage: dbtest5 <nodename> [<configname>]");
    return 1;
  }
  const std::string nodename = argv[1];

  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));

  TCPServerSocket server_socket("0.0.0.0", 9090);
  server_socket.open();
  TCPSocket socket;
  NSMNode node(nodename);
  NSM2SocketCallback callback(node, 2);
  PThread(new NSMNodeDaemon(&callback));
  while (true) {
    socket = server_socket.accept();
    LogFile::debug("Accepted new connection");
    TCPSocketWriter writer(socket);
    TCPSocketReader reader(socket);
    NSMMessage msg;
    while (true) {
      NSMMessage msg_out;
      if (!recieve(reader, msg)) {
        socket.close();
        break;
      }
      NSMCommand cmd(msg.getRequestName());
      if (cmd == Enum::UNKNOWN) {
        LogFile::warning("Unknown request : %s", msg.getRequestName());
      } else if (cmd == NSMCommand::DBGET) {
        try {
          db->connect();
          StringList str_v = StringUtil::split(msg.getData(), ' ', 5);
          std::string nodename = str_v[1];
          std::string configname = str_v[2];
          if (msg.getNParams() > 0 && msg.getParam(0) > 0) {
            configname = ConfigInfoTable(db).get(16).getName();
          }
          ConfigObject obj = ConfigObjectTable(db).get(configname, nodename);
          msg_out.setRequestName(NSMCommand::DBSET);
          msg_out.setData(obj);
          send(writer, msg_out);
          ConfigObjectTable(db).get(configname, nodename);
          db->close();
        } catch (const DBHandlerException& e) {
          LogFile::error(e.what());
        }
      } else if (cmd == NSMCommand::NSMGET) {
        StringList argv = StringUtil::split(msg.getData(), ' ', 2);
        try {
          int revision = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
          NSMData& data(callback.getData(argv[0], argv[1], revision));
          if (data.isAvailable()) {
            msg_out.setRequestName(NSMCommand::NSMSET);
            msg_out.setData(data);
            send(writer, msg_out);
          }
        } catch (const NSMHandlerException& e) {
          LogFile::error(e.what());
          msg_out.setRequestName(NSMCommand::LOG);
          msg_out.setData(DAQLogMessage("NSM2Socket", LogFile::WARNING,
                                        StringUtil::form("NSM data (%s) is not ready for read",
                                                         argv[0].c_str())));
          send(writer, msg_out);
        }
      }
    }
  }
  return 0;
}
