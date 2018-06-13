/// @file ProcHandler.h
/// @brief Process handler class definition
/// @author Soohyung Lee
/// @date Jul 14 2008

#pragma once

#include <zmq.hpp>
#include <set>
#include <string>
#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

namespace Belle2 {

  /** A class to manage processes for parallel processing. */
  class ProcHandler {
  public:
    /** Constructor
     *
     * Children marked as local will not cause the parent process to die
     * in case of anormal termination. Errors can be detected using
     * the return value of waitForAllProcesses().
     * Note that you probably shouldn't have multiple ProcHandlers with
     * local children running at the same time.
     */
    ProcHandler(unsigned int nWorkerProc, bool markChildrenAsLocal = false);
    /** Destructor */
    ~ProcHandler();

    /** Fork and initialize an input process */
    void startInputProcess();
    /** Fork and initialize worker processes. */
    void startWorkerProcesses();
    /** There is no real output process, but marks current process as output. */
    void startOutputProcess();

    void startProxyProcess(std::string xpubProxySocketName, std::string xsubProxySocketName);

    void setAsMonitoringProcess();

    /** Wait until all forked processes handled by this ProcHandler terminate.
     *
     * In case the markChildrenAsLocal option was set when a process was started
     * and this process terminated anormaly (via signal or non-zero exit code), the
     * return value is set to false. Otherwise, true is returned.
     */
    bool waitForAllProcesses();




    /** Returns true if multiple processes have been spawned, false in single-core mode. */
    static bool parallelProcessingUsed();
    /** Return true if the process is the same ProcType then the parameter*/
    static bool isProcess(ProcType procType);
    static bool isOutputProcess();
    static bool isWorkerProcess();



    /** Return number of worker processes (configured value, not current) */
    static int numEventProcesses();

    /** Return list of PIDs managed by this ProcHandler instance. */
    std::set<int> processList() const;
    /** Return list of all PIDs (from all ProcHandler instances). */
    static std::set<int> globalProcessList();

    /** Return ID of the current process.
     *
     * Return values mean:
     *  -1:     no parallel processing used
     *  <10000  one of the worker processes (between input and output paths). In range 0..Environment::getInstance().getNumberProcesses()-1.
     *  >=10000 input path
     *  >=20000 output path
     */
    static int EvtProcID();

    /** Get a name for this process. (input, event, output...). */
    static std::string getProcessName();

    /* TODO : candidate for helper function because duplicated in the ZMQModule
     ** PCB functions for the multicast
     */
    void initPCBMulticast(std::string& xpubProxySocketAddr, std::string& xsubProxySocketAddr);
    void stopPCBMulticast();
    void subscribePCBMulticast(c_MessageTypes filter);
    bool isPCBMulticast();

    /** Send zmq message across multicast */
    void sendPCBMessage(const c_MessageTypes msgType,  const std::string& data = "");


    /** Wait for input and output process ready */
    bool waitForStartEvtProc();
    /** When input and output ready worker can start with event processing */
    void startEvtProc();
    // TODO : stopEvtProc()
    /** Sends the terminate Message */
    void stopEvtProc();
    /** Sends stop signal across multicast when possible after timeout hard kill*/
    void killAllChildProc(unsigned int timeout = 5);

    void deleteFromPidList(unsigned int pid);



  private:
    /** Start a new process, adding its PID to processList, and setting s_processID = id. Returns true in child process. */
    bool startProc(std::set<int>* processList, const std::string& procType, int id);

    bool m_markChildrenAsLocal; /**< Anormal termination of child will not stop parent, waitForAllProcesses() returns status. */
    std::set<int> m_processList;  /**< PIDs of processes controlled by this ProcHandler. */
    std::set<int> m_pidList; /**< list of the os pids*/
    unsigned int m_numWorkerProcesses; /**< Number of worker processes controlled by this ProcHandler. */

    std::unique_ptr<zmq::context_t> m_context = nullptr;
    std::unique_ptr<zmq::socket_t> m_pubSocket = nullptr;
    std::unique_ptr<zmq::socket_t> m_subSocket = nullptr;

    /** Multicast status  */
    bool m_statusPCBMulticast = false;
  };
}
