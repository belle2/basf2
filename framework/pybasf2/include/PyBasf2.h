/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PYBASF2_H_
#define PYBASF2_H_

#include <framework/core/FrameworkExceptions.h>

namespace Belle2 {

  //Define exceptions
  /** Exception is thrown if the basf2 Python module could not be embedded into Python. */
  BELLE2_DEFINE_EXCEPTION(PythonModuleNotEmbeddedError, "Could not embed the basf2 Python module !");

  /**
   * The Pybasf2 class.
   *
   * This class defines the Python basf2 module. Import this module
   * in a plane python script to have the basf2 framework available.
   */
  class PyBasf2 {

  public:

    /** Embeds the Python module. */
    static void embedPythonModule() throw(PythonModuleNotEmbeddedError);

  };

} //end of namespace Belle2

#endif /* PYBASF2_H_ */
