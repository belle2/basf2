#ifndef _Belle2_RunControlCallback_h
#define _Belle2_RunControlCallback_h

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/runcontrol/RCNode.h>

#include <daq/slc/database/RunNumberTable.h>
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/LogFile.h>

#include <vector>
#include <map>

namespace Belle2 {

  class RunControlCallback : public RCCallback {

    typedef std::vector<RCNode> RCNodeList;
    typedef RCNodeList::iterator RCNodeIterator;
    typedef std::vector<NSMData> NSMDataList;

  public:
    RunControlCallback();
    virtual ~RunControlCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void ok(const char* nodename, const char* data) throw();
    virtual void error(const char* nodename, const char* data) throw();
    virtual void fatal(const char* nodename, const char* data) throw();
    virtual void boot(const std::string& opt, const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);
    virtual void vset(NSMCommunicator& com, const NSMVar& v) throw();
    virtual void check() throw(RCHandlerException);

  public:
    void setPriorityToDB(LogFile::Priority pri) { m_priority_db = pri; }
    void setPriorityToGlobal(LogFile::Priority pri) { m_priority_global = pri; }
    void setExcludedNodes(const StringList& excluded) { m_excluded_v = excluded; }
    void setLocalRunControls(const StringList& rc);
    void setLogTable(const std::string table) { m_logtable = table; }
    void setCallback(RCCallback* callback) { m_callback = callback; }
    void setRestartTime(double restarttime) { m_restarttime = restarttime; }

  private:
    void distribute(NSMMessage msg) throw();
    void distribute_r(NSMMessage msg) throw();
    void postRun() throw();
    RCNode& findNode(const std::string& nodename) throw(std::out_of_range);
    bool check(const std::string& nodename, const RCState& state) throw();
    bool checkAll(const std::string& nodename, const RCState& state) throw();
    void logging(const NSMNode& node, LogFile::Priority pri,
                 const char* text, ...);
    void logging_imp(const NSMNode& node, LogFile::Priority pri,
                     const Date& date, const std::string& msg);
    bool addAll(const DBObject& obj) throw();
    using RCCallback::setState;

  public:
    void setState(NSMNode& node, const RCState& state) throw();
    // void setConfig(RCNode& node, const std::string& config) throw();
    void setExpNumber(int expno) throw();
    bool setRCUsed(int used) throw();
    bool getRCUsed() throw();
    void setGlobalAll(bool isglobal) throw();

  private:
    RCCallback* m_callback;
    RCNodeList m_node_v;
    RunNumber m_runno;
    std::string m_operators;
    std::string m_comment;
    std::string m_logtable;
    LogFile::Priority m_priority_db;
    LogFile::Priority m_priority_global;
    StringList m_excluded_v;
    RCNodeList m_lrc_v;
    int m_port;
    double m_restarttime;
    double m_starttime;
    bool m_restarting;

  private:
    class Distributor {
    public:
      Distributor(RunControlCallback& callback, const NSMMessage& msg)
        : m_callback(callback), m_msg(msg), m_enabled(true) {}
      virtual ~Distributor() {}

    public:
      virtual void operator()(RCNode& node) throw();

    protected:
      RunControlCallback& m_callback;
      NSMMessage m_msg;
      bool m_enabled;

    };

  private:
    class Recoveror : public Distributor {
    public:
      Recoveror(RunControlCallback& callback, const NSMMessage& msg)
        : Distributor(callback, msg) {}
      virtual ~Recoveror() {}

    public:
      virtual void operator()(RCNode& node) throw();

    };

  };

}

#endif

