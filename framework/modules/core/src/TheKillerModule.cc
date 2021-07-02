/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/modules/core/TheKillerModule.h>
#include <csignal>
#include <boost/algorithm/string.hpp>

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
* bus error

This error can occur in a selected event to test behavior during processing.)DOC");

  // Parameter definitions
  addParam("method", m_methodStr, "How to kill the event, one of (abort, terminate, "
           "quick_exit, exit, exception, signal, segfault, buserror)", std::string("abort"));
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
  else if (m_methodStr == "buserror") m_method = EMethod::c_buserror;
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
#ifndef __clang_analyzer__
      {
        // this is an intentional nullptr dereference so lets hide it from clang analyzer
        volatile int* foo {nullptr};
        *foo = 5;
      }
#endif
      break;
    case EMethod::c_buserror:
#ifndef __clang_analyzer__
      {
#if defined(__GNUC__) && defined(__x86_64__)
        __asm__("pushf\norl $0x40000,(%rsp)\npopf");
#endif
        auto* cptr = (char*) malloc(sizeof(int) + 1);
        auto* iptr = (int*)(cptr + 1);
        *iptr = 42;
        free(cptr);
      }
#endif
      break;
    default:
      B2FATAL("Illegal method");
  }
  B2FATAL("This should never be called ...");
}
