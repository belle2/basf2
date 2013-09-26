#include "RunControlMessageManager.hh"

#include "GUICommunicator.hh"
#include "LocalNSMCommunicator.hh"

#include <nsm/RunStatus.hh>
#include <nsm/NSMNodeDaemon.hh>

#include <db/MySQLInterface.hh>
#include <db/DBNodeSystemConfigurator.hh>

#include <xml/NodeLoader.hh>

#include <system/PThread.hh>

#include <node/NSMNode.hh>

#include <runcontrol/RCState.hh>
#include <runcontrol/RCCallback.hh>

#include <util/ConfigReader.hh>
#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <iostream>
#include <cstdlib>

namespace B2DAQ {
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
  using namespace B2DAQ;

  if (argc < 1) {
    std::cerr << "Usage : ./runcontrold <path to config file>"
              << std::endl;
    return 1;
  }

  ConfigReader conf(argv[1]);

  const std::string node_name = conf.get("NSMNODE_NAME");
  const std::string dir = conf.get("XML_DIR");
  const std::string entry = conf.get("XML_ENTRY");
  const std::string path = conf.get("NSMDATA_LIB");
  const std::string ip = conf.get("SERVER_IP");
  const int port = atoi(conf.get("SERVER_PORT").c_str());
  const std::string db_host = getenv("B2SC_DB_HOST");
  const std::string db_name = getenv("B2SC_DB_NAME");
  const std::string db_user = getenv("B2SC_DB_USER");
  const std::string db_password = getenv("B2SC_DB_PASS");
  const int db_port = atoi(getenv("B2SC_DB_PORT"));

  NodeLoader* loader = new NodeLoader(dir);
  loader->setVersion(0);
  loader->load(entry);
  NSMDataManager* data = new NSMDataManager(loader);

  NodeSystem& node_system(loader->getSystem());
  NSMNode* rc_node = new NSMNode(node_name);
  rc_node->setState(RCState::INITIAL_S);
  node_system.setRunControlNode(rc_node);
  NSMCommunicator* comm = new NSMCommunicator(rc_node);
  RCCallback* callback = new RCCallback(rc_node);
  comm->setCallback(callback);
  try {
    comm->init();
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("TTD daemon : Failed to connect NSM network. Terminate process...");
    return 1;
  }
  data->allocateData(path);
  data->allocateRunConfig();
  data->allocateRunStatus();
  data->writeData();
  data->writeRunConfig();
  data->writeRunStatus();
  B2DAQ::PThread(new LocalNSMCommunicator(comm));

  TCPServerSocket server_socket(ip, port);
  server_socket.open();

  MySQLInterface* db = new MySQLInterface();
  db->init();
  db->connect(db_host, db_name, db_user, db_password, db_port);
  try {
    db->execute("select version, run_no, exp_no from run_config order by start_time desc limit 1;");
    std::vector<DBRecord>& record_v(db->loadRecords());
    if (record_v.size() > 0) {
      data->getRunConfig()->setVersion(record_v[0].getFieldValueInt("version"));
      data->getRunStatus()->setExpNumber(record_v[0].getFieldValueInt("exp_no"));
      data->getRunStatus()->setRunNumber(record_v[0].getFieldValueInt("run_no"));
      DBNodeSystemConfigurator config(db, &node_system);
      try {
        config.readTables(data->getRunConfig()->getVersion());
      } catch (const IOException& e) {
        B2DAQ::debug("Error on loading system configuration.:%s", e.what());
      }
    } else {
      data->getRunConfig()->setVersion(0);
      data->getRunStatus()->setExpNumber(0);
      data->getRunStatus()->setRunNumber(0);
    }
  } catch (const std::exception& e) {}
  HostCommunicator* ui_comm =  new GUICommunicator(server_socket, db, loader, data);
  B2DAQ::PThread(new Listener(ui_comm));

  RunControlMessageManager* manager
    = new RunControlMessageManager(db, comm, data, ui_comm,
                                   rc_node, &node_system);
  manager->run();

  return 0;
}

