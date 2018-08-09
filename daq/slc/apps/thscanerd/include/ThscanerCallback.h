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
    void initialize() throw();
    void stopRun();
    void startRun(int expno, int runno);
    bool initRun();
    void loadFile(const std::string& file);

    int getFTSW() const { return m_ftsw; }

  private:
    ConfigFile m_config;
    int m_trate;
    int m_tlimit;
    std::string m_ttype;
    std::string m_ftstate;
    int m_toutcnt;
    int m_ftsw;
    std::string m_rcstate;
    int m_nevents_cur;
    int m_nth_cur;
    int m_expno;
    int m_runno;
    std::vector<NSMNode> m_host;
    std::vector<std::string> m_copper;
    std::vector<std::string> m_hslb;
    NSMNode m_ttdnode;
    NSMNode m_rcnode;
    NSMNode m_ronode;
    DBObject m_dbobj;
    int m_count;
    bool m_adj;
    DBObject m_obj;
    size_t m_i_adj;
    bool m_recovering;
    std::map<std::string, int> m_hslbnevt;
    int m_nrun_total;
    bool m_next_run;

  };

}

#endif
