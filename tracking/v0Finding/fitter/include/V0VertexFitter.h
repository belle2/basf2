/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

namespace genfit {
  class GFRaveVertex;
  class Track;
}

namespace Belle2 {

  /** Interface of the vertex fitters usable by the V0Fitter.
   *
   * Each implementation wraps one vertex fitting tool and hides it behind the fit() function.
   * The instances are created by the V0VertexFitterFactory, so that the tool to be used can be
   * selected at run time with a module parameter of the V0Finder.
   */
  class V0VertexFitter {

  public:

    /// Destructor.
    virtual ~V0VertexFitter() = default;

    /** Fit the V0 vertex.
     * @param trackPlus genfit Track of the positively-charged daughter
     * @param trackMinus genfit Track of the negatively-charged daughter
     * @param pdgTrackPlus PDG code of the hypothesis of the positively-charged daughter
     * @param pdgTrackMinus PDG code of the hypothesis of the negatively-charged daughter
     * @param vertex result of the fit is returned via reference
     * @return true (false) if the vertex fit has done well (failed)
     */
    virtual bool fit(genfit::Track& trackPlus, genfit::Track& trackMinus,
                     const int pdgTrackPlus, const int pdgTrackMinus,
                     genfit::GFRaveVertex& vertex) = 0;

    /// Get the name this fitter is registered with in the V0VertexFitterFactory.
    virtual std::string getName() const = 0;
  };

}
