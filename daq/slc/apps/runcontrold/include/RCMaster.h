#ifndef _Belle2_RCMaster_h
#define _Belle2_RCMaster_h

#include "daq/slc/apps/runcontrold/RunControlMessage.h"
#include "daq/slc/apps/runcontrold/RunStatus.h"
#include "daq/slc/apps/runcontrold/RunConfig.h"

#include <daq/slc/xml/XMLElement.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <daq/slc/base/NSMNode.h>
#include <daq/slc/base/DataObject.h>

#include <vector>

namespace Belle2 {

  class RCCommunicator;
  class RCDatabaseManager;

  class RCMaster {

  public:
    RCMaster(NSMNode* node,
             RunConfig* config, RunStatus* status);
    ~RCMaster() throw();

  public:
    typedef std::vector<NSMNode*> NSMNodeList;

  public:
    NSMNodeList& getNSMNodes() { return _node_v; }
    DataObject* getData() { return _data; }
    void setData(DataObject* data);
    NSMNode* findNode(int id,  const NSMMessage& msg) throw();
    NSMNode* getNodeByID(int id) throw();
    NSMNode* getNodeByName(const std::string& name) throw();
    void addNode(NSMNode* node) throw();
    void addNode(int id, NSMNode* node) throw();
    bool isSynchronized(NSMNode* node);
    void setNodeControl(XMLElement* el);
    void lock() throw();
    void unlock() throw();
    void wait() throw();
    void signal() throw();
    RCCommunicator* getClientCommunicator() { return _client_comm; }
    RCCommunicator* getMasterCommunicator() { return _master_comm; }
    void setClientCommunicator(RCCommunicator* comm) { _client_comm = comm; }
    void setMasterCommunicator(RCCommunicator* comm) { _master_comm = comm; }
    NSMNode* getNode() { return _master_node; }
    RunStatus* getStatus() { return _status; }
    RunConfig* getConfig() { return _config; }
    RCDatabaseManager* getDBManager() { return _dbmanager; }
    void setDBManager(RCDatabaseManager* manager) { _dbmanager = manager; }
    bool isSending() const { return _is_sending; }
    void setSending(bool sending) { _is_sending = sending; }

  private:
    bool _is_sending;
    Mutex _mutex;
    Cond _cond;
    NSMNode* _master_node;
    RunStatus* _status;
    RunConfig* _config;
    DataObject* _data;
    RCCommunicator* _master_comm;
    RCCommunicator* _client_comm;
    NSMNodeList _node_v;
    std::map<int, NSMNode*> _node_id_m;
    std::map<std::string, NSMNode*> _node_name_m;
    RCDatabaseManager* _dbmanager;

  };

}

#endif
