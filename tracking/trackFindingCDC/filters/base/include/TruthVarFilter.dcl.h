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

#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// MC Filter Type using a VarSet and the truth variable in it.
    template<class ATruthVarSet>
    class TruthVarFilter : public ChoosableFromVarSetFilter<ATruthVarSet> {

    private:
      /// Type of the base class
      using Super = ChoosableFromVarSetFilter<ATruthVarSet>;

    public:
      /// Type of the handled object.
      using Object = typename Super::Object;

    public:
      /// Constructor.
      TruthVarFilter();

      /// Default destructor
      ~TruthVarFilter();

    public:
      /// Reject an item if the truth variable is 0, else accept it.
      Weight operator()(const Object& object) override;
    };
  }
}
