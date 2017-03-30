#ifndef _Belle2_HVMasterCallback_h
#define _Belle2_HVMasterCallback_h

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/hvcontrol/HVHandlerException.h>
#include <daq/slc/hvcontrol/HVNode.h>
#include <daq/slc/hvcontrol/HVConfig.h>

#include <vector>

namespace Belle2 {

  class HVMasterCallback : public RCCallback {

    typedef std::vector<HVNode> HVNodeList;
    typedef HVNodeList::iterator HVNodeIterator;
    typedef std::vector<NSMData> NSMDataList;

  public:
    HVMasterCallback();
    virtual ~HVMasterCallback() throw();

  public:
    virtual bool perform(NSMCommunicator& com) throw();
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);
    virtual void turnon() throw(HVHandlerException);
    virtual void turnoff() throw(HVHandlerException);
    virtual void standby() throw(HVHandlerException);
    virtual void shoulder() throw(HVHandlerException);
    virtual void peak() throw(HVHandlerException);
    virtual void recover() throw(HVHandlerException);
    virtual void ok(const char* nodename, const char* data) throw();
    virtual void error(const char* nodename, const char* data) throw();

  private:
    void distribute(NSMMessage msg) throw();
    void distribute_r(NSMMessage msg) throw();
    HVNode& findNode(const std::string& nodename) throw(std::out_of_range);
    bool check(const std::string& nodename, const HVState& state) throw();
    bool checkAll(const std::string& node, const HVState& state) throw();

  public:
    void setHVState(const HVState& state) throw();
    void setHVState(HVNode& node, const HVState& state) throw();
    void setConfig(HVNode& node, const std::string& config) throw();

  private:
    HVNodeList m_node_v;
    StringList m_excluded_v;
    HVNode m_hvnode;
    bool m_loading;

  private:
    class Distributor {
    public:
      Distributor(HVMasterCallback& callback, const NSMMessage& msg)
        : m_callback(callback), m_msg(msg), m_enabled(true) {}
      virtual ~Distributor() {}

    public:
      virtual void operator()(HVNode& node) throw();

    protected:
      HVMasterCallback& m_callback;
      NSMMessage m_msg;
      bool m_enabled;

    };

  };

}

#endif
