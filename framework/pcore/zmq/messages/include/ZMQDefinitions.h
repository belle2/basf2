/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Anselm Baur                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {
  /// Type the messages can have
  enum class c_MessageTypes {
    c_helloMessage = 'h', // is sent by all processes on startup
    c_terminateMessage = 't', // is sent by all processed on termination
    c_stopMessage = 's', // Ask all processes to (gently) stop
    c_deleteMessage = 'd', // Delete the events of a given worker
    c_eventMessage = 'e', // a normal message with event data
    c_endMessage = 'q', // indicates the last event was processed by the input process (sent to worker and output)
    c_readyMessage = 'r', // is sent from the worker to the input to show, that it can process an event
    c_whelloMessage = 'w', // is sent from the workers to the input for initialisation
    c_confirmMessage = 'c', // is sent when an event is confirmed to be at the output process
    c_deathMessage = 'k', // is sent when a worker should be killed (because it needed too much time)
    c_statisticMessage = 'f', // the last statistics will be send from the output to the process monitor
  };
}