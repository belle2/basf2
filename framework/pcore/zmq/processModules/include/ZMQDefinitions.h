#pragma once

namespace Belle2 {


  enum class c_MessageTypes {
    //c_notypeMessage = '',
    c_emptyMessage = '0',
    c_eventMessage = 'e',
    c_endMessage = 't',
    c_readyMessage = 'r',
    c_helloMessage = 'h',
    c_whelloMessage = 'w',
    c_confirmMessage = 'c',
    c_broadcastMessage = 'b',
    c_privateMessage = 'p'     // use this for private messages e.g. p<pid> for filter at subscriber
  };

  const bool c_sendMore = true;

}