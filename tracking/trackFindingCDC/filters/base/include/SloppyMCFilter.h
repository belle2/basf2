/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <TRandom.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Sloppy MC Filter Type using a VarSet and the truth variable in it. It will true for correct ones and
    /// sometimes also for false ones.
    template<class ATruthVarSet>
    class SloppyMCFilter : public MCFilter<ATruthVarSet> {
    private:
      using Super = MCFilter<ATruthVarSet>;

    public:
      /// Reject an item if the truth variable is 0 in some fractions of the events, else accept it.
      Weight operator()(const typename Super::Object& object) override
      {
        Weight truth = Super::operator()(object);

        if (std::isnan(truth)) {
          const unsigned int randomNumber = gRandom->Integer(m_scaleFactor);
          if (randomNumber == 0) {
            return 1.0;
          } else {
            return NAN;
          }
        } else {
          return 1.0;
        }
      }

    private:
      unsigned int m_scaleFactor = 100;
    };
  }
}
