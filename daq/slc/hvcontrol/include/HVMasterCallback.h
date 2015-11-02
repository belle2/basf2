#ifndef _Belle2_HVMasterCallback_h
#define _Belle2_HVMasterCallback_h

#include <daq/slc/hvcontrol/HVHandlerException.h>
#include <daq/slc/hvcontrol/HVConfig.h>
#include <daq/slc/hvcontrol/HVState.h>
#include <daq/slc/hvcontrol/HVCommand.h>

#include <daq/slc/database/DAQLogMessage.h>
#include <daq/slc/database/DBInterface.h>

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMNode.h>
#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/LogFile.h>

#include <vector>

namespace Belle2 {

  class HVMasterCallback : public NSMCallback {

    typedef std::vector<NSMNode> NSMNodeList;
    typedef NSMNodeList::iterator NSMNodeIterator;
    typedef std::vector<NSMData> NSMDataList;

  public:
    HVMasterCallback();
    virtual ~HVMasterCallback() throw() {}

  public:
    virtual void turnon() throw(HVHandlerException);
    virtual void turnoff() throw(HVHandlerException);
    virtual void standby() throw(HVHandlerException);
    virtual void shoulder() throw(HVHandlerException);
    virtual void peak() throw(HVHandlerException);
    virtual void recover() throw(HVHandlerException);
    virtual void configure(const DBObject&) throw(HVHandlerException);
    virtual void initialize(const DBObject&) throw(HVHandlerException);
    virtual void ok(const char* nodename, const char* data) throw();
    virtual void error(const char* nodename, const char* data) throw();
    virtual void fatal(const char* nodename, const char* data) throw();

    //const HVState& getStateDemand() const throw() { return m_state_demand; }

  public:
    bool perform(NSMCommunicator& com) throw();
    void setLogTable(const std::string table) { m_logtable = table; }

  private:
    void distribute(NSMMessage msg) throw();
    void distribute_r(NSMMessage msg) throw();
    NSMNode& findNode(const std::string& nodename) throw(std::out_of_range);
    bool check(const std::string& nodename, const HVState& state) throw();
    void logging(const NSMNode& node, LogFile::Priority pri,
                 const char* text, ...);
    void logging_imp(const NSMNode& node, LogFile::Priority pri,
                     const Date& date, const std::string& msg, bool recorded);
    bool addAll(const DBObject& obj) throw();

  public:
    void setState(const HVState& state) throw();
    void setState(NSMNode& node, const HVState& state) throw();
    void setConfig(NSMNode& node, const std::string& config) throw();

  public:
    void setDB(const std::string& table, DBInterface* db) throw()
    {
      m_table = table;
      m_db = db;
    }
    DBInterface* getDB() throw() { return m_db; }

  private:
    NSMNodeList m_node_v;
    std::string m_logtable;
    StringList m_excluded_v;
    int m_port;
    DBInterface* m_db;
    std::string m_table;
    HVState m_state_demand;

  private:
    class Distributor {
    public:
      Distributor(HVMasterCallback& callback, const NSMMessage& msg)
        : m_callback(callback), m_msg(msg), m_enabled(true) {}
      virtual ~Distributor() {}

    public:
      virtual void operator()(NSMNode& node) throw();

    protected:
      HVMasterCallback& m_callback;
      NSMMessage m_msg;
      bool m_enabled;

    };

  };

}

#endif

