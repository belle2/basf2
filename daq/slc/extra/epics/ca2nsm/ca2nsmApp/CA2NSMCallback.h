#ifndef _Belle2_CA2NSMCallback_h
#define _Belle2_CA2NSMCallback_h

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/Cond.h>
#include <daq/slc/system/Mutex.h>

#include <map>

struct MYNODE {
  char value[20];
  chid mychid;
  evid myevid;
};

namespace Belle2 {

  class CA2NSMVHandler : public NSMVHandler {

  public:
    CA2NSMVHandler(const std::string& name, const NSMVar& var);
    virtual ~CA2NSMVHandler() throw() {}

  public:
    virtual bool handleGet(NSMVar& var);
    virtual bool handleSet(const NSMVar& var);

  private:
    chid m_cid;

  };

  class CA2NSMCallback : public NSMCallback {

  private:
    struct PVInfo {
      std::string pvname;
      std::string pv;
      char* pname;
      MYNODE* pvnode;
    };

  private:
    StringList m_pvnames;
    StringList m_pvnames_reserved;
    Cond m_pvcond;
    Mutex m_pvmutex;
    std::map<std::string, chid> m_cids;

  public:
    CA2NSMCallback(const NSMNode& node, const StringList pvnames);
    virtual ~CA2NSMCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& var) throw();
    virtual void vget(const std::string& nodename,
                      const std::string& vname) throw();

  public:
    void lock() { m_pvmutex.lock(); }
    void unlock() { m_pvmutex.unlock(); }
    void notify() { m_pvcond.signal(); }
    void addPVs() throw();
    bool addPV(const std::string& pvname) throw();

  };

}

#endif
