/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <TH1F.h>

namespace Belle2 {


  /**
   * Class to store T0 information
   */
  class TOPTimeZero : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPTimeZero()
    {}

    /**
     * Usefull constructor
     * @param moduleID slot number, photon hits from this slot used to determine t0
     * @param t0 value of t0
     * @param err estimated error on t0
     * @param numPhotons number of photon hits used to find minimum
     */
    TOPTimeZero(int moduleID, double t0, double err, int numPhotons):
      m_moduleID(moduleID), m_t0(t0), m_err(err), m_numPhotons(numPhotons)
    {}

    /**
     * Sets histograms
     * @param chi2 chi^2 versus t0 used to find minimum
     * @param pdf PDF projected to time axis
     * @param time distribution of hits
     */
    void setHistograms(const TH1F& chi2, const TH1F& pdf, const TH1F& hits)
    {
      m_chi2 = chi2;
      m_pdf = pdf;
      m_hits = hits;
    }

    /**
     * Returns slot number used to determine t0
     * @return slot number (1-based)
     */
    unsigned getModuleID() const {return m_moduleID;}

    /**
     * Returns time zero
     * @return time zero [ns]
     */
    double getTime() const {return m_t0;}

    /**
     * Returns time zero uncertainty
     * @return uncertainty [ns]
     */
    double getError() const {return m_err;}

    /**
     * Returns number of photons used to find minimum
     * @return number of photons
     */
    int getNumPhotons() const {return m_numPhotons;}

    /**
     * Returns histogram of chi^2 versus t0 that was used to find minimum
     * @return 1D histogram
     */
    const TH1F& getChi2() const {return m_chi2;}

    /**
     * Returns histogram of PDF projected to time axis
     * @return 1D histogram
     */
    const TH1F& getPDF() const {return m_pdf;}

    /**
     * Returns histogram of time distribution of hits
     * @return 1D histogram
     */
    const TH1F& getHits() const {return m_hits;}

  private:

    int m_moduleID = 0; /**< slot number (1-based) */
    float m_t0 = 0;    /**< time zero in [ns] */
    float m_err = 0;   /**< error on time zero [ns] */
    int m_numPhotons = 0; /**< number of photons */
    TH1F m_chi2;  /**< chi^2 versus t0 used to find minimum */
    TH1F m_pdf;  /**< PDF projected to time */
    TH1F m_hits;  /**< time distribution of hits */

    ClassDef(TOPTimeZero, 2); /**< ClassDef */

  };

}
