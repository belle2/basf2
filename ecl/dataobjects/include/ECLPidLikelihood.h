/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
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
      for (unsigned int i = 0; i < c_noOfHypotheses; i++) {
        m_logl[i] = 0.0;
      }
      setVariables(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    /** actually const float (&logl)[Const::ChargedStable::c_SetSize], but CINT complains. */
    ECLPidLikelihood(const float* logl, float energy = 0, float eop = 0, float e9e25 = 0, float lat = 0, float dist = 0,
                     float trkDepth = 0, float shDepth = 0, int ncrystals = 0, int nclusters = 0): RelationsObject()
    {
      //for all particles
      for (unsigned int i = 0; i < c_noOfHypotheses; i++) {
        m_logl[i] = logl[i];
      }
      setVariables(energy, eop, e9e25, lat, dist, trkDepth, shDepth, ncrystals, nclusters);
    }

    /** A helper to get the correct particle hypothesis index in an array depending on reco charge sign.
    NB: this assumes the array size is twice the size of Const::chargedStableSet, and '+' particles indexes
    come first in the array than '-' particles.
     */
    static int getChargeAwareIndex(const Const::ChargedStable& part, const short& recoCharge)
    {
      if (recoCharge / abs(recoCharge) > 0) return part.getIndex();
      else if (recoCharge / abs(recoCharge) < 0) return part.getIndex() + c_offset;
      return -1;
    }

    /** returns log-likelihood value for a particle hypothesis.
     *  The correct particle hypothesis will be considered depending upon the reconstructed track charge.
     *
     * This can be used for classifications using the ratio
     * \f$ \mathcal{L}_m / \mathcal{L}_n \f$ of the likelihoods for two
     * particle types m and n.
     *
     * @param type  The desired particle hypothesis.
     * @param charge The charge of the reconstructed track.
     */
    float getLogLikelihood(const Const::ChargedStable& type, const short& charge) const
    {
      return m_logl[getChargeAwareIndex(type, charge)];
    }

    /** returns exp(getLogLikelihood(type)) with sufficient precision. */
    double getLikelihood(const Const::ChargedStable& type, const short& charge) const
    {
      return exp((double)m_logl[getChargeAwareIndex(type, charge)]);
    }

    /** corresponding setter for m_logl. */
    void setLogLikelihood(const Const::ChargedStable& type, const short& charge, float logl)
    {
      m_logl[getChargeAwareIndex(type, charge)] = logl;
    }

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

    /** Number of particle hypotheses */
    static const int c_noOfHypotheses = Const::ChargedStable::c_SetSize * 2;

  private:

    /** Offset to pick correct index in array */
    static const unsigned int c_offset = Const::ChargedStable::c_SetSize;

    float m_logl[c_noOfHypotheses]; /**< log likelihood for each particle, not including momentum prior */

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
#endif
