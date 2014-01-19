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
#include <daq/slc/base/SystemLog.h>

#include <vector>
#include <list>

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
    void setClientCommunicator(RCCommunicator* comm) { _client_comm = comm; }
    bool hasMasterCommunicator();
    void addMasterCommunicator(RCCommunicator* comm);
    void removeMasterCommunicator(RCCommunicator* comm);
    void sendMessageToMaster(const RunControlMessage& msg) throw();
    void sendStateToMaster(NSMNode* node) throw();
    void sendDataObjectToMaster(const std::string& name,
                                DataObject* data) throw();
    void sendLogToMaster(const SystemLog& log) throw();
    NSMNode* getNode() { return _master_node; }
    RunStatus* getStatus() { return _status; }
    RunConfig* getConfig() { return _config; }
    RCDatabaseManager* getDBManager() { return _dbmanager; }
    void setDBManager(RCDatabaseManager* manager) { _dbmanager = manager; }
    bool isSending() const { return _is_sending; }
    void setSending(bool sending) { _is_sending = sending; }
    bool isGlobal() const { return _is_global; }
    void setGlobal(bool is_global) { _is_global = is_global; }

  private:
    bool _is_sending;
    Mutex _mutex;
    Cond _cond;
    NSMNode* _master_node;
    RunStatus* _status;
    RunConfig* _config;
    DataObject* _data;
    //RCCommunicator* _master_comm;
    std::list<RCCommunicator*> _master_comm_v;
    Mutex _mutex_comm;
    RCCommunicator* _client_comm;
    NSMNodeList _node_v;
    std::map<int, NSMNode*> _node_id_m;
    std::map<std::string, NSMNode*> _node_name_m;
    RCDatabaseManager* _dbmanager;
    bool _is_global;

  };

}

#endif
