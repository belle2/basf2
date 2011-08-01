/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include <framework/gearbox/Unit.h>

#include <string>
#include <vector>
#include <iomanip>

namespace Belle2 {
  /** Utility functions related to filename validation and filesystem access */
  namespace FileSystem {
    /** Check if a given filename exits */
    bool fileExists(const std::string& filename);
    /** Check if the dir containing the filename exists */
    bool fileDirExists(const std::string& filename);
    /** Check if filename points to an existing file */
    bool isFile(const std::string& filename);
    /** Check if filename points to an existing directory */
    bool isDir(const std::string& filename);

    /**
     * Load a shared library
     * @param library  Name of the library
     * @param fullname If false, the Library name is interpreted as short name
     *                 like given to the compiler with -l. It will be expanded
     *                 to lib<library>.so
     */
    bool loadLibrary(std::string library, bool fullname = true);
  }

  namespace Utils {
    /**
     * Return current value of the high performance clock.
     *
     * The returned value is meant to measure relative times and does not show
     * absolute time values;
     *
     * @return Clock value in default time unit (ns)
     */
    double getClock();
  }

  /**
   * Print an INFO message txt followed by the time it took to execute the
   * statment given as second argument.
   *
   * Care should be taken not to define variables in this call since they will
   * be scoped and thus unavaiable after the macro
   */
#define B2INFO_MEASURE_TIME(txt, ...) {\
    double __b2_elapsed_time__ = ::Belle2::Utils::getClock();\
    {__VA_ARGS__;}\
    __b2_elapsed_time__ = (::Belle2::Utils::getClock() - __b2_elapsed_time__)/Unit::ms;\
    B2INFO(txt << std::fixed << std::setprecision(3) << __b2_elapsed_time__ << " ms");\
  }

} // Belle2 namespace

#endif //CORE_FUNCTIONS_H
