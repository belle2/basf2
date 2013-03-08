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
    int n_sync; /**< Number of pending synchronizations. If not 0, RbCtlMgr::sync_wait() will wait. */
    int flags[127]; /**< flags accessed by RbCtlMgr::set_flag/get_flag(). */
    int reserved[128]; /**< For future use. */
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

    // Synchronization functions (not used right now)
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
    /** Lock semaphore protecting m_ctlshm. */
    int sem_lock(void);
    /** Unlock semaphore protecting m_ctlshm. */
    int sem_unlock(void);

  private:
    int m_shmid; /**< ID of shared memory segment. (See shmget(2)) */
    RbCtlShm* m_ctlshm; /**< Control structure identified by m_shmid. */
    int m_semid; /**< Semaphore ID, protects shared m_ctlshm. */
  };

} // namespace Roobasf

#endif
