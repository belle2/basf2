/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    ARICHLikelihood(): m_flag(0) {
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) m_logL[i] = 0;
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) m_expPhot[i] = 0;
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) m_detPhot[i] = 0;
    }

    /*! full constructor
     * @param flag:     reconstruction flag
     * @param logL log likelihoods in the order of Const::ChargedStableSet
     * @param expPhot expected number of photons in the order of Const::ChargedStableSet
     * @param detPhot detected number of photons in the order of Const::ChargedStableSet
     */

    ARICHLikelihood(int flag,
                    const double* logL,
                    const int* detPhot,
                    const double* expPhot
                   ) {
      m_flag = flag;
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_logL[i] = logL[i];
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_expPhot[i] = expPhot[i];
      for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_detPhot[i] = detPhot[i];
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
    float getLogL(const Const::ChargedStable& part) const {
      return m_logL[part.getIndex()];
    }

    /*!
     * Return number of detected photons for a given particle
     * @param part charged stable particle
     * @return detected photon number
     */
    float getDetPhot(const Const::ChargedStable& part) const {
      return m_detPhot[part.getIndex()];
    }

    /*!
     * Return number of expected photons for a given particle
     * @param part charged stable particle
     * @return detected photon number
     */
    float getExpPhot(const Const::ChargedStable& part) const {
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

    /*! Get number of detected photons in electron expected ring
     * @return number of detected photons  in electron expected ring
     */
    int getNphotD_e() const {return m_detPhot[Const::electron.getIndex()];}

    /*! Get number of detected photons in muon expected ring
     * @return number of detected photons in muon expected ring
     */
    int getNphotD_mu() const {return m_detPhot[Const::muon.getIndex()];}

    /*! Get number of detected photons in pion expected ring
     * @return number of detected photons in pion expected ring
     */
    int getNphotD_pi() const {return m_detPhot[Const::pion.getIndex()];}

    /*! Get number of detected photons in kaon expected ring
     * @return number of detected photons in kaon expected ring
     */
    int getNphotD_K() const {return m_detPhot[Const::kaon.getIndex()];}

    /*! Get number of detected photons in proton expected ring
     * @return number of detected photons in proton expected ring
     */
    int getNphotD_p() const {return m_detPhot[Const::proton.getIndex()];}

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
    int m_detPhot[Const::ChargedStable::c_SetSize]; /**< detected number of photons */

    ClassDef(ARICHLikelihood, 2); /**< the class title */

  };

} // end namespace Belle2

#endif

