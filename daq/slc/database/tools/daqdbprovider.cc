#include <daq/slc/database/PostgreSQLInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/Daemon.h>
#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    const int port = config.getInt("provider.port");
    const std::string dbtable = config.get("dbtable");
    TCPServerSocket server("0.0.0.0", port);
    try {
      server.open();
    } catch (const IOException& e) {
      LogFile::error("failed to open server socket %d", port);
      exit(1);
    }
    LogFile::debug("Open port %d for %s", port, "0.0.0.0");
    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));
    while (true) {
      TCPSocket socket;
      try {
        socket = server.accept();
        TCPSocketReader reader(socket);
        int flag = reader.readInt();
        if (flag == 1) {
          const std::string path = reader.readString();
          StringList s = StringUtil::split(path, '/');
          const std::string table = (s.size() > 0) ? s[0] : dbtable;
          const std::string config = (s.size() > 0) ? s[1] : path;
          DBObject obj(DBObjectLoader::load(db, dbtable, config));
          LogFile::debug("loaded %s from %s", obj.getName().c_str(), table.c_str());
          obj.print();
          TCPSocketWriter writer(socket);
          writer.writeObject(obj);
        } else if (flag == 2) {
          DBObject obj;
          const std::string table = reader.readString();
          reader.readObject(obj);
          DBObjectLoader::createDB(db, table, obj);
        }
      } catch (const IOException& e) {
        LogFile::error(e.what());
      }
      socket.close();
      db.close();
    }
  }
  return 0;
}

