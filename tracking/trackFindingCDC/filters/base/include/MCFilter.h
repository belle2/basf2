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

namespace Belle2 {
  namespace TrackFindingCDC {
    /// MC Filter Type using a VarSet and the truth variable in it.
    template<class ATruthVarSet>
    class MCFilter : public FilterOnVarSet<ATruthVarSet> {

    public:
      /// Type of the super class.
      typedef FilterOnVarSet<ATruthVarSet> Super;
      /// Type of the handled object.
      typedef typename ATruthVarSet::Object Object;

    public:
      /// Constructor.
      MCFilter() : Super() { }

      /// Reject an item if the truth variable is 0, else accept it.
      virtual Weight operator()(const Object& object) override
      {
        Super::operator()(object);
        const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValuesWithPrefix();

        if (varSet.at("truth") == 0.0)
          return NAN;
        else
          return 1.0;
      }
    };
  }
}
