//
// Created by abaur on 02.05.18.
//

#pragma once

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/Path.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

namespace Belle2 {

  class ProcHandler;

  /**
    This class provides the core event processing loop for parallel processing.
  */
  class pEventProcessor : public EventProcessor {
  public:

    /** Constructor */
    pEventProcessor(const std::string& socketAddress);

    /** Destructor */
    virtual ~pEventProcessor();

    /** Processes the full module chain using parallel processing, starting with the first module in the given path. */
    /**
        Processes all events for the given run number and for events from 0 to maxEvent.
        \param spath The processing starts with the first module of this path.
        \param maxEvent The maximum number of events that will be processed.
            If the number is smaller or equal 0, all events will be processed.
    */
    void process(PathPtr spath, long maxEvent);

    /** signal handler for Ctrl+C (async-safe)
     *
     *  When called the first time, does nothing (input process handles SIGINT by itself).
     *  produced (mostly equivalent to previous behaviour on Ctrl+C)
     * */
    void gotSigINT();

    /** clean up IPC resources (should only be called in one process). */
    void cleanup();


  private:
    /** TFiles are stored in a global list and cleaned up by root
     * since this will happen in all forked processes, these will be corrupted if we don't clean the list!
     *
     * needs to be called at the end of every process.
     */
    void clearFileList();

    /** Tries a soft shutdown when this fails -> hard kill */
    void terminateProcesses(ModulePtrList* localModules, const ModulePtrList& prependModules);

    /** Send zmq message across multicast */
    void sendPCBMessage(const c_MessageTypes msgType,  const std::string& data = "");

    /** handler to fork and manage processes. */
    std::unique_ptr<ProcHandler> m_procHandler;

    /** are there forked processes? */
    bool m_multicastOnline = false;

    const std::string m_socketAddress;
  };
}