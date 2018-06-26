#pragma once

namespace Belle2 {
  enum class c_MessageTypes {
    c_helloMessage = 'h', // is sent by all processes on startup
    c_terminateMessage = 't', // is sent by all processed on termination
    c_stopMessage = '2', // Ask all processes to stop
    c_deleteMessage = '3', // TODO where is this used??
    c_eventMessage = 'e', // a normal message with event data
    c_endMessage = 'q', // indicates the last event reached the input process for confirmation
    c_readyMessage = 'r', // is sent from the worker to the input to show, that it can process an event
    c_whelloMessage = 'w', // is sent from the workers to the input
    c_confirmMessage = 'c', // is sent when an event is confirmed to be at the output process
    c_deathMessage = 'd', // is sent when a worker should be killed (because it needed too much time)
  };

  const bool c_sendMore = true;

}