/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
