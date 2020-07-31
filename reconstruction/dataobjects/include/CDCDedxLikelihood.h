/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <cmath>

namespace Belle2 {
  /** Container for likelihoods obtained by the CDC dE/dx PID (CDCDedxPIDModule).
   */
  class CDCDedxLikelihood : public RelationsObject {
  public:
    /** default constructor */
    CDCDedxLikelihood(): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_cdcLogl[i] = 0.0;
    }

    /** actually const double (&logl)[Const::ChargedStable::c_SetSize], but CINT complains. */
    explicit CDCDedxLikelihood(const double* cdcLogl): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_cdcLogl[i] = cdcLogl[i];
    }

    /** returns unnormalised log-likelihood value for a particle hypothesis using CDC information.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * @param type  The desired particle hypothesis.
     */
    double getLogL(const Const::ChargedStable& type) const { return m_cdcLogl[type.getIndex()]; }

    /** returns exp(getCDCLikelihood(type)) with sufficient precision. */
    double getCDCLikelihood(const Const::ChargedStable& type) const { return exp((double)m_cdcLogl[type.getIndex()]); }

  private:
    double m_cdcLogl[Const::ChargedStable::c_SetSize]; /**< CDC log likelihood for each particle, not including momentum prior */

    ClassDef(CDCDedxLikelihood, 1); /**< Container for likelihoods obtained by the CDC dE/dx PID (CDCDedxPIDModule). */
  };
}
