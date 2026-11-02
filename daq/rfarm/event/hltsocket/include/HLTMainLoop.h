/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/pcore/RingBuffer.h>

namespace Belle2 {
  class HLTMainLoop {
  public:
    HLTMainLoop();

    static bool isRunning();
    static int writeToRingBufferWaiting(Belle2::RingBuffer* ringBuffer, const int* buffer, unsigned int size);
    static int readFromRingBufferWaiting(RingBuffer* ringBuffer, int* buffer);

  private:
    static bool s_interrupted;
    static void signalHandler(int signalValue);

    static void initSignalHandler();
  };
}
