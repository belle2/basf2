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

#include <fwcore/Module.h>
#include <fwcore/Path.h>
#include <fwcore/Framework.h>
#include <fwcore/ModuleParamList.h>

#include <gearbox/Gearbox.h>

namespace Belle2 {

  class Kbasf2PyModule {

  public:

    static void embeddPythonModule() throw(FwExcPythonModuleNotEmbedded);

  };

} //end of namespace Belle2

#endif /* KBASF2MODULE_H_ */

