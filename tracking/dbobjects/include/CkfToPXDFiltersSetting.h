/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {

  /** The payload containing
   * - the toPXD CKF hitFilter and seedFilter names
   * - the parameters values for the `intercept` filters only
   */
  class CkfToPXDFiltersSetting: public TObject {

  public:

    /** Default constructor, parameters set to NaN */
    CkfToPXDFiltersSetting();

    /** Destructor */
    ~CkfToPXDFiltersSetting() {}

    /** Set the hitFilterName
     * @param hitFilterName name of the hitFilter
     */
    void setHitFilterName(const std::string hitFilterName)
    {
      m_hitFilterName = hitFilterName;
    }
    /** Set the seedFilterName
     * @param seedFilterName name of the seedFilter
     */
    void setSeedFilterName(const std::string seedFilterName)
    {
      m_seedFilterName = seedFilterName;
    }

    /** Set the PtThresholdTrackToHitCut
     * @param PtThresholdTrackToHitCut intercept parameter
     */
    void setPtThresholdTrackToHitCut(const double PtThresholdTrackToHitCut)
    {
      m_paramPtThresholdTrackToHitCut = PtThresholdTrackToHitCut;
    }

    /** Set the PhiInterceptToHitCut
     * @param PhiInterceptToHitCut intercept parameter
     */
    void setPhiInterceptToHitCut(const double PhiInterceptToHitCut)
    {
      m_paramPhiInterceptToHitCut = PhiInterceptToHitCut;
    }

    /** Set the EtaInterceptToHitCut
     * @param EtaInterceptToHitCut intercept parameter
     */
    void setEtaInterceptToHitCut(const double EtaInterceptToHitCut)
    {
      m_paramEtaInterceptToHitCut = EtaInterceptToHitCut;
    }

    /** Set the PhiRecoTrackToHitCut
     * @param PhiRecoTrackToHitCut intercept parameter
     */
    void setPhiRecoTrackToHitCut(const double PhiRecoTrackToHitCut)
    {
      m_paramPhiRecoTrackToHitCut = PhiRecoTrackToHitCut;
    }

    /** Set the EtaRecoTrackToHitCut
     * @param EtaRecoTrackToHitCut intercept parameter
     */
    void setEtaRecoTrackToHitCut(const double EtaRecoTrackToHitCut)
    {
      m_paramEtaRecoTrackToHitCut = EtaRecoTrackToHitCut;
    }

    /** Set the PhiHitHitCut
     * @param PhiHitHitCut intercept parameter
     */
    void setPhiHitHitCut(const double PhiHitHitCut)
    {
      m_paramPhiHitHitCut = PhiHitHitCut;
    }

    /** Set the EtaHitHitCut
     * @param EtaHitHitCut intercept parameter
     */
    void setEtaHitHitCut(const double EtaHitHitCut)
    {
      m_paramEtaHitHitCut = EtaHitHitCut;
    }

    /** Set the PhiOverlapHitHitCut
     * @param PhiOverlapHitHitCut intercept parameter
     */
    void setPhiOverlapHitHitCut(const double PhiOverlapHitHitCut)
    {
      m_paramPhiOverlapHitHitCut = PhiOverlapHitHitCut;
    }

    /** Set the EtaOverlapHitHitCut
     * @param EtaOverlapHitHitCut intercept parameter
     */
    void setEtaOverlapHitHitCut(const double EtaOverlapHitHitCut)
    {
      m_paramEtaOverlapHitHitCut = EtaOverlapHitHitCut;
    }

    /** Set the PXDInterceptsName
     * @param PXDInterceptsName intercept parameter
     */
    void setPXDInterceptsName(const std::string PXDInterceptsName)
    {
      m_paramPXDInterceptsName = PXDInterceptsName;
    }

    /** Get the hitFilterName
     */
    const std::string& getHitFilterName() const { return this->m_hitFilterName; }

    /** Get the seedFilterName
     */
    const std::string& getSeedFilterName() const { return this->m_seedFilterName; }

    /** Get the PtThresholdTrackToHitCut
     */
    const double& getPtThresholdTrackToHitCut() const { return this->m_paramPtThresholdTrackToHitCut; }

    /** Get the PhiInterceptToHitCut
     */
    const double& getPhiInterceptToHitCut() const { return this->m_paramPhiInterceptToHitCut; }

    /** Get the EtaInterceptToHitCut
     */
    const double& getEtaInterceptToHitCut() const { return this->m_paramEtaInterceptToHitCut; }

    /** Get the PhiRecoTrackToHitCut
     */
    const double& getPhiRecoTrackToHitCut() const { return this->m_paramPhiRecoTrackToHitCut; }

    /** Get the EtaRecoTrackToHitCut
     */
    const double& getEtaRecoTrackToHitCut() const { return this->m_paramEtaRecoTrackToHitCut; }

    /** Get the PhiHitHitCut
     */
    const double& getPhiHitHitCut() const { return this->m_paramPhiHitHitCut; }

    /** Get the EtaHitHitCut
     */
    const double& getEtaHitHitCut() const { return this->m_paramEtaHitHitCut; }

    /** Get the PhiOverlapHitHitCut
     */
    const double& getPhiOverlapHitHitCut() const { return this->m_paramPhiOverlapHitHitCut; }

    /** Get the EtaOverlapHitHitCut
     */
    const double& getEtaOverlapHitHitCut() const { return this->m_paramEtaOverlapHitHitCut; }

    /** Get the PXDInterceptsName
     */
    const std::string& getPXDInterceptsName() const { return this->m_paramPXDInterceptsName; }


  private:

    /** The hitFilterName */
    std::string m_hitFilterName = "not set";

    /** The seedFilterName */
    std::string m_seedFilterName = "not set";

    // parameter values for the intercept filter only
    // the seed filter parameter values are not stored in this payload

    /** Treshold on pT to apply inverse pT scale on cut value */
    double m_paramPtThresholdTrackToHitCut;
    /** Cut in phi for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state */
    double m_paramPhiInterceptToHitCut;
    /** Cut in eta for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state*/
    double m_paramEtaInterceptToHitCut;
    /** Cut in phi for the difference between RecoTrack information and current hit-based state */
    double m_paramPhiRecoTrackToHitCut;
    /** Cut in eta for the difference between RecoTrack information and current hit-based state */
    double m_paramEtaRecoTrackToHitCut;
    /** Cut in phi between two hit-based states*/
    double m_paramPhiHitHitCut;
    /** Cut in eta between two hit-based states*/
    double m_paramEtaHitHitCut;
    /** Cut in phi between two hit-based states in ladder overlap*/
    double m_paramPhiOverlapHitHitCut;
    /** Cut in eta between two hit-based states in ladder overlap */
    double m_paramEtaOverlapHitHitCut;
    /** Name of the PXDIntercepts StoreArray */
    std::string m_paramPXDInterceptsName = "not set";

    ClassDef(CkfToPXDFiltersSetting, 1);  /**< ClassDef, necessary for ROOT */
  };
}
