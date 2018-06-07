#pragma once

namespace Belle2 {
  enum class c_MessageTypes {
    c_startMessage = '1', // sent from pEventProcessor when worker allowed to start
    c_stopMessage = '2',
    c_deleteMessage = '3',
    c_terminateMessage = '4',
    c_emptyMessage = 'n',
    c_eventMessage = 'e',
    c_endMessage = 'q',
    c_readyMessage = 'r',
    c_helloMessage = 'h',
    c_whelloMessage = 'w',
    c_confirmMessage = 'c',
    c_multicastMessage = 'm',
    c_deathMessage = 'd',
    c_privateMessage = 'p'     // use this for private messages e.g. p<pid> for filter at subscriber
  };

  const bool c_sendMore = true;

}