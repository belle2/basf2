/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefano Spataro                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <map>

namespace Belle2 {

  /** The payload containing the momentum threshold to enable the particle hypothesis in  the
      track fit. If the track momentum (from the seed) is greater than the theshold, the
      particle hypothesis is enabled; if the momentum is lower, the lightest enabled particle
      hypothesis is used. Pion (default hypothesis) threshold has to be always -1 (always
      enabled). The values are expressed in GeV/c. */
  class TrackFitMomentumRange: public TObject {

  public:

    /** Default constructor */
    TrackFitMomentumRange() {}
    /** Destructor */
    ~TrackFitMomentumRange() {}

    /** Set the momentum threshold for the pdg particle hypothesis (-1 means always enabled) */
    void setMomentumRange(const int pdg, const float momentum)
    {
      m_pThreshold.insert(std::pair<int, float>(pdg, momentum));
    }

    /** Get the momentum threshold for the pdg particle hypothesis (-1 means always enabled) */
    float getMomentumRange(int pdg) const
    {
      std::map<int, float>::const_iterator it = m_pThreshold.find(pdg);
      if (it != m_pThreshold.end()) {
        return it->second;
      } else {
        B2FATAL("Momentum threshold for pdf " << pdg << " not found");
      }
    }

  private:
    /** The p threshold that is used to determine if you want to fit the particle hypothesis,
    for each particle hypotheis (pdg number) */
    std::map<int, float> m_pThreshold;

    ClassDef(TrackFitMomentumRange, 1);  /**< ClassDef, necessary for ROOT */
  };
}
