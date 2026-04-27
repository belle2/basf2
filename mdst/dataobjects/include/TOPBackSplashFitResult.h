/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store the quantities determined in the `TOPBackSplashTiming` module
   * and relate to corresponding ECL clusters, namely the fitted peak timing of
   * TOP digits closest neutral hadron ECL clusters, the number of recorded
   * Cherenkov photons used in the fit and the fit quality
   */

  class TOPBackSplashFitResult : public RelationsObject {

  public:
    TOPBackSplashFitResult() = default;
    /**
     * Useful constructor
     * @param time      extracted TOP timing from fit
     * @param chisqdof  chi2 per degrees of freedom associated with fit
     * @param nPhotons  no. of Cherenkov photons recorded by TOP and used in fit
     */
    TOPBackSplashFitResult(double time, double chisqdof, int nPhotons)
      : m_time(time),
        m_chisqdof(chisqdof),
        m_nPhotons(nPhotons)
    {}

    /**
     * Return time of extracted fit time
     * @return extracted fit time to TOP digits in slot nearest to cluster [ns]
     */
    double getTime() const { return m_time; }

    /**
     * Return chi/d.o.f
     * @return chi2 per degree of freedom resulting from fit to TOP digits
     */
    double getChisqdof() const { return m_chisqdof; }

    /**
     * Return no. of Cherenkov photons in fit
     * @return no. of recorded photons used to extract TOP timing nearest to clusters
     */
    int getNphotons() const { return m_nPhotons; }

  private:
    float m_time{0.0};      /**< fitted timing from TOP */
    float m_chisqdof{0.0};  /**< chi2 per d.o.f. of timing fit */
    int m_nPhotons{0};     /**< no. of recorded Cherenkov photons used in fit */

    ClassDef(TOPBackSplashFitResult, 1); /**< ClassDef  */
  };

} // end namespace Belle2
