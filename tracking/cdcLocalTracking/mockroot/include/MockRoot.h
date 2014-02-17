/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MOCKROOT_H_
#define MOCKROOT_H_

#include "ToggleMockRoot.h"
#include <tracking/cdcLocalTracking/config/CDCLocalTrackingConfig.h>
#include <TObject.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    //ROOT defines
    //TObject
    //ClassDef(ClassName, ClassVersion);
    //ClassImp(ClassName);

    // we fake it in case we do not want to use ROOT


#ifdef CDCLOCALTRACKING_USE_ROOT

    typedef TObject UsedTObject;

#define ClassDefInCDCLocalTracking(ClassName,ClassVersion) ClassDef(ClassName, ClassVersion)
#define ClassImpInCDCLocalTracking(ClassName) ClassImp(ClassName);

#else
    // Do not use the ROOT inheritance in all Tracking objects
    // Use an empty base class instead
    class MockTObject { ; };
    typedef MockTObject UsedTObject;

#define ClassDefInCDCLocalTracking(ClassName,ClassVersion)
#define ClassImpInCDCLocalTracking(ClassName)

#endif


  } //end namespace CDCLocalTracking

} //end namespace Belle2


#endif //TOGGLEMOCKROOT_H_  
