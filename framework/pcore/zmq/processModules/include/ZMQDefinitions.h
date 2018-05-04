#pragma once

namespace Belle2 {
  /*
    const std::string COM_PROTOCOL = "ipc://";
    const std::string COM_SOCK_PATH = "/tmp/";
    const std::string XPUB_NAME = "xpub";
    const std::string XSUB_NAME = "xsub";
    const std::string SOCK_XPUB_ADDR = COM_PROTOCOL + COM_SOCK_PATH + XPUB_NAME + ".socket";
    const std::string SOCK_XSUB_ADDR = COM_PROTOCOL + COM_SOCK_PATH + XSUB_NAME + ".socket";
  */

  enum class c_MessageTypes {
    c_emptyMessage = '0',
    c_eventMessage = 'e',
    c_endMessage = 't',
    c_readyMessage = 'r',
    c_helloMessage = 'h',
    c_whelloMessage = 'w',
    c_confirmMessage = 'c',
    c_multicastMessage = 'm',
    c_privateMessage = 'p'     // use this for private messages e.g. p<pid> for filter at subscriber
  };

  const bool c_sendMore = true;

}