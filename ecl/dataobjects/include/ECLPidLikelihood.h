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

  /** Container for likelihoods with ECL PID (ECLElectronIdModule) */
  class ECLPidLikelihood : public RelationsObject {
  public:
    /** default constructor */
    ECLPidLikelihood(): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = 0.0;
      }
      setVariables(0, 0, 0, 0, 0);
    }

    /** actually const float (&logl)[Const::ChargedStable::c_SetSize], but CINT complains. */
    ECLPidLikelihood(const float* logl, float energy = 0, float eop = 0, float e9e25 = 0, int ncrystals = 0,
                     int nclusters = 0): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = logl[i];
      }
      setVariables(energy, eop, e9e25, ncrystals, nclusters);
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

    void setVariables(float energy, float eop, float e9e25, int ncrystals, int nclusters)
    {
      m_energy = energy; m_eop = eop; m_e9e25 = e9e25; m_nCrystals = ncrystals; m_nClusters = nclusters;
    }

    double energy() const { return m_energy; }
    double eop() const { return m_eop; }
    double e9e25() const { return m_e9e25; }
    int nCrystals() const { return m_nCrystals; }
    int nClusters() const { return m_nClusters; }
  private:
    float m_logl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    float m_energy;
    float m_eop;
    float m_e9e25;
    int m_nCrystals;
    int m_nClusters;

    ClassDef(ECLPidLikelihood, 2); /**< Build ROOT dictionary */
  };
}
#endif
