/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SWITCHABLEROOTIFICATIONBASE_H_
#define SWITCHABLEROOTIFICATIONBASE_H_

#include <tracking/trackFindingCDC/rootification/RootificationBase.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/pybasf2/PyStoreArray.h>

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#include <TObject.h>
#include <TClass.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    // The rootification scheme is depricated.
    // We dismantle it step by step.

    // Do not use the ROOT inheritance in all Tracking objects
    // Use an empty base class instead
    /// Empty base class for the switched of ROOT inheritance
    class SwitchableRootificationBaseificationBase { ; };

    /// Typedef the empty class as the base class of the track finder in case the ROOT inheritance is switched off.
    typedef  SwitchableRootificationBaseificationBase SwitchableRootificationBase;

#define TRACKFINDINGCDC_SwitchableClassDef(CLASSNAME,CLASSVERSION)
#define TRACKFINDINGCDC_SwitchableClassImp(CLASSNAME)

  } //end namespace TrackFindingCDC

} //end namespace Belle2


#endif // SWITCHABLEROOTIFICATIONBASE_H_
