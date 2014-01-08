/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ECLPIDLIKELIHOOD
#define ECLPIDLIKELIHOOD

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <string>
#include <cmath>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /** Container for likelihoods with ECL PID (ECLElectronIdModule)
   */
  class ECLPidLikelihood : public RelationsObject {
  public:
    /** default constructor */
    ECLPidLikelihood(): RelationsObject() {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = 0.0;
      }
    }

    /** actually const float (&logl)[Const::ChargedStable::c_SetSize], but CINT complains. */
    ECLPidLikelihood(const float* logl): RelationsObject() {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = logl[i];
      }
    }

    /** returns log-likelihood value for a particle hypothesis.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * @param type  The desired particle hypothesis.
     */
    float getLogLikelihood(const Const::ChargedStable& type) const { return m_logl[type.getIndex()]; }

    /** returns exp(getLogLikelihood(type)) with sufficient precision. */
    double getLikelihood(const Const::ChargedStable& type) const { return exp((double)m_logl[type.getIndex()]); }

    /** corresponding setter for m_logl. */
    void setLogLikelihood(const Const::ChargedStable& type, float logl) { m_logl[type.getIndex()] = logl; }

  private:
    float m_logl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    ClassDef(ECLPidLikelihood, 1); /**< Build ROOT dictionary */
  };
  /*! @} */
}
#endif
