/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/Path.h>
#include <framework/pcore/zmq/utils/ZMQParent.h>

namespace Belle2 {
  /**
   * EventProcessor to be used on the HLT with all specialities
   * of the HLT processing:
   * * no input or output path - just workers
   * * multiprocessing forked out after initialization, which is *before* the first real event is processed
   * * restart of terminated workers (configurable) and unregistration and DQM server and collector
   * * special run start/end handling: please check the HLTZMQ2DsModule for details
   * * check of the child processes in an additional monitoring process (happens to be the parent process) by keeping track of the current state
   *
   * This event processor is specialies to the HLT and should *not* be used apart from that.
   * The event processor is exported as a python module. It can be called with
   *
   *  import hbasf2
   *  hbasf2.process(path, [address, ...], True)
   */
  class HLTEventProcessor : public EventProcessor {
  public:
    /// Create a new event processor and store the ZMQ addresses where to unregister workers
    HLTEventProcessor(const std::vector<std::string>& outputAddresses);

    /**
     * Process the given path. If requested, restart failed workers (or not)
     * For this,
     * * first check if all modules have a parallel flag
     * * then call initialize in the main process
     * * fork out all workers and process the path
     * * while monitoring their status
     * * in the end kill remaining processes (if needed) and re-raise any collected signals
     */
    void process(PathPtr spath, bool restartFailedWorkers);

  private:
    /// An instance of a ZMQParent to create sockets for unregistering workers
    ZMQParent m_parent;
    /// The current list of running processes (with their PIDs)
    std::vector<int> m_processList;
    /// The created sockets for unregistering workers. TODO: use connections
    std::vector<std::unique_ptr<zmq::socket_t>> m_sockets;

    /// Send an unregister message to all sockets if the given PID died. Wait at max 10s for the confirmation (if requested)
    void sendTerminatedMessage(unsigned int pid, bool waitForConfirmation);
    /// Fork out as much workers as requested and in each run the given path using processCore
    void runWorkers(PathPtr path, unsigned int numProcesses);
    /**
     * Process the path by basically calling processEvent until a termination is requested. #
     * Will not any initialization - it is assumed this has already happened before.
     * In the end, terminate is called.
     */
    void processCore(PathPtr path);

    /**
     * Process a single event by iterating through the module path once.
     * In principle very similar to the EventProcessor::processEvent function,
     * but has different assumptions for the run changes happening induced by the
     * master module (which is always the ZMQ2Ds module in the HLT case).
     *
     * The logic happening after the master module is the following:
     * * if an EndOfData is set in the event meta data, just break out
     * * if a HLT-specific EndOfRun is set, call the end of run methods of all modules (without begin run)
     * * if the previous event meta data has the EndOfRun/Data set, call the begin run functions
     * * if the run change was induced due to a changing run number, call begin and end run functions
     *
     * The rest (e.g. module conditions, db store) is the same as the EventProcessor case.
     */
    bool processEvent(PathIterator moduleIter, bool firstRound);

    /**
     * Check if one of the started processes has died.
     * If it has died with a non-zero exit code, increase the counter of workers to restart.
     *
     * Returns the number of still alive workers and the number of workers needed
     * to restart (aka the number of workers that died with a non-zero exit code) as
     * a pair.
     */
    std::pair<unsigned int, unsigned int> checkChildProcesses();

    /// Release the parent resource, which is needed after forking to not close it twice.
    void release();

    /// Helper function to fork out. Sets the Python state correctly and adds the process to the internal state
    bool forkOut();
  };
}