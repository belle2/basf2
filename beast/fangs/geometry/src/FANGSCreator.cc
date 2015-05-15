/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/fangs/geometry/FANGSCreator.h>
#include <beast/fangs/simulation/SensitiveDetector.h>

#include <geometry/CreatorFactory.h>

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the FANG detector */
  namespace fangs {

    // Register the creator
    /** Creator creates the FANG geometry */
    geometry::CreatorFactory<FANGSCreator> FangFactory("FANGSCreator");

    FANGSCreator::FANGSCreator(): claws::CLAWSCreator(new SensitiveDetector()) {}
  } // fangs namespace
} // Belle2 namespace
