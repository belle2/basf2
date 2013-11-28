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

namespace Belle2 {

  class Muid;

  /*! Class to store Muid log likelihoods (output of MuidModule)
   * relation from Tracks
   * filled in tracking/modules/muid/src/MuidModule.cc
   */
  class MuidLikelihood : public RelationsObject {

  public:

    /*! default constructor */
    MuidLikelihood() :
      RelationsObject(),
      m_logL_mu(0),
      m_logL_pi(0),
      m_logL_K(0),
      m_logL_miss(0),
      m_logL_junk(0) {
    }

    /*! full constructor
     * @param muid container of muid likelihoods
     */
    MuidLikelihood(const Muid* muid);

    /*! Get muon log likelihood
     * @return muon log likelihood
     */
    double getLogL_mu() const {return m_logL_mu;}

    /*! Get pion log likelihood
     * @return pion log likelihood
     */
    double getLogL_pi() const {return m_logL_pi;}

    /*! Get kaon log likelihood
     * @return kaon log likelihood
     */
    double getLogL_K() const {return m_logL_K;}

    /*! Get missed-KLM log likelihood
     * @return missed-KLM log likelihood
     */
    double getLogL_miss() const {return m_logL_miss;}

    /*! Get unclassifiable-track log likelihood
     * @return unclassifiable-track log likelihood
     */
    double getLogL_junk() const {return m_logL_junk;}

  private:
    double m_logL_mu;    /**< log likelihood for muon hypothesis */
    double m_logL_pi;    /**< log likelihood for pion hypothesis */
    double m_logL_K;     /**< log likelihood for kaon hypothesis */
    double m_logL_miss;     /**< log likelihood for missed-KLM hypothesis */
    double m_logL_junk;     /**< log likelihood for unclassifiable-track hypothesis */

    ClassDef(MuidLikelihood, 1); /**< ClassDef */

  };
}
