/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KBASF2MODULE_H_
#define KBASF2MODULE_H_

#include <boost/python.hpp>

#include <framework/core/Module.h>
#include <framework/core/Path.h>
#include <framework/core/Framework.h>
#include <framework/core/ModuleParam.h>

#include <framework/gearbox/Gearbox.h>

namespace Belle2 {

  //!  The Kbasf2PyModule class.
  /*!
     This class defines the Python basf2 module.
  */
  class Kbasf2PyModule {

  public:

    //! Embeds the Python module.
    static void embedPythonModule() throw(FwExcPythonModuleNotEmbedded);

  };

} //end of namespace Belle2

#endif /* KBASF2MODULE_H_ */

