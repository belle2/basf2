/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDDEDXLIKELIHOOD_H
#define VXDDEDXLIKELIHOOD_H

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <cmath>

namespace Belle2 {
  /** Container for likelihoods obtained by the VXD dE/dx PID (VXDDedxPIDModule).
   */
  class VXDDedxLikelihood : public RelationsObject {
  public:
    /** default constructor */
    VXDDedxLikelihood(): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_vxdLogl[i] = 0.0;
    }

    VXDDedxLikelihood(const double* vxdLogl): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_vxdLogl[i] = vxdLogl[i];
    }

    /** returns unnormalised log-likelihood value for a particle hypothesis using SVD (and/or PXD) information.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * @param type  The desired particle hypothesis.
     */
    double getLogL(const Const::ChargedStable& type) const { return m_vxdLogl[type.getIndex()]; }

    /** returns exp(getVXDLikelihood(type)) with sufficient precision. */
    double getVXDLikelihood(const Const::ChargedStable& type) const { return exp((double)m_vxdLogl[type.getIndex()]); }

  private:
    double m_vxdLogl[Const::ChargedStable::c_SetSize]; /**< SVD (and/or PXD) log likelihood for each particle, not including momentum prior */

    ClassDef(VXDDedxLikelihood, 1); /**< Container for likelihoods obtained by the VXD dE/dx PID (VXDDedxPIDModule). */
  };
}
#endif
