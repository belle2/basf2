/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <string>

#define D2_SOCKBUF_SIZE  8000000

namespace Belle2 {
  class HLTSocket {
  public:
    virtual ~HLTSocket();

    int put(char* data, int len);
    int put_wordbuf(int* data, int len);
    int get(char* data, int len);
    int get_wordbuf(int* data, int len);

    bool accept(unsigned int port);
    bool connect(const std::string& hostName, unsigned int port, const HLTMainLoop& mainLoop);
    bool initialized() const;
    void deinitialize();

  private:
    int read_data(char* data, int len);
    int write_data(char* data, int len);

    void close(int socket);

    int m_socket = -1;
    int m_listener = -1;
    bool m_initialized = false;
  };
}






