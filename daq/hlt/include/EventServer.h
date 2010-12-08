/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *               Ryosuke Itoh                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTSERVER_H
#define EVENTSERVER_H

#include <string>
#include <vector>

#include "framework/pcore/RingBuffer.h"

namespace Belle2 {

  class EventServer {
  public:
    EventServer(void);
    EventServer(RingBuffer* inBuffer, RingBuffer* outBuffer);
    EventServer(std::vector<std::string> inBuffer, std::vector<std::string> outBuffer);
    virtual ~EventServer(void);

    virtual void initialize() {};
    virtual void beginRun() {};
    virtual void event() {};
    virtual void endRun() {};
    virtual void terminate() {};

    void process(const std::string data);

    virtual void eventServer() {};

  private:
    RingBuffer* m_inBuffer;
    RingBuffer* m_outBuffer;
  };
}

#endif
