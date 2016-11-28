/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// MC Filter Type using a VarSet and the truth variable in it.
    template<class ATruthVarSet>
    class MCFilter : public OnVarSet<Filter<typename ATruthVarSet::Object> > {

    private:
      /// Type of the super class.
      using Super = OnVarSet<Filter<typename ATruthVarSet::Object> > ;

    public:
      /// Type of the handled object.
      using Object = typename ATruthVarSet::Object;

    public:
      /// Constructor.
      MCFilter() : Super(makeUnique<ATruthVarSet>())
      {
      }

      /// Reject an item if the truth variable is 0, else accept it.
      Weight operator()(const Object& object) override
      {
        Super::operator()(object);
        MayBePtr<Float_t> truth = this->getVarSet().find("truth");

        if (not truth or (*truth) == 0.0) {
          return NAN;
        } else {
          return 1.0;
        }
      }
    };
  }
}
