/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/v0Finding/fitter/V0VertexFitter.h>

namespace Belle2 {

  /** V0 vertex fitter using Rave, through the genfit interface. */
  class RaveV0VertexFitter : public V0VertexFitter {

  public:

    /// Fit the V0 vertex. The PDG codes of the daughters are not used by Rave.
    bool fit(genfit::Track& trackPlus, genfit::Track& trackMinus,
             const int pdgTrackPlus, const int pdgTrackMinus,
             genfit::GFRaveVertex& vertex) override;

    /// Get the name this fitter is registered with in the V0VertexFitterFactory.
    std::string getName() const override {return "Rave";}
  };

}
