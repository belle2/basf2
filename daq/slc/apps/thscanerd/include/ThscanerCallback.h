#ifndef _Belle2_ThscanerCallback_h
#define _Belle2_ThscanerCallback_h

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/base/ConfigFile.h"
#include "daq/slc/database/DBObject.h"

namespace Belle2 {

  class ThscanerCallback : public NSMCallback {

  public:
    ThscanerCallback(const std::string& nodename,
                     ConfigFile& config, int timout = 1);
    virtual ~ThscanerCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& var) throw();
    virtual void vreply(NSMCommunicator&, const std::string& name, bool ret) throw();
    virtual void ok(const char* node, const char* data) throw();

  public:
    void stopRun();
    void startRun(int expno, int runno);
    bool initRun();
    void loadFile(const std::string& file);

  private:
    ConfigFile m_config;
    int m_tlimit;
    std::string m_ftstate;
    std::string m_rcstate;
    int m_nevents_cur;
    int m_nth_cur;
    int m_expno;
    int m_runno;
    std::vector<NSMNode> m_copper;
    std::vector<std::string> m_hslb;
    NSMNode m_ttdnode;
    NSMNode m_rcnode;
    NSMNode m_ronode;
    DBObject m_dbobj;
    int m_count;
    int m_toutcnt;
    bool m_adj;
    DBObject m_obj;
    size_t m_i_adj;

  };

}

#endif
