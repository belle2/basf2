/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>

#include <framework/logging/Logger.h>

#include <chrono>
#include <csignal>
#include <thread>

using namespace std::chrono_literals;
using namespace Belle2;

bool HLTMainLoop::s_interrupted = false;

void HLTMainLoop::signalHandler(int signalValue)
{
  B2RESULT("Termination request...");
  HLTMainLoop::s_interrupted = true;
}

HLTMainLoop::HLTMainLoop()
{
  initSignalHandler();
}

bool HLTMainLoop::isRunning() const
{
  return not s_interrupted;
}

void HLTMainLoop::initSignalHandler() const
{
  struct sigaction action;
  action.sa_handler = signalHandler;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
}

int HLTMainLoop::writeToRingBufferWaiting(RingBuffer* ringBuffer, int* buffer, unsigned int size) const
{
  int irb = 0;
  while (isRunning()) {
    irb = ringBuffer->insq(buffer, size);
    if (irb >= 0) {
      break;
    }
    std::this_thread::sleep_for(20us);
  }

  return irb;
}

int HLTMainLoop::readFromRingBufferWaiting(RingBuffer* ringBuffer, int* buffer) const
{
  int bsize = 0;
  while ((bsize = ringBuffer->remq(buffer)) == 0 and isRunning()) {
    std::this_thread::sleep_for(20us);
  }

  return bsize;
}
