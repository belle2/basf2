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

namespace Belle2 {
  /** Container for likelihoods obtained by the dE/dx PID (DedxPIDModule).
   */
  class DedxLikelihood : public RelationsObject {
  public:
    /** default constructor */
    DedxLikelihood(): RelationsObject() {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_cdcLogl[i] = 0.0;
        m_svdLogl[i] = 0.0;
      }
    }

    /** actually const float (&logl)[Const::ChargedStable::c_SetSize], but CINT complains. */
    DedxLikelihood(const float* cdcLogl, const float* svdLogl): RelationsObject() {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_cdcLogl[i] = cdcLogl[i];
        m_svdLogl[i] = svdLogl[i];
      }
    }

    /** returns unnormalised log-likelihood value for a particle hypothesis using CDC information.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * @param type  The desired particle hypothesis.
     */
    float getCDCLogLikelihood(const Const::ChargedStable& type) const { return m_cdcLogl[type.getIndex()]; }

    /** returns exp(getCDCLikelihood(type)) with sufficient precision. */
    double getCDCLikelihood(const Const::ChargedStable& type) const { return exp((double)m_cdcLogl[type.getIndex()]); }

    /** returns unnormalised log-likelihood value for a particle hypothesis using SVD (and/or PXD) information.
     *
     * @sa getCDCLogLikelihood()
     */
    float getSVDLogLikelihood(const Const::ChargedStable& type) const { return m_svdLogl[type.getIndex()]; }

    /** returns exp(getSVDLikelihood(type)) with sufficient precision. */
    double getSVDLikelihood(const Const::ChargedStable& type) const { return exp((double)m_svdLogl[type.getIndex()]); }

  private:
    float m_cdcLogl[Const::ChargedStable::c_SetSize]; /**< CDC log likelihood for each particle, not including momentum prior */
    float m_svdLogl[Const::ChargedStable::c_SetSize]; /**< SVD (and/or PXD) log likelihood for each particle, not including momentum prior */

    ClassDef(DedxLikelihood, 4); /**< Container for likelihoods obtained by the dE/dx PID (DedxPIDModule). */
  };
}
#endif
