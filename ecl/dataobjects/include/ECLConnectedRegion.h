/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store connected regions (CRs)
   */

  class ECLConnectedRegion : public RelationsObject {
  public:

    /** default constructor for ROOT */
    ECLConnectedRegion()
    {
      m_CRId                     = 0;     /**< CR identifier */
      m_IsTrack                  = false; /**< track or no track? */
      m_LikelihoodMIPNGamma      = -1.0;  /**< Likelihood for MIPNGamma */
      m_LikelihoodChargedHadron  = -1.0;  /**< Likelihood for Charged Hadron */
      m_LikelihoodElectronNGamma = -1.0;  /**< Likelihood for ElectronNGamma */
      m_LikelihoodNGamma         = -1.0;  /**< Likelihood for NGamma */
      m_LikelihoodNeutralHadron  = -1.0;  /**< Likelihood for Neutral Hadron */
      m_LikelihoodMergedPi0      = -1.0;  /**< Likelihood for MergedPi0 */
    }

    // setters
    /*! Set CR identifier
     */
    void setCRId(int CRId) { m_CRId = CRId; }

    /*! Set track or not
     */
    void setIsTrack(bool IsTrack) { m_IsTrack = IsTrack; }

    /*! Set Likelihood for MIPNGamma
     */
    void setLikelihoodMIPNGamma(float likelihood) { m_LikelihoodMIPNGamma = likelihood; }

    /*! Set Likelihood for Charged Hadron
     */
    void setLikelihoodChargedHadron(float likelihood) { m_LikelihoodChargedHadron = likelihood; }

    /*! Set Likelihood for ElectronNGamma
     */
    void setLikelihoodElectronNGamma(float likelihood) { m_LikelihoodElectronNGamma = likelihood; }

    /*! Set Likelihood for NGamma
     */
    void setLikelihoodNGamma(float likelihood) { m_LikelihoodNGamma = likelihood; }

    /*! Set Likelihood for Neutral Hadron
     */
    void setLikelihoodNeutralHadron(float likelihood) { m_LikelihoodNeutralHadron = likelihood; }

    /*! Set Likelihood for MergedPi0
     */
    void setLikelihoodMergedPi0(float likelihood) { m_LikelihoodMergedPi0 = likelihood; }

    // getters
    /*! Get CR identifieer
     * @return CRId
     */
    int getCRId() const { return m_CRId; }

    /*! Get boolean to flag if a track is matched to the CR
     * @return IsTrack
     */
    bool isTrack() const { return m_IsTrack; }

    /*! Get MIPNGamma likelihood (ECL based only) T1
     * @return LikelihoodMIPNGamma
     */
    float getLikelihoodMIPNGamma() const { return m_LikelihoodMIPNGamma; }

    /*! Get Charged Hadron likelihood (ECL based only) T2
     * @return LikelihoodChargedHadron
     */
    float getLikelihoodChargedHadron() const { return m_LikelihoodChargedHadron; }

    /*! Get ElectronNGamma likelihood (ECL based only) T3
     * @return LikelihoodElectronNGamma
     */
    float getLikelihoodElectronNGamma() const { return m_LikelihoodElectronNGamma; }

    /*! Get NGamma likelihood (ECL based only) N1
     * @return LikelihoodNGamma
     */
    float getLikelihoodNGamma() const { return m_LikelihoodNGamma; }

    /*! Get Neutral Hadron likelihood (ECL based only) N2
     * @return LikelihoodNeutralHadron
     */
    float getLikelihoodNeutralHadron() const { return m_LikelihoodNeutralHadron; }

    /*! Get Merged Pi0 likelihood (ECL based only) N3
     * @return LikelihoodMergedPi0
     */
    float getLikelihoodMergedPi0() const { return m_LikelihoodMergedPi0; }

  private:
    int m_CRId;                         /**< CR ID */
    bool m_IsTrack;                     /**< true if a track is matched to the connected region */
    float m_LikelihoodMIPNGamma;        /**< likelihood for T1  (MIP and any photons (including no photons)*/
    float m_LikelihoodChargedHadron;    /**< likelihood for T2 (charged hadron) */
    float m_LikelihoodElectronNGamma;   /**< likelihood for T3 (electron and any photons (including no photons) */
    float m_LikelihoodNGamma;           /**< likelihood for N1 (all photons)*/
    float m_LikelihoodNeutralHadron;    /**< likelihood for N2 (neutral hadron) */
    float m_LikelihoodMergedPi0;        /**< likelihood for N3 (merged pi0) */

    // 1: First version
    // 2: Added enumerator to hold hypothesis Ids
    // keep 2, but removed the enumerator (moved to ECLCluster)
    ClassDef(ECLConnectedRegion, 2); /**< ClassDef */

  };

} // end namespace Belle2

