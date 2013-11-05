#include "RunControlMessageManager.h"

#include "GUICommunicator.h"
#include "LocalNSMCommunicator.h"

#include <database/MySQLInterface.h>

#include <nsm/RCCallback.h>

#include <base/NSMNode.h>
#include <base/ConfigFile.h>

#include <system/PThread.h>

#include <base/Debugger.h>

#include <iostream>
#include <cstdlib>
#include <unistd.h>

int main(int argc, char** argv)
{
  using namespace Belle2;

  ConfigFile config("slc_config.conf");

  NSMNode* rc_node = new NSMNode(config.get("RC_NSMNAME"));

  NodeLoader* loader = new NodeLoader(config.get("RC_XML_PATH"));
  loader->load(config.get("RC_XML_ENTRY"));
  NodeSystem& node_system(loader->getSystem());
  node_system.setStatus(new RunStatus(config.get("RC_STATUS_NAME"),
                                      config.getInt("RC_STATUS_REV")));
  node_system.setConfig(new RunConfig(config.get("RC_CONFIG_NAME"),
                                      config.getInt("RC_CONFIG_REV")));
  node_system.setRunControlNode(rc_node);

  NSMCommunicator* gcomm = new NSMCommunicator(rc_node, config.get("RC_NSMHOST_IP"),
                                               config.getInt("RC_NSMHOST_PORT"));
  gcomm->setCallback(new RCCallback(rc_node));
  NSMCommunicator* lcomm = new NSMCommunicator(rc_node, config.get("RC_NSMHOST_LOCAL_IP"),
                                               config.getInt("RC_NSMHOST_LOCAL_PORT"));
  lcomm->setCallback(new RCCallback(rc_node));
  LocalNSMCommunicator* nsm_comm = new LocalNSMCommunicator(gcomm, lcomm);
  PThread th_nsm(nsm_comm);

  MySQLInterface* db = new MySQLInterface();
  db->init();
  db->connect(config.get("DB_HOST"), config.get("DB_NAME"),
              config.get("DB_USER"), config.get("DB_PASS"), config.getInt("DB_PORT"));
  DBNodeSystemConfigurator dbconfig(db, loader);
  dbconfig.rereadTable();

  GUICommunicator* gui_comm =  new GUICommunicator(config.get("RC_HOST_IP"),
                                                   config.getInt("RC_HOST_PORT"),
                                                   db, loader);
  PThread th_gui(gui_comm);

  RunControlMessageManager* manager
    = new RunControlMessageManager(db, nsm_comm, gui_comm, rc_node, loader);
  manager->run();

  return 0;
}

