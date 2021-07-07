/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHLIKELIHOOD_H
#define ARICHLIKELIHOOD_H

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {



  /** This is a class to store ARICH likelihoods in the datastore.
   *
   *  It is the output of reconstruction module (ARICHReconstructor).
   */

  class ARICHLikelihood : public RelationsObject {

  public:

    /*! default constructor */

    ARICHLikelihood(): m_flag(0)
    {
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) m_logL[i] = 0;
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) m_expPhot[i] = 0;
      m_detPhot = 0;
    }

    /*!
     * Set values
     * @param flag:     reconstruction flag
     * @param logL      log likelihoods in the order of Const::ChargedStableSet
     * @param expPhot   expected number of photons in the order of Const::ChargedStableSet
     * @param detPhot   detected number of photons in the order of Const::ChargedStableSet
     */
    void setValues(int flag,
                   const double* logL,
                   int detPhot,
                   const double* expPhot
                  )
    {
      m_flag = flag;
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_logL[i] = logL[i];
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_expPhot[i] = expPhot[i];

      m_detPhot = detPhot;
    }

    /*! Get reconstruction flag
     * @return reconstruction flag: 1=OK, 0=out of acceptance, -1=inside gap btw. bars
     */
    int getFlag() const {return m_flag;}

    /*!
     * Return log likelihood for a given particle
     * @param part charged stable particle
     * @return log likelihood
     */
    float getLogL(const Const::ChargedStable& part) const
    {
      return m_logL[part.getIndex()];
    }

    /*!
     * Return number of detected photons for a given particle
     * @return detected photon number
     */
    float getDetPhot() const
    {
      return m_detPhot;
    }

    /*!
     * Return number of expected photons for a given particle
     * @param part charged stable particle
     * @return detected photon number
     */
    float getExpPhot(const Const::ChargedStable& part) const
    {
      return m_expPhot[part.getIndex()];
    }


    /*! Get electron log likelihood
     * @return electron log likelihood
     */
    double getLogL_e() const {return m_logL[Const::electron.getIndex()];}

    /*! Get muon log likelihood
     * @return muon log likelihood
     */
    double getLogL_mu() const {return m_logL[Const::muon.getIndex()];}

    /*! Get pion log likelihood
     * @return pion log likelihood
     */
    double getLogL_pi() const {return m_logL[Const::pion.getIndex()];}

    /*! Get kaon log likelihood
     * @return kaon log likelihood
     */
    double getLogL_K() const {return m_logL[Const::kaon.getIndex()];}

    /*! Get proton log likelihood
     * @return proton log likelihood
     */
    double getLogL_p() const {return m_logL[Const::proton.getIndex()];}


    /*! Get number of expected photons for electron
     * @return number of expected photons for electron
     */
    double getNphot_e() const {return m_expPhot[Const::electron.getIndex()];}

    /*! Get number of expected photons for muon
     * @return number of expected photons for muon
     */
    double getNphot_mu() const {return m_expPhot[Const::muon.getIndex()];}

    /*! Get number of expected photons for pion
     * @return number of expected photons for pion
     */
    double getNphot_pi() const {return m_expPhot[Const::pion.getIndex()];}

    /*! Get number of expected photons for kaon
     * @return number of expected photons for kaon
     */
    double getNphot_K() const {return m_expPhot[Const::kaon.getIndex()];}

    /*! Get number of expected photons for proton
     * @return number of expected photons for proton
     */
    double getNphot_p() const {return m_expPhot[Const::proton.getIndex()];}

  private:
    int m_flag;          /**< reconstruction flag */
    float m_logL[Const::ChargedStable::c_SetSize]; /**< log likelihoods */
    float m_expPhot[Const::ChargedStable::c_SetSize]; /**< expected number of photons */
    int m_detPhot; /**< detected number of photons */

    ClassDef(ARICHLikelihood, 3); /**< the class title */

  };

} // end namespace Belle2

#endif

