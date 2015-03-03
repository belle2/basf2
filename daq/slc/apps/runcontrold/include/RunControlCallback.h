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
    RunControlCallback(int port);
    virtual ~RunControlCallback() throw() {}

  public:
    virtual bool initialize(const DBObject& obj) throw();
    virtual bool configure(const DBObject& obj) throw();
    virtual void ok(const char* nodename, const char* data) throw();
    virtual void error(const char* nodename, const char* data) throw();
    virtual void log(const DAQLogMessage&) throw();
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover() throw(RCHandlerException);
    virtual void resume() throw(RCHandlerException);
    virtual void pause() throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void timeout(NSMCommunicator& com) throw();

  public:
    void setPriorityToDB(LogFile::Priority pri) { m_priority_db = pri; }
    void setPriorityToGlobal(LogFile::Priority pri) { m_priority_global = pri; }
    void setExcludedNodes(const StringList& excluded) { m_excluded_v = excluded; }
    void setCallback(RCCallback* callback) { m_callback = callback; }

  private:
    void update() throw();
    void distribute(NSMMessage msg) throw();
    void distribute_r(NSMMessage msg) throw();
    void postRun() throw();
    RCNode& findNode(const std::string& nodename) throw(std::out_of_range);
    bool check(const std::string& nodename, const RCState& state) throw();
    void logging(const NSMNode& node, LogFile::Priority pri,
                 const char* text, ...);
    void logging_imp(const NSMNode& node, LogFile::Priority pri,
                     const Date& date, const std::string& msg, bool recorded);
    bool vaddAll(const DBObject& obj) throw();
    using RCCallback::setState;
    void setState(NSMNode& node, const RCState& state) throw();

  private:
    RCCallback* m_callback;
    RCNodeList m_node_v;
    RunNumber m_runno;
    std::string m_operators;
    std::string m_comment;
    LogFile::Priority m_priority_db;
    LogFile::Priority m_priority_global;
    StringList m_excluded_v;
    int m_port;

  private:
    class Distributer {
    public:
      Distributer(RunControlCallback& callback, const NSMMessage& msg)
        : m_callback(callback), m_msg(msg), m_enabled(true) {}

    public:
      void operator()(RCNode& node) throw();

    private:
      RunControlCallback& m_callback;
      NSMMessage m_msg;
      bool m_enabled;

    };

  private:
    class ConfigProvider {
    public:
      ConfigProvider(DBInterface& db, const std::string& dbtable, int port)
        : m_db(db), m_dbtable(dbtable), m_port(port) {}

    public:
      void run();

    private:
      DBInterface& m_db;
      const std::string m_dbtable;
      int m_port;
    };

  private:
    static bool handleGetNNodes(Callback* callback, int& v,
                                const std::string& name, const void* pdata);
    static bool handleGetExpNumber(Callback* callback, int& v,
                                   const std::string& name, const void* pdata);
    static bool handleGetRunNumber(Callback* callback, int& v,
                                   const std::string& name, const void* pdata);
    static bool handleGetSubNumber(Callback* callback, int& v,
                                   const std::string& name, const void* pdata);
    static bool handleGetTStart(Callback* callback, int& v,
                                const std::string& name, const void* pdata);
    static bool handleGetConfig(Callback* callback, std::string& v,
                                const std::string& name, const void* pdata);
    static bool handleGetState(Callback* callback, std::string& v,
                               const std::string& name, const void* pdata);
    static bool handleGetOperators(Callback* callback, std::string& v,
                                   const std::string& name, const void* pdata);
    static bool handleGetComment(Callback* callback, std::string& v,
                                 const std::string& name, const void* pdata);
    static bool handleGetName(Callback* callback, std::string& v,
                              const std::string& name, const void* pdata);
    static bool handleGetUsed(Callback* callback, int& v,
                              const std::string& name, const void* pdata);
    static bool handleGetExcluded(Callback* callback, int& v,
                                  const std::string& name, const void* pdata);
    static bool handleSetExpNumber(Callback* callback, int v,
                                   const std::string& name, const void* pdata);
    static bool handleSetConfig(Callback* callback, const std::string& v,
                                const std::string& name, const void* pdata);
    static bool handleSetOperators(Callback* callback, const std::string& v,
                                   const std::string& name, const void* pdata);
    static bool handleSetComment(Callback* callback, const std::string& v,
                                 const std::string& name, const void* pdata);
    static bool handleSetRequest(Callback* callback, const std::string& v,
                                 const std::string& name, const void* pdata);
    static bool handleSetUsed(Callback* callback, int v,
                              const std::string& name, const void* pdata);
    static bool handleSetExcluded(Callback* callback, int v,
                                  const std::string& name, const void* pdata);

  };

}

#endif

