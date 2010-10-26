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
    ProcHandler(void);
    ~ProcHandler(void);

    int init_EvtServer(void);
    int init_EvtProc(int nproc);
    int init_OutServer(int id);

    int wait_processes(void);
    int wait_output_server(void);
    int remove_pid(pid_t pid);

    int isFramework(void);
    int isEvtServer(void);
    int isEvtProc(void);
    int isOutputSrv(void);

    static int EvtProcID(void);

    key_t get_srcShmKey(void);
    key_t get_outShmKey(void);

    int isproc(int pid, char* exe);
    void display(void);
  };

} // namespace Roobasf

#endif
