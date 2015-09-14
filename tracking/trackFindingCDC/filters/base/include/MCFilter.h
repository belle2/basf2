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
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base MC Filter
    template<class TruthVarSet>
    class MCFilter : public FilterOnVarSet<TruthVarSet> {

    public:
      /// Type of the super class
      typedef FilterOnVarSet<TruthVarSet> Super;
      typedef typename TruthVarSet::Object Object;

    public:
      /// Constructor
      MCFilter() : Super() { }

      virtual CellWeight operator()(const Object& object) IF_NOT_CINT(override)
      {
        Super::operator()(object);
        const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValuesWithPrefix();

        if (varSet.at("truth") == 0.0)
          return NOT_A_CELL;
        else
          return 1.0;
      }
    };
  }
}
