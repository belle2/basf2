/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>

#include <tracking/trackFindingCDC/utilities/Backtrace.h>

#include <framework/logging/Logger.h>

#include <typeinfo>

using namespace Belle2;
using namespace TrackFindingCDC;

ProcessingSignalListener::ProcessingSignalListener() = default;

ProcessingSignalListener::~ProcessingSignalListener()
{
  if (m_initialized) {
    B2ERROR("Processing signal listener " << m_initializedAs << " was not terminated after initialization");
  }
}

void ProcessingSignalListener::initialize()
{
  if (m_initialized) {
    B2ERROR(typeid(*this).name() << " has been initialized before");
    printBacktrace(LogConfig::c_Warning, 10);
  }
  m_initializedAs = typeid(*this).name();
  m_initialized = true;
}

void ProcessingSignalListener::beginRun()
{
  if (not m_initialized) {
    B2ERROR(typeid(*this).name() << " was not initialized");
  }
}

void ProcessingSignalListener::beginEvent()
{
  if (not m_initialized) {
    B2ERROR(typeid(*this).name() << " was not initialized");
  }
}

void ProcessingSignalListener::endRun()
{
  if (not m_initialized) {
    B2ERROR(typeid(*this).name() << " was not initialized");
  }
}

void ProcessingSignalListener::terminate()
{
  if (m_terminated) {
    B2ERROR(typeid(*this).name() << " has been terminated before");
    printBacktrace(LogConfig::c_Warning, 10);
  }
  m_terminated = true;

  if (not m_initialized) {
    B2ERROR(typeid(*this).name() << " was not initialized before termination");
    printBacktrace(LogConfig::c_Warning, 10);
  }
  m_initialized = false;
}
