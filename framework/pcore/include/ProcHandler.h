/// @file prochandler.h
/// @brief Process handler class definition
/// @author Soohyung Lee
/// @date Jul 14 2008

#ifndef PROCHANDLER_H
#define PROCHANDLER_H

#include <vector>
#include <sys/types.h>

//#include "shm.h"
//#include "evtprocess.h" // DEPRECATED

namespace Belle2 {

/// @class ProcHandler
/// @brief Process handler class definition
  /*! A class to manage processes for parallel processing */
  class ProcHandler {
  private:
    int m_nEvtSrv;          ///< # of event servers
    int m_nEvtProc;         ///< # of event processes
    int m_nOutputSrv;       ///< # of output servers

    std::vector<pid_t> m_lEvtSrv;         ///< pid of event servers
    std::vector<pid_t> m_lEvtProc;        ///< pid of event processes
    std::vector<pid_t> m_lOutputSrv;      ///< pid of output servers

    int m_fEvtServer;       ///< flag for event server (1:evtserver)
    int m_fEvtProc;         ///< flag for event process (1:evtproc)
    int m_fOutputSrv;       ///< flag for output server (1:outputsrv)
    int m_fEvtServerID;     ///< ID of event server
    static int m_fEvtProcID;       ///< ID of event process
    int m_fOutputSrvID;     ///< ID of output server

    key_t m_kSrcShm;        ///< key value for source shared memory
    key_t m_kOutShm;        ///< key value for output shared memory

  public:
    /*! Constructor */
    ProcHandler(void);
    /*! Destructor */
    ~ProcHandler(void);

    /*! Fork and initialize an event_server process */
    int init_EvtServer(void);
    /*! Fork and initialize an event process */
    int init_EvtProc(int nproc);
    /*! Fork and initialize an output_server process */
    int init_OutServer(int id);

    /*! Wait until all forked processes are completed */
    int wait_processes(void);
    /*! Wait untill event server process is completed */
    int wait_event_server(void);
    /*! Wait until all event processes are completed */
    int wait_event_processes(void);
    /*! Wait until output servers are completed */
    int wait_output_server(void);

    /*! remove the pid from the process list */
    int remove_pid(pid_t pid);

    /*! Return 1 if the process is mother */
    int isFramework(void);
    /*! Return 1 if the process is an event_server */
    int isEvtServer(void);
    /*! Return 1 if the process is an event process */
    int isEvtProc(void);
    /*! Return 1 if the process is an output_server */
    int isOutputSrv(void);

    /** Return ID of the current process.
     *
     * Return values mean:
     *  -1:     no parallel processing used
     *  <10000  one of the main processes (between input and output paths)
     *  >=10000 input path
     *  >=20000 output path
     */
    static int EvtProcID(void);

    // Obsolete functions
    key_t get_srcShmKey(void);
    key_t get_outShmKey(void);

    int isproc(int pid, char* exe);
    void display(void);
  };
}

#endif
