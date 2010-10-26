#ifndef RBCTLMGR_H
#define RBCTLMGR_H
//+
// File : rbctlmgr.h
// Description : Shared memory for roobasf multi-process handling
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 12 - Jan - 2010
//-


namespace Belle2 {

  struct RbCtlShm {
    int n_sync;
    int flags[127];
    int reserved[128];
  };

  class RbCtlMgr {
  public:
    RbCtlMgr(void);
    RbCtlMgr(int shmid);
    ~RbCtlMgr(void);
    void terminate(void);

    // Synchronization functions
    void sync_set(int nprocess);
    void sync_wait(void);
    void sync_done(void);

    // Communication flags
    void set_flag(int id, int val);
    int get_flag(int id);

    // Shared memory related function
    RbCtlShm* ctlshm(void);
    int shmid(void);

  private:
    int sem_lock(void);
    int sem_unlock(void);

  private:
    int m_shmid;
    RbCtlShm* m_ctlshm;
    int m_semid;
  };

} // namespace Roobasf

#endif
