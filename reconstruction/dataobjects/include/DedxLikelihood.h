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

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <string>
#include <cmath>

class TH1F;

namespace Belle2 {
  /** Container for likelihoods obtained by the dE/dx PID (DedxPIDModule).
   */
  class DedxLikelihood : public RelationsObject {
  public:
    /** default constructor */
    DedxLikelihood(): RelationsObject() {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = 0.0;
      }
    }

    /** actually const float (&logl)[Const::ChargedStable::c_SetSize], but CINT complains. */
    DedxLikelihood(const float* logl): RelationsObject() {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = logl[i];
      }
    }

    /** returns unnormalised log-likelihood value for a particle hypothesis.
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

    ClassDef(DedxLikelihood, 3); /**< Container for likelihoods obtained by the dE/dx PID (DedxPIDModule). */
  };
}
#endif
