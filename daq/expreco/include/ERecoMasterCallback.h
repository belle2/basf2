#ifndef _Belle2_RFMasterCallback_h
#define _Belle2_RFMasterCallback_h

#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <vector>

namespace Belle2 {

  class ERecoRunControlCallback;

  class ERecoMasterCallback : public RCCallback {

  public:
    ERecoMasterCallback(ConfigFile& config);
    virtual ~ERecoMasterCallback() {}

  public:
    virtual void initialize(const DBObject& obj);
    virtual void configure(const DBObject& obj);
    virtual void monitor();
    virtual void ok(const char* node, const char* data);
    virtual void error(const char* node, const char* data);
    virtual void load(const DBObject& obj, const std::string& runtype);
    virtual void start(int expno, int runno);
    virtual void stop();
    virtual void recover(const DBObject& obj, const std::string& runtype);
    virtual bool resume(int subno);
    virtual bool pause();
    virtual void abort();
    virtual bool perform(NSMCommunicator& com);

  public:
    void setState(NSMNode& node, const RCState& state);
    void setCallback(ERecoRunControlCallback* callback) { m_callback = callback; }
    void addData(const std::string& dataname, const std::string& format);

  private:
    ERecoRunControlCallback* m_callback;
    StringList m_dataname;
    typedef std::vector<NSMNode> NSMNodeList;
    NSMNodeList m_nodes;
    NSMNode m_rcnode;
    std::string m_script;
  };

}

#endif
