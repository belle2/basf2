/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/LogMethod.h>
#include <framework/pcore/RingBuffer.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <signal.h>

namespace Belle2 {
  class HLTMainLoop {
  public:
    HLTMainLoop();

    bool isRunning() const;
    int writeToRingBufferWaiting(RingBuffer* ringBuffer, int* buffer, unsigned int size) const;
    int readFromRingBufferWaiting(RingBuffer* ringBuffer, int* buffer) const;

  private:
    static bool s_interrupted;
    static void signalHandler(int signalValue);

    void initSignalHandler() const;
  };
}
