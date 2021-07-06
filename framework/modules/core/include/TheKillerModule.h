/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * This Modules kills basf2 as horribly as possible (or selected)
   *
   * With this module you can kill basf2 in a variety of ways to test what happens if processing is interrupted by
   *
   * * std::abort()
   * * std::terminate()
   * * std::quick_exit()
   * * std::exit(N)
   * * uncaught exception
   * * signal
   * * segfault
   * * bus error
   *
   * This error can occur in a selected event to test behavior during processing
   */
  class TheKillerModule final: public Module {
  public:
    /** All the defined methods to kill us ... */
    enum class EMethod {
      c_abort,      /**< call std::abort */
      c_buserror,   /**< produce bus error */
      c_terminate,  /**< call std::terminate */
      c_quick_exit, /**< call std::quick_exit */
      c_exit,       /**< call std::exit */
      c_exception,  /**< raise std::runtime_error */
      c_signal,     /**< raise signal(N) */
      c_segfault,   /**< produce segfault */
    };

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TheKillerModule();

    /** parse the method parameter */
    void initialize() override;

    /** kill if necessary */
    void event() override;

  private:
    /** How to kill the event, one of (abort, terminate, quick_exit, exit, exception, signal, segfault) */
    std::string m_methodStr;
    /** How to kill the event after parsing the string parameter */
    EMethod m_method{EMethod::c_abort};
    /** Optional parameter for the kill method: for quick_exit and exit it is the return code, for signal it is the signal number */
    unsigned int m_parameter;
    /** In which event to kill the processing */
    int m_eventToKill;
    /** Current event */
    int m_event{0};
  };
}

