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
  public:
    /** Constructor */
    ProcHandler();
    /** Destructor */
    ~ProcHandler();

    /** Fork and initialize an input process */
    void startInputProcess();
    /** Fork and initialize an event process */
    void startEventProcesses(int nproc);
    /** Fork and initialize an output process */
    void startOutputProcess(int id);

    /** Wait until all forked processes are completed */
    void waitForAllProcesses();
    /** Wait until input process is completed */
    void waitForInputProcesses();
    /** Wait until all event processes are completed */
    void waitForEventProcesses();
    /** Wait until output processes are completed */
    void waitForOutputProcesses();

    /** Return true if the process is mother (aka. global process)
     *
     * Also true if no parallel processing is used.
     */
    static bool isFramework();
    /** Return true if the process is an input process */
    static bool isInputProcess();
    /** Return true if the process is an event process */
    static bool isEventProcess();
    /** Return true if the process is an output process */
    static bool isOutputProcess();

    /** Return ID of the current process.
     *
     * Return values mean:
     *  -1:     no parallel processing used (or global process)
     *  <10000  one of the main processes (between input and output paths)
     *  >=10000 input path
     *  >=20000 output path
     */
    static int EvtProcID();

  private:
    /** wait for all processes in the given list to terminate. */
    void waitForProcesses(std::vector<pid_t>& pids);

    std::vector<pid_t> m_inputProcessList;  //< PIDs of input processes
    std::vector<pid_t> m_eventProcessList;  //< PIDs of event processes
    std::vector<pid_t> m_outputProcessList; //< PIDs of output processes

    static int s_processID;       ///< ID of current process

  };
}

#endif
