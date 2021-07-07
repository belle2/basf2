/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {
  /// Type the messages can have
  enum class EMessageTypes {
    // Used by DAQ and Framework: general control messages
    c_confirmMessage = 'c',   // is sent when an event is confirmed to be at the output process
    c_helloMessage = 'h',     // registration
    c_deleteWorkerMessage = 'd', // Delete the events of a given worker
    c_lastEventMessage = 'l', // indicates the last event was processed by the process
    c_readyMessage = 'r',     // is sent from the worker to the input to show, that it can process an event
    c_terminateMessage = 'x', // Ask all processes to (gently) stop

    // Used by DAQ and Framework: event messages
    c_rawDataMessage = 'u',        // a normal message with event data but in raw format
    c_compressedDataMessage = 'v', // a normal message with event data but in compressed format
    c_eventMessage = 'w',          // a normal message with event data

    // Only needed by framework
    c_goodbyeMessage = 'g',      // un-registration
    c_statisticMessage = 's',    // the last statistics will be send from the output to the process monitor
    c_killWorkerMessage = 'k',   // is sent when a worker should be killed (because it needed too much time)

    // Only needed by DAQ
    c_monitoringMessage = 'm',   // sent in DAQ package to monitor from remote
    c_newRunMessage = 'n',              // sent in DAQ package on starting
  };
}