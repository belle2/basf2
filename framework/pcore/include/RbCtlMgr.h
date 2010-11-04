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

  /*! Shared memory structure to share information among processes */
  struct RbCtlShm {
    int n_sync;
    int flags[127];
    int reserved[128];
  };

  /*! Class to manage synchronization of processes in parallel processing */
  class RbCtlMgr {
  public:
    /*! Default constructor creating a new shared memory */
    RbCtlMgr(void);
    /*! Constructor attaching to an existing shared memory */
    RbCtlMgr(int shmid);
    /*! Destructor to detatch shared memory */
    ~RbCtlMgr(void);
    /*! Function to detach and remove shared memory */
    void terminate(void);

    // Synchronization functions
    /*! Set number of processes to be synchronized */
    void sync_set(int nprocess);
    /*! Wait for the synchronization */
    void sync_wait(void);
    /*! Tell mother process that this process is ready to be synchronized */
    void sync_done(void);

    // Communication flags
    /*! Set flag id to val in shared memory */
    void set_flag(int id, int val);
    /*! Get value of flag id */
    int get_flag(int id);

    // Shared memory related function
    /*! Get address of shared memory */
    RbCtlShm* ctlshm(void);
    /*! Get ID of shared memory */
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
