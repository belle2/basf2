#include "daq/slc/apps/runcontrold/RCMaster.h"

#include "daq/slc/apps/runcontrold/RCGUIAcceptor.h"
#include "daq/slc/apps/runcontrold/RCClientAcceptor.h"

#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"
#include "daq/slc/apps/runcontrold/RCMasterCallback.h"
#include "daq/slc/apps/runcontrold/RCClientCallback.h"
#include "daq/slc/apps/runcontrold/RCDatabaseManager.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/xml/ObjectLoader.h>
#include <daq/slc/xml/XMLParser.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  std::string configname = (argc > 1) ? argv[1] : "runcontrol";
  ConfigFile config("slowcontrol", "cdc");
  ObjectLoader oloader(config.get("RC_XML_PATH"));
  DataObject* data = oloader.load(config.get("RC_XML_ENTRY"));
  XMLParser parser;
  XMLElement* el = parser.parse(config.get("RC_XML_PATH") + "/" +
                                config.get("RC_XML_ENTRY") + ".xml");
  const std::string name = el->getAttribute("name");
  int revision = data->getRevision();
  NSMNode* node_master = new NSMNode(name);
  NSMNode* node_client = new NSMNode(config.get("RC_NSM_NAME"));
  RunConfig* run_config = new RunConfig(name + "_config", revision);
  RunStatus* run_status = new RunStatus(name + "_status", revision);
  run_status->setConfig(run_config);
  RCMaster* master = new RCMaster(node_master, run_config, run_status);
  master->setData(data);
  master->setNodeControl(el);
  run_config->add(oloader.getClassList(), master->getNSMNodes());
  PThread(new NSMNodeDaemon(new RCClientCallback(node_client, master),
                            config.get("NSM_LOCAL_HOST"),
                            config.getInt("NSM_LOCAL_PORT")));
  sleep(5);
  RCMasterCallback* callback = new RCMasterCallback(node_master);
  callback->setMaster(master);
  int port = config.getInt("NSM_GLOBAL_PORT");
  if (port > 0) {
    sleep(5);
    PThread(new NSMNodeDaemon(callback, config.get("NSM_GLOBAL_HOST"), port));
  }
  sleep(5);
  PostgreSQLInterface* db =
    new PostgreSQLInterface(config.get("DATABASE_HOST"), config.get("DATABASE_NAME"),
                            config.get("DATABASE_USER"), config.get("DATABASE_PASS"),
                            config.getInt("DATABASE_PORT"));
  RCDatabaseManager* dbmanager = new RCDatabaseManager(db, master);
  master->setDBManager(dbmanager);
  dbmanager->createTables();
  dbmanager->readStatus();
  dbmanager->writeConfigs();
  port = config.getInt("RC_GLOBAL_PORT");
  if (port > 0) {
    PThread(new RCGUIAcceptor(config.get("RC_GLOBAL_HOST"), port, callback));
  }
  RCClientAcceptor* acceptor =
    new RCClientAcceptor(config.get("RC_LOCAL_HOST"),
                         config.getInt("RC_LOCAL_PORT"), master);
  acceptor->run();
  return 0;
}

