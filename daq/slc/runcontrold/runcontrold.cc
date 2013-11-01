#include "RunControlMessageManager.h"

#include "GUICommunicator.h"
#include "LocalNSMCommunicator.h"

#include <nsm/RunStatus.h>
#include <nsm/NSMNodeDaemon.h>
#include <nsm/RCCallback.h>

#include <database/MySQLInterface.h>
#include <database/DBNodeSystemConfigurator.h>

#include <xml/NodeLoader.h>

#include <system/PThread.h>

#include <base/NSMNode.h>
#include <base/ConfigReader.h>
#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <iostream>
#include <cstdlib>
#include <unistd.h>

namespace Belle2 {
  class Listener {
  public:
    Listener(HostCommunicator* comm)
      : _comm(comm) {}
    ~Listener() {}
    void run() {
      _comm->run();
    }
  private:
    HostCommunicator* _comm;
  };
}

int main(int argc, char** argv)
{
  using namespace Belle2;

  if (argc < 1) {
    std::cerr << "Usage : ./runcontrold [ip=50000]"
              << std::endl;
    return 1;
  }

  const std::string node_name = "RUNCONTROL";
  const std::string dir = getenv("B2SC_XML_PATH");
  const std::string entry = getenv("B2SC_XML_ENTRY");
  const std::string path = getenv("B2SC_CPRLIB_PATH");
  const std::string ip = getenv("B2SC_SERVER_HOST");
  const int port = (argc > 1) ? atoi(argv[1]) : 50000;
  const std::string db_host = getenv("B2SC_DB_HOST");
  const std::string db_name = getenv("B2SC_DB_NAME");
  const std::string db_user = getenv("B2SC_DB_USER");
  const std::string db_password = getenv("B2SC_DB_PASS");
  const int db_port = atoi(getenv("B2SC_DB_PORT"));

  NodeLoader* loader = new NodeLoader(dir);
  loader->setVersion(0);
  loader->load(entry);

  NodeSystem& node_system(loader->getSystem());
  NSMDataManager* data = new NSMDataManager(&node_system);
  NSMNode* rc_node = new NSMNode(node_name);
  node_system.setRunControlNode(rc_node);
  NSMCommunicator* comm = new NSMCommunicator(rc_node);
  RCCallback* callback = new RCCallback(rc_node);
  comm->setCallback(callback);
  while (true) {
    try {
      comm->init();
      break;
    } catch (const NSMHandlerException& e) {
      Belle2::debug("[DEBUG] Failed to connect NSM network. Re-trying to connect...");
      sleep(3);
    }
  }
  data->allocateRunConfig();
  data->allocateRunStatus();
  data->writeRunConfig();
  data->writeRunStatus();

  TCPServerSocket server_socket(ip, port);
  server_socket.open();

  MySQLInterface* db = new MySQLInterface();
  db->init();
  db->connect(db_host, db_name, db_user, db_password, db_port);
  try {
    db->execute("select * from run_config order by start_time desc limit 1;");
    std::vector<DBRecord>& record_v(db->loadRecords());
    if (record_v.size() > 0) {
      data->getRunConfig()->setRunType(record_v[0].getFieldValue("run_type"));
      data->getRunConfig()->setOperators(record_v[0].getFieldValue("operators"));
      data->getRunConfig()->setVersion(record_v[0].getFieldValueInt("version"));
      data->getRunStatus()->setExpNumber(record_v[0].getFieldValueInt("exp_no"));
      data->getRunStatus()->setRunNumber(record_v[0].getFieldValueInt("run_no"));
    } else {
      data->getRunConfig()->setVersion(0);
      data->getRunStatus()->setExpNumber(0);
      data->getRunStatus()->setRunNumber(0);
    }
    try {
      DBNodeSystemConfigurator config(db, &node_system);
      config.readTables(data->getRunConfig()->getVersion());
    } catch (const IOException& e) {
      Belle2::debug("[FATAL] Error on loading system configuration.:%s", e.what());
      return 1;
    }
  } catch (const std::exception& e) {
    data->getRunConfig()->setVersion(0);
    data->getRunStatus()->setExpNumber(0);
    data->getRunStatus()->setRunNumber(0);
  }
  HostCommunicator* ui_comm =  new GUICommunicator(server_socket, db, loader);
  Belle2::PThread(new Listener(ui_comm));
  Belle2::PThread(new LocalNSMCommunicator(comm));

  RunControlMessageManager* manager
    = new RunControlMessageManager(db, comm, data, ui_comm,
                                   rc_node, &node_system);
  manager->run();

  return 0;
}

