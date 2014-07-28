/// @file ProcHandler.h
/// @brief Process handler class definition
/// @author Soohyung Lee
/// @date Jul 14 2008

#ifndef PROCHANDLER_H
#define PROCHANDLER_H

#include <vector>
#include <string>
#include <sys/types.h>

namespace Belle2 {

  /** A class to manage processes for parallel processing. */
  class ProcHandler {
  public:
    /** Constructor */
    ProcHandler();
    /** Destructor */
    ~ProcHandler();

    /** Fork and initialize an input process */
    void startInputProcess();
    /** Fork and initialize an event process */
    void startEventProcesses(int nproc);
    /** There is no real output process, but marks current process as output. */
    void startOutputProcess();

    /** Wait until all forked processes are completed */
    void waitForAllProcesses();
    /** Wait until input process is completed */
    void waitForInputProcesses();
    /** Wait until all event processes are completed */
    void waitForEventProcesses();
    /** Wait until output processes are completed */
    void waitForOutputProcesses();

    /** Returns true if multiple processes have been spawned, false in single-core mode. */
    static bool parallelProcessingUsed();
    /** Return true if the process is an input process */
    static bool isInputProcess();
    /** Return true if the process is an event process */
    static bool isEventProcess();
    /** Return true if the process is an output process */
    static bool isOutputProcess();



    /** Return ID of the current process.
     *
     * Return values mean:
     *  -1:     no parallel processing used
     *  <10000  one of the main processes (between input and output paths). In range 0..Environment::getInstance().getNumberProcesses()-1.
     *  >=10000 input path
     *  >=20000 output path
     */
    static int EvtProcID();

    /** Get a name for this process. (input, event, output...). */
    static std::string getProcessName();

  private:
    /** wait for all processes in the given list to terminate. */
    void waitForProcesses(std::vector<pid_t>& pids);

    std::vector<pid_t> m_inputProcessList;  /**< PIDs of input processes. */
    std::vector<pid_t> m_eventProcessList;  /**< PIDs of event processes. */
    std::vector<pid_t> m_outputProcessList; /**< PIDs of output processes. */

    static int s_processID;       ///< ID of current process

  };
}

#endif
