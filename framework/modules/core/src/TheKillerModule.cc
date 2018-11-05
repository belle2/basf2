/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/core/TheKillerModule.h>
#include <signal.h>
#include <boost/algorithm/string.hpp>
#include <TRandom.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TheKiller);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TheKillerModule::TheKillerModule() : Module()
{
  // Set module properties
  setDescription(R"DOC("This Modules kills basf2 as horribly as possible (or as selected)

With this module you can kill basf2 in a variety of ways to test what happens if processing is interrupted by

* std::abort()
* std::terminate()
* std::quick_exit()
* std::exit(N)
* uncaught exception
* signal
* segfault

This error can occur in a selected event to test behavior during processing.)DOC");

  // Parameter definitions
  addParam("method", m_methodStr, "How to kill the event, one of (abort, terminate, "
           "quick_exit, exit, exception, signal, segfault, random)", std::string("abort"));
  addParam("parameter", m_parameter, "Optional parameter for the kill method: for "
           "quick_exit and exit it is the return code, for signal it is the signal number", 0u);
  addParam("event", m_eventToKill, "In which event to kill the processing");
}

void TheKillerModule::initialize()
{
  boost::to_lower(m_methodStr);
  if (m_methodStr == "abort") m_method = EMethod::c_abort;
  else if (m_methodStr == "terminate") m_method = EMethod::c_terminate;
  else if (m_methodStr == "quick_exit") m_method = EMethod::c_quick_exit;
  else if (m_methodStr == "exit") m_method = EMethod::c_exit;
  else if (m_methodStr == "exception") m_method = EMethod::c_exception;
  else if (m_methodStr == "signal") m_method = EMethod::c_signal;
  else if (m_methodStr == "segfault") m_method = EMethod::c_segfault;
  else B2ERROR("Unknown method , choose one of (abort, terminate, quick_exit, exit, "
                 "exception, signal, segfault)" << LogVar("method", m_methodStr));
}

void TheKillerModule::event()
{
  if (++m_event < m_eventToKill) return;
  switch (m_method) {
    case EMethod::c_abort:
      std::abort();
    case EMethod::c_terminate:
      std::terminate();
    case EMethod::c_quick_exit:
      std::quick_exit(m_parameter);
    case EMethod::c_exit:
      std::exit(m_parameter);
    case EMethod::c_exception:
      throw std::runtime_error("DIE! DIE! DIE!");
    case EMethod::c_signal:
      if (raise(m_parameter) != 0) B2FATAL("Invalid signal number" << LogVar("signal", m_parameter));
      break;
    case EMethod::c_segfault:
      volatile int* foo {nullptr};
      *foo = 5;
      B2FATAL("This should never be called ...");
      break;
  }
}
