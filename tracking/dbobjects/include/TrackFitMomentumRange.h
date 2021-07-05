/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <map>

namespace Belle2 {

  /** The payload containing the momentum threshold to disable the particle hypothesis in the
      track fit. If the track momentum (from the seed) is greater than the hypothesis threshold,
      the particle hypothesis is disabled; if the momentum is lower, the particle
      hypothesis is used. Pion (default hypothesis) threshold has to be always infinite (large, always
      enabled). The values are expressed in GeV/c. */
  class TrackFitMomentumRange: public TObject {

  public:

    /** Default constructor */
    TrackFitMomentumRange() {}
    /** Destructor */
    ~TrackFitMomentumRange() {}

    /** Set the momentum threshold for the pdg particle hypothesis (infinite means always enabled)
     * @param pdg Particle hypothesis pdg code
     * @param momentum Momentum cut un GeV/c
     */
    void setMomentumRange(const int pdg, const float momentum)
    {
      m_pThreshold.insert(std::pair<int, float>(pdg, momentum));
    }

    /** Get the momentum threshold for the pdg particle hypothesis (100. means always enabled)
     * @param pdg Particle hypothesis pdg code
     */
    float getMomentumRange(int pdg) const
    {
      std::map<int, float>::const_iterator it = m_pThreshold.find(pdg);
      if (it != m_pThreshold.end()) {
        return it->second;
      } else {
        B2FATAL("Momentum threshold for pdg " << pdg << " not found");
      }
    }

  private:
    /** The p threshold that is used to determine if you want to fit the particle hypothesis,
    for each particle hypotheis (pdg number) */
    std::map<int, float> m_pThreshold;

    ClassDef(TrackFitMomentumRange, 1);  /**< ClassDef, necessary for ROOT */
  };
}
