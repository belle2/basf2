/// @file ProcHandler.h
/// @brief Process handler class definition
/// @author Soohyung Lee
/// @date Jul 14 2008

#ifndef PROCHANDLER_H
#define PROCHANDLER_H

#include <vector>
#include <sys/types.h>

namespace Belle2 {

  /** A class to manage processes for parallel processing. */
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
    int m_fOutputSrvID;     ///< ID of output server

    key_t m_kSrcShm;        ///< key value for source shared memory
    key_t m_kOutShm;        ///< key value for output shared memory

    static int s_fEvtProcID;       ///< ID of current process

  public:
    /*! Constructor */
    ProcHandler();
    /*! Destructor */
    ~ProcHandler();

    /*! Fork and initialize an event_server process */
    int init_EvtServer();
    /*! Fork and initialize an event process */
    int init_EvtProc(int nproc);
    /*! Fork and initialize an output_server process */
    int init_OutServer(int id);

    /*! Wait until all forked processes are completed */
    int wait_processes();
    /*! Wait until event server process is completed */
    int wait_event_server();
    /*! Wait until all event processes are completed */
    int wait_event_processes();
    /*! Wait until output servers are completed */
    int wait_output_server();

    /*! remove the pid from the process list */
    int remove_pid(pid_t pid);

    /*! Return 1 if the process is mother */
    int isFramework();
    /*! Return 1 if the process is an event_server */
    int isEvtServer();
    /*! Return 1 if the process is an event process */
    int isEvtProc();
    /*! Return 1 if the process is an output_server */
    int isOutputSrv();

    /** Return ID of the current process.
     *
     * Return values mean:
     *  -1:     no parallel processing used
     *  <10000  one of the main processes (between input and output paths)
     *  >=10000 input path
     *  >=20000 output path
     */
    static int EvtProcID();

    // Obsolete functions
    key_t get_srcShmKey();
    key_t get_outShmKey();
  };
}

#endif
