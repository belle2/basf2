/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
       A sentinal variable set that does not contain variables and does no extraction.
     */
    template<class AObject>
    class EmptyVarSet IF_NOT_CINT(final) : public BaseVarSet<AObject> {

    private:
      /// Number of floating point values represented by this class.
      static const size_t nVars = 0;

    public:
      /// Constructure taking a optional prefix that can be attached to the names if request.
      explicit EmptyVarSet(const std::string & /*prefix*/ = "") {}

    }; //end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
