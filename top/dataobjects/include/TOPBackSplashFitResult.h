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
     * Basic constructor with minimal information
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
     * Full constructor for development studies
     * @param  time fitted timing from TOP
     * @param  timeErr error fitted timing from TOP
     * @param  deltaT time difference between direct and reflected peaks
     * @param  deltaTerr error on the time difference between direct and reflected peaks
     * @param  frac fractional yield of the first peak
     * @param  fracTErr error on the fractional yield of the first peak
     * @param  signalPhotons fitted number of photons
     * @param  signalPhotonsErr error on  the fitted number of photons
     */
    TOPBackSplashFitResult(double time,
                           double timeErr,
                           double deltaT,
                           double deltaTerr,
                           double frac,
                           double fracErr,
                           double signalPhotons,
                           double signalPhotonsErr,
                           double chisqdof,
                           int nPhotons)
      : m_time(time),
        m_timeErr(timeErr),
        m_deltaT(deltaT),
        m_deltaTerr(deltaTerr),
        m_frac(frac),
        m_fracErr(fracErr),
        m_signalPhotons(signalPhotons),
        m_signalPhotonsErr(signalPhotonsErr),
        m_chisqdof(chisqdof),
        m_nPhotons(nPhotons)
    {}

    /**
     * Return time of extracted fit time
     * @return extracted fit time to TOP digits in slot nearest to cluster [ns]
     */
    double getTime() const { return m_time; }

    /**
     * Return time of extracted fit time error
     * @return error on the extracted fit time to TOP digits in slot nearest to cluster [ns]
     */
    double getTimeErr() const { return m_timeErr; }

    /**
     * Return time difference between the direct and reflected light peaks
     * @return time difference between the two peaks of the fit PDF [ns]
     */
    double getDelta() const { return m_deltaT; }

    /**
     * Return error on time difference between the direct and reflected light peaks
     * @return error on the time difference between the two peaks of the fit PDF [ns]
     */
    double getDeltaErr() const { return m_deltaTerr; }

    /**
     * Return the relative fraction of the direct peak with respect to the total
     * @return fraction of the first peak with respect to the total signal PDF
     */
    double getFrac() const { return m_frac; }

    /**
     * Return the error on the relative fraction of the direct peak with respect to the total
     * @return error on the fraction of the first peak with respect to the total signal PDF
     */
    double getFracErr() const { return m_fracErr; }

    /**
     * Return the normalization of the signal PDF
     * @return normalization of the signal PDF
     */
    double getSignalPhotons() const { return m_signalPhotons; }

    /**
     * Return error on the normalization of the signal PDF
     * @return error on the normalization of the signal PDF
     */
    double getSignalPhotonsErr() const { return m_signalPhotonsErr; }

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
    float m_timeErr{0.0};      /**< error fitted timing from TOP */
    float m_deltaT{0.0};      /**< time difference between direct and reflected peaks */
    float m_deltaTerr{0.0};      /**< error on the time difference between direct and reflected peaks  */
    float m_frac{0.0};      /**< fractional yield of the first peak */
    float m_fracErr{0.0};      /**< error on the fractional yield of the first peak */
    float m_signalPhotons{0.0};      /**< fitted number of photons */
    float m_signalPhotonsErr{0.0};      /**< error on  the fitted number of photons */

    float m_chisqdof{0.0};  /**< chi2 per d.o.f. of timing fit */
    int m_nPhotons{0};     /**< no. of recorded Cherenkov photons used in fit */

    ClassDef(TOPBackSplashFitResult, 1); /**< ClassDef  */
  };

} // end namespace Belle2
