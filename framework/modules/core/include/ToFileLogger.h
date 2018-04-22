#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 **************************************************************************/

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {
  /**
   * For better handling non BASF2-Logging lines, you can insert this module.
   * It redirect all output to the console into the filename given as parameter.
   * With the usage of the basf2-own logging framework you can even write the basf2-related logging
   * into another file as the non-basf2-related if you like.
   */
  class ToFileLoggerModule : public Module {

  public:

    /** Constructor */
    ToFileLoggerModule();

    /** Init the module. */
    virtual void initialize();
    /** Don't break the terminal. */
    virtual void terminate();

  protected:
    /** Filename to log into. */
    std::string m_param_fileName;

    /** File Device of Stdout. Will be filled in initialize. */
    int m_fileDeviceOfStdout;

    /** File Device of Stderr. Will be filled in initialize. */
    int m_fileDeviceOfStderr;
  };
} // end namespace Belle2
