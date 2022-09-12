/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      m_moduleID(moduleID), m_t0(t0), m_err(err), m_numPhotons(numPhotons), m_valid(true)
    {}

    /**
     * Sets histograms
     * @param chi2 chi^2 versus t0 used to find minimum
     * @param pdf PDF projected to time axis
     * @param hits time distribution of hits
     */
    void setHistograms(const TH1F& chi2, const TH1F& pdf, const TH1F& hits)
    {
      m_chi2 = chi2;
      m_pdf = pdf;
      m_hits = hits;
    }

    /**
     * Sets particle mass used in reconstruction
     * @param mass particle mass
     */
    void setAssumedMass(double mass) {m_assumedMass = mass;}

    /**
     * Sets time zero to invalid. Usefull when time zero not successfully determined.
     */
    void setInvalid() {m_valid = false;}

    /**
     * Sets chi2 value at minimum
     * @param minChi2 value at minimum
     */
    void setMinChi2(double minChi2) {m_minChi2 = minChi2;}

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
     * Returns chi2 value at minimum
     * @return chi2 at minimum
     */
    double getMinChi2() const {return m_minChi2;}

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

    /**
     * Returns particle mass used in time zero reconstruction
     * @return particle mass
     */
    double getAssumedMass() const {return m_assumedMass;}

    /**
     * Returns the status of time zero reconstruction
     * @return true if successfully determined
     */
    bool isValid() const {return m_valid;}

  private:

    int m_moduleID = 0; /**< slot number (1-based) */
    float m_t0 = 0;    /**< time zero in [ns] */
    float m_err = 0;   /**< error on time zero [ns] */
    int m_numPhotons = 0; /**< number of photons */
    TH1F m_chi2;  /**< chi^2 versus t0 used to find minimum */
    TH1F m_pdf;  /**< PDF projected to time */
    TH1F m_hits;  /**< time distribution of hits */
    float m_assumedMass = 0; /**< particle mass used in reconstruction */
    bool m_valid = false; /**< status: true if time zero is successfully determined */
    float m_minChi2 = 0; /**< chi2 value at minimum */

    ClassDef(TOPTimeZero, 4); /**< ClassDef */

  };

}
