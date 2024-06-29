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
  class ckf_ToPXD_FiltersSetting: public TObject {

  public:

    /** Default constructor, parameters set to NaN */
    ckf_ToPXD_FiltersSetting();

    /** Destructor */
    ~ckf_ToPXD_FiltersSetting() {}

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
      m_param_PtThresholdTrackToHitCut = PtThresholdTrackToHitCut;
    }

    /** Set the PhiInterceptToHitCut
     * @param PhiInterceptToHitCut intercept parameter
     */
    void setPhiInterceptToHitCut(const double PhiInterceptToHitCut)
    {
      m_param_PhiInterceptToHitCut = PhiInterceptToHitCut;
    }

    /** Set the EtaInterceptToHitCut
     * @param EtaInterceptToHitCut intercept parameter
     */
    void setEtaInterceptToHitCut(const double EtaInterceptToHitCut)
    {
      m_param_EtaInterceptToHitCut = EtaInterceptToHitCut;
    }

    /** Set the PhiRecoTrackToHitCut
     * @param PhiRecoTrackToHitCut intercept parameter
     */
    void setPhiRecoTrackToHitCut(const double PhiRecoTrackToHitCut)
    {
      m_param_PhiRecoTrackToHitCut = PhiRecoTrackToHitCut;
    }

    /** Set the EtaRecoTrackToHitCut
     * @param EtaRecoTrackToHitCut intercept parameter
     */
    void setEtaRecoTrackToHitCut(const double EtaRecoTrackToHitCut)
    {
      m_param_EtaRecoTrackToHitCut = EtaRecoTrackToHitCut;
    }

    /** Set the PhiHitHitCut
     * @param PhiHitHitCut intercept parameter
     */
    void setPhiHitHitCut(const double PhiHitHitCut)
    {
      m_param_PhiHitHitCut = PhiHitHitCut;
    }

    /** Set the EtaHitHitCut
     * @param EtaHitHitCut intercept parameter
     */
    void setEtaHitHitCut(const double EtaHitHitCut)
    {
      m_param_EtaHitHitCut = EtaHitHitCut;
    }

    /** Set the PhiOverlapHitHitCut
     * @param PhiOverlapHitHitCut intercept parameter
     */
    void setPhiOverlapHitHitCut(const double PhiOverlapHitHitCut)
    {
      m_param_PhiOverlapHitHitCut = PhiOverlapHitHitCut;
    }

    /** Set the EtaOverlapHitHitCut
     * @param EtaOverlapHitHitCut intercept parameter
     */
    void setEtaOverlapHitHitCut(const double EtaOverlapHitHitCut)
    {
      m_param_EtaOverlapHitHitCut = EtaOverlapHitHitCut;
    }

    /** Set the PXDInterceptsName
     * @param PXDInterceptsName intercept parameter
     */
    void setPXDInterceptsName(const std::string PXDInterceptsName)
    {
      m_param_PXDInterceptsName = PXDInterceptsName;
    }

    /** Get the hitFilterName
     */
    const std::string& getHitFilterName() const { return this->m_hitFilterName; }

    /** Get the seedFilterName
     */
    const std::string& getSeedFilterName() const { return this->m_seedFilterName; }

    /** Get the PtThresholdTrackToHitCut
     */
    const double& getPtThresholdTrackToHitCut() const { return this->m_param_PtThresholdTrackToHitCut; }

    /** Get the PhiInterceptToHitCut
     */
    const double& getPhiInterceptToHitCut() const { return this->m_param_PhiInterceptToHitCut; }

    /** Get the EtaInterceptToHitCut
     */
    const double& getEtaInterceptToHitCut() const { return this->m_param_EtaInterceptToHitCut; }

    /** Get the PhiRecoTrackToHitCut
     */
    const double& getPhiRecoTrackToHitCut() const { return this->m_param_PhiRecoTrackToHitCut; }

    /** Get the EtaRecoTrackToHitCut
     */
    const double& getEtaRecoTrackToHitCut() const { return this->m_param_EtaRecoTrackToHitCut; }

    /** Get the PhiHitHitCut
     */
    const double& getPhiHitHitCut() const { return this->m_param_PhiHitHitCut; }

    /** Get the EtaHitHitCut
     */
    const double& getEtaHitHitCut() const { return this->m_param_EtaHitHitCut; }

    /** Get the PhiOverlapHitHitCut
     */
    const double& getPhiOverlapHitHitCut() const { return this->m_param_PhiOverlapHitHitCut; }

    /** Get the EtaOverlapHitHitCut
     */
    const double& getEtaOverlapHitHitCut() const { return this->m_param_EtaOverlapHitHitCut; }

    /** Get the PXDInterceptsName
     */
    const std::string& getPXDInterceptsName() const { return this->m_param_PXDInterceptsName; }


  private:

    /** The hitFilterName */
    std::string m_hitFilterName = "not set";

    /** The seedFilterName */
    std::string m_seedFilterName = "not set";

    // parameter values for the intercept filter only
    // the seed filter parameter values are not stored in this payload

    /** Treshold on pT to apply inverse pT scale on cut value */
    double m_param_PtThresholdTrackToHitCut;
    /** Cut in phi for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state */
    double m_param_PhiInterceptToHitCut;
    /** Cut in eta for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state*/
    double m_param_EtaInterceptToHitCut;
    /** Cut in phi for the difference between RecoTrack information and current hit-based state */
    double m_param_PhiRecoTrackToHitCut;
    /** Cut in eta for the difference between RecoTrack information and current hit-based state */
    double m_param_EtaRecoTrackToHitCut;
    /** Cut in phi between two hit-based states*/
    double m_param_PhiHitHitCut;
    /** Cut in eta between two hit-based states*/
    double m_param_EtaHitHitCut;
    /** Cut in phi between two hit-based states in ladder overlap*/
    double m_param_PhiOverlapHitHitCut;
    /** Cut in eta between two hit-based states in ladder overlap */
    double m_param_EtaOverlapHitHitCut;
    /** Name of the PXDIntercepts StoreArray */
    std::string m_param_PXDInterceptsName = "not set";

    ClassDef(ckf_ToPXD_FiltersSetting, 1);  /**< ClassDef, necessary for ROOT */
  };
}
