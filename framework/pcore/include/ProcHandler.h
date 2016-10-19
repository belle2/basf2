/// @file ProcHandler.h
/// @brief Process handler class definition
/// @author Soohyung Lee
/// @date Jul 14 2008

#ifndef PROCHANDLER_H
#define PROCHANDLER_H

#include <set>
#include <string>

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
    /** Fork and initialize 'nproc' worker processes. */
    void startWorkerProcesses(int nproc);
    /** There is no real output process, but marks current process as output. */
    void startOutputProcess();

    /** Wait until all forked processes handled by this ProcHandler are completed */
    void waitForAllProcesses();

    /** Returns true if multiple processes have been spawned, false in single-core mode. */
    static bool parallelProcessingUsed();
    /** Return true if the process is an input process */
    static bool isInputProcess();
    /** Return true if the process is a worker process */
    static bool isWorkerProcess();
    /** Return true if the process is an output process */
    static bool isOutputProcess();


    /** Return number of worker processes */
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

  private:
    std::set<int> m_processList;  /**< PIDs of processes controlled by this ProcHandler. */

  };
}

#endif
