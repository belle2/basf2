/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXLIKELIHOOD_H
#define DEDXLIKELIHOOD_H

//only used for a few constants, no linking necessary
#include <reconstruction/modules/dedxPID/DedxConstants.h>

#include <framework/datastore/RelationsObject.h>

#include <string>
#include <cmath>

class TH1F;

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /** Container for likelihoods obtained by the dE/dx PID (DedxPIDModule).
   */
  class DedxLikelihood : public RelationsObject {
  public:
    /** default constructor */
    DedxLikelihood():
      RelationsObject(),
      m_p(0.0) {
      //for all particles
      for (int i = 0; i < Dedx::c_num_particles; i++) {
        m_logl[i] = 0.0;
      }
    }

    /** actually const float (&logl)[Dedx::c_num_particles], but CINT complains. */
    DedxLikelihood(const float* logl, float p):
      RelationsObject(),
      m_p(p) {
      //for all particles
      for (int i = 0; i < Dedx::c_num_particles; i++) {
        m_logl[i] = logl[i];
      }
    }


    /** returns unnormalised log-likelihood value for a particle hypothesis.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * Instead of comparing hypotheses in pairs, one can also use the probability
     * for a certain hypothesis provided by getProbability(), which takes into
     * account all hypotheses and their momentum distribution.
     *
     * @param type  The desired particle hypothesis.
     */

    float getLogLikelihood(Dedx::Particle type) const { return m_logl[type]; }

    /** returns exp(getLogLikelihood(type)) with sufficient precision. */
    double getLikelihood(Dedx::Particle type) const { return exp((double)m_logl[type]); }

    /** corresponding setter for m_logl. */
    void setLogLikelihood(Dedx::Particle type, float logl) { m_logl[type] = logl; }

    /** the associated track momentum at the origin*/
    float getMomentum() const { return m_p; }

    /** corresponding setter for m_p */
    void setMomentum(float p) { m_p = p; }

  private:
    float m_logl[Dedx::c_num_particles]; /**< log likelihood for each particle, not including momentum prior */
    float m_p; /**< track momentum used for PDF lookups */

    ClassDef(DedxLikelihood, 2); /**< Build ROOT dictionary */
  };
  /*! @} */
}
#endif
