/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROOTIFICATIONBASE_H_
#define ROOTIFICATIONBASE_H_


#include <framework/datastore/RelationsObject.h>
#include <tracking/cdcLocalTracking/config/CDCLocalTrackingConfig.h>


namespace Belle2 {

  namespace CDCLocalTracking {

#ifdef CDCLOCALTRACKING_USE_ROOT
    const bool ROOTIFICATIONNBASE_IS_ACTIVE = true;
#else
    const bool ROOTIFICATIONNBASE_IS_ACTIVE = false;
#endif

    /** Base class for the ROOT inheritance.
     *  This class unconditionally inherits from ROOT,
     *  so it can always be ask if it is the active base class from Python.
     */
    class RootificationBase : public RelationsObject {

    public:
      /// Returns true, if this class is the active base and all other objects are accessable from Python.
      static bool getIsActive() { return ROOTIFICATIONNBASE_IS_ACTIVE; }

    private:
      static const bool s_isActive = ROOTIFICATIONNBASE_IS_ACTIVE;

    private:
      /// ROOT Macro to unconditionally make RootificationBase a ROOT class.
      ClassDef(RootificationBase, 1);

    };

  } //end namespace CDCLocalTracking

} //end namespace Belle2


#endif // ROOTIFICATIONBASE_H_  
