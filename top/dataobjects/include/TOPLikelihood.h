/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Class to store TOP log likelihoods (output of TOPReconstructor).
   * relation from Tracks
   * filled in top/modules/TOPReconstruction/src/TOPReconstructorModule.cc
   */

  class TOPLikelihood : public RelationsObject {

  public:

    /**
     * default constructor
     */
    TOPLikelihood()
    {}

    /**
     * Sets reconstruction flag
     * @param flag reconstruction flag: 1 = OK, 0 = out of acceptance, -1 = error in reconstruction
     */
    void setFlag(int flag) {m_flag = flag;}

    /**
     * Sets other data members for a given particle hypothesis
     * @param part particle hypothesis
     * @param numPhot number of measured photons
     * @param logL log likelihood
     * @param estPhot expected number of photons (including background)
     * @param estBkg expected number of background hits
     */
    void set(const Const::ChargedStable& part, int numPhot, double logL, double estPhot, double estBkg)
    {
      auto i = part.getIndex();
      m_numPhot = numPhot;
      m_logL[i] = logL;
      m_estPhot[i] = estPhot;
      m_estBkg = estBkg;
    }

    /**
     * Return reconstruction flag
     * @return reconstruction flag: 1 = OK, 0 = out of acceptance, -1 = error in reconstruction
     */
    int getFlag() const {return m_flag;}

    /**
     * Return number of detected photons
     * @return number of detected photons
     */
    int getNphot() const {return m_numPhot;}

    /**
     * Return log likelihood for a given particle
     * @param part charged stable particle
     * @return log likelihood
     */
    float getLogL(const Const::ChargedStable& part) const
    {
      return m_logL[part.getIndex()];
    }

    /**
     * Return estimated number of photons for a given particle
     * @param part charged stable particle
     * @return estimated number of photons (signal + background)
     */
    float getEstPhot(const Const::ChargedStable& part) const
    {
      return m_estPhot[part.getIndex()];
    }

    /**
     * Return estimated number of background photons
     * @return estimated background
     */
    float getEstBkg() const {return m_estBkg;}

    /**
     * Return electron log likelihood
     * @return electron log likelihood
     */
    double getLogL_e() const {return m_logL[Const::electron.getIndex()];}

    /**
     * Return muon log likelihood
     * @return muon log likelihood
     */
    double getLogL_mu() const {return m_logL[Const::muon.getIndex()];}

    /**
     * Return pion log likelihood
     * @return pion log likelihood
     */
    double getLogL_pi() const {return m_logL[Const::pion.getIndex()];}

    /**
     * Return kaon log likelihood
     * @return kaon log likelihood
     */
    double getLogL_K() const {return m_logL[Const::kaon.getIndex()];}

    /**
     * Return proton log likelihood
     * @return proton log likelihood
     */
    double getLogL_p() const {return m_logL[Const::proton.getIndex()];}

    /**
     * Return number of expected photons for electron
     * @return number of expected photons for electron
     */
    double getNphot_e() const {return m_estPhot[Const::electron.getIndex()];}

    /**
     * Return number of expected photons for muon
     * @return number of expected photons for muon
     */
    double getNphot_mu() const {return m_estPhot[Const::muon.getIndex()];}

    /**
     * Return number of expected photons for pion
     * @return number of expected photons for pion
     */
    double getNphot_pi() const {return m_estPhot[Const::pion.getIndex()];}

    /**
     * Return number of expected photons for kaon
     * @return number of expected photons for kaon
     */
    double getNphot_K() const {return m_estPhot[Const::kaon.getIndex()];}

    /**
     * Return number of expected photons for proton
     * @return number of expected photons for proton
     */
    double getNphot_p() const {return m_estPhot[Const::proton.getIndex()];}

  private:
    int m_flag = 0;     /**< reconstruction flag */
    int m_numPhot = 0;  /**< number of photons */
    float m_logL[Const::ChargedStable::c_SetSize] = {0};    /**< log likelihoods */
    float m_estPhot[Const::ChargedStable::c_SetSize] = {0}; /**< estimated number of photons */
    float m_estBkg = 0; /**< estimated background */

    ClassDef(TOPLikelihood, 2); /**< ClassDef */

  };


} // end namespace Belle2

