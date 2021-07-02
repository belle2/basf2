/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/modules/CurlTagger/Selector.h>

#include <analysis/dataobjects/Particle.h>
#include <vector>

namespace Belle2 {
  namespace CurlTagger {

    /** Simple cut based selector for curl tracks taken from Belle note 1079 */
    class SelectorCut : public Selector {

    public:
      /** Constructor */
      explicit SelectorCut(bool belleFlag);

      /** Destructor */
      ~SelectorCut();

      /** Selector response that this pair of particles come from the same mc/actual particle */
      virtual float getResponse(Particle* iPart, Particle* jPart) override;

      /** returns vector of variables used by this selector. Not really useful here but included to be consistent with eventual MVA method. */
      virtual std::vector<float> getVariables(Particle* iPart, Particle* jPart) override;

    private:
      /** cut for momentum magnitude difference */
      double m_magDiffPCut;

      /** cut for angle between momenta of the two particles when they have the same charge */
      double m_sameChargePhiCut;

      /** cut for angle between momenta of the two particles when they have the opposite charge */
      double m_oppositeChargePhiCut;

    }; //class

  } // curlTagger Module namespace
}// Belle 2 namespace

