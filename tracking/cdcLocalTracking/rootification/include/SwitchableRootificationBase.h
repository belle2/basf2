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

#include "RootificationBase.h"

#include <tracking/cdcLocalTracking/config/CDCLocalTrackingConfig.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    // BASF2 defines
    // RelationsObject
    // ClassDef(ClassName, ClassVersion);
    // ClassImp(ClassName);
    // to be used for a full blown rootification.
    // Using theses as base class and macro in a user object
    // enables access from Python as well as access through the DataStore

    // In the scons build system you can activate the additional compile time macro using
    // scons --extra-ccflags='-DCDCLOCALTRACKING_USE_ROOT'.
    // Forgeting to give the flag should result in the correct
    // default behaviour, which is to build without ROOT support. This avoids that one
    // accidentally checks in a version with ROOT switch on.


#ifdef CDCLOCALTRACKING_USE_ROOT
    /// Typedef the normal TObject as the base class of the track finder in case the ROOT inheritance is switched on.
    typedef RootificationBase SwitchableRootificationBase;

#define CDCLOCALTRACKING_SwitchableClassDef(ClassName,ClassVersion) ClassDef(ClassName,ClassVersion);
#define CDCLOCALTRACKING_SwitchableClassImp(ClassName) ClassImp(ClassName);

#else
    // Do not use the ROOT inheritance in all Tracking objects
    // Use an empty base class instead
    /// Empty base class for the switched of ROOT inheritance
    class SwitchableRootificationBaseificationBase { ; };

    /// Typedef the empty class as the base class of the track finder in case the ROOT inheritance is switched off.
    typedef  SwitchableRootificationBaseificationBase SwitchableRootificationBase;

#define CDCLOCALTRACKING_SwitchableClassDef(ClassName,ClassVersion)
#define CDCLOCALTRACKING_SwitchableClassImp(ClassName)

#endif


  } //end namespace CDCLocalTracking

} //end namespace Belle2


#endif // SWITCHABLEROOTIFICATIONBASE_H_
