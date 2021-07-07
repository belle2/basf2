/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <cmath>

namespace Belle2 {

  /** Container for likelihoods with ECL PID (ECLChargedPIDModule) */
  class ECLPidLikelihood : public RelationsObject {
  public:
    /** default constructor */
    ECLPidLikelihood(): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = 0.0;
      }
      setVariables(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    /** actually const float (&logl)[Const::ChargedStable::c_SetSize], but CINT complains. */
    explicit ECLPidLikelihood(const float* logl, float energy = 0, float eop = 0, float e9e25 = 0, float lat = 0, float dist = 0,
                              float trkDepth = 0, float shDepth = 0, int ncrystals = 0, int nclusters = 0): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_logl[i] = logl[i];
      }
      setVariables(energy, eop, e9e25, lat, dist, trkDepth, shDepth, ncrystals, nclusters);
    }

    /** returns log-likelihood value for a particle hypothesis.
     *  The correct particle hypothesis will be considered depending upon the reconstructed track charge.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * @param type  The desired particle hypothesis.
     */
    float getLogLikelihood(const Const::ChargedStable& type) const
    {
      return m_logl[type.getIndex()];
    }

    /** returns exp(getLogLikelihood(type)) with sufficient precision. */
    double getLikelihood(const Const::ChargedStable& type) const
    {
      return exp((double)m_logl[type.getIndex()]);
    }

    /** corresponding setter for m_logl. */
    void setLogLikelihood(const Const::ChargedStable& type, float logl)
    {
      m_logl[type.getIndex()] = logl;
    }

    /** Set cluster features upon which the likelihood might depend. */
    void setVariables(float energy, float eop, float e9e25, float lat, float dist, float trkDepth, float shDepth, int ncrystals,
                      int nclusters)
    {
      m_energy = energy; m_eop = eop; m_e9e25 = e9e25; m_nCrystals = ncrystals; m_nClusters = nclusters;
      m_lat = lat; m_dist = dist; m_trkDepth = trkDepth; m_shDepth = shDepth;
    }

    double energy() const { return m_energy; } /**< Cluster energy*/
    double eop() const { return m_eop; } /**< E/p ratio for cluster */
    double e9e25() const { return m_e9e25; } /**< Ratio of energies of the (central) 3x3 crystal matrix and outermost 5x5 matrix */
    float lat() const { return m_lat; } /**< Cluster LAT */
    float dist() const { return m_dist; } /**< Cluster-Shower distance */
    float trkDepth() const { return m_trkDepth; } /**< Track Depth*/
    float shDepth() const { return m_shDepth; } /**< Cluster Depth */
    int nCrystals() const { return m_nCrystals; } /**< Number of crystals per candidate */
    int nClusters() const { return m_nClusters; } /**< Number of clusters per candidate */

  private:

    float m_logl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    float m_energy;  /**< Cluster Energy */
    float m_eop; /**< E/p ratio for cluster */
    float m_e9e25; /**< Ratio of energy in 3x3 matrix over energy in 5x5 matrix (should now be called E9oE21) for cluster */
    int m_nCrystals; /**< Number of crystals per candidate */
    int m_nClusters;/**< Number of clusters per candidate */
    float m_lat; /**< Cluster LAT */
    float m_dist; /**< Cluster-Shower distance */
    float m_trkDepth;  /**< Track Depth */
    float m_shDepth;  /**< Cluster Depth */

    // 3: Added DeltaL track match variable (GDN, increment by TF)
    ClassDef(ECLPidLikelihood, 3); /**< Build ROOT dictionary */
  };
}

