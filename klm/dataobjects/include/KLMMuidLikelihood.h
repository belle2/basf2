B/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/eklm/dataobjects/EKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Class to store the likelihoods from KLM with additional informations related to the extrapolation.
   */
  class KLMMuidLikelihood : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    KLMMuidLikelihood();

    /**
     * Destructor.
     */
    ~KLMMuidLikelihood();

    /**
     * Get the PDG code of the particle hypothesis used during the extrapolation.
     */
    int getPDGCode() const { return m_PDGCode; }

    /**
     * Get the charge of the particle hypothesis used during the extrapolation.
     */
    int getCharge() const;

    /**
     * Get the normalized PDF.
     * @param[in] pdg PDG code of the hypothesis.
     */
    double getPDFValue(int pdg) const { return m_PDFValue[Const::ChargedStable(pdg).getIndex()]; }

    /**
     * Get the normalized PDF for the muon hypothesis.
     */
    double getMuonPDFValue() const { return getPDFValue(Const::muon.getPDGCode()); }

    /**
     * Get the normalized PDF for the pion hypothesis.
     */
    double getPionPDFValue() const { return getPDFValue(Const::pion.getPDGCode()); }

    /**
     * Get the normalized PDF for the kaon hypothesis.
     */
    double getKaonPDFValue() const { return getPDFValue(Const::kaon.getPDGCode()); }

    /**
     * Get the normalized PDF for the proton hypothesis.
     */
    double getProtonPDFValue() const { return getPDFValue(Const::proton.getPDGCode()); }

    /**
     * Get the normalized PDF for the deuteron hypothesis.
     */
    double getDeuteronPDFValue() const { return getPDFValue(Const::deuteron.getPDGCode()); }

    /**
     * Get the normalized PDF for the electron hypothesis.
     */
    double getElectronPDFValue() const { return getPDFValue(Const::electron.getPDGCode()); }

    /**
     * Get the junk flag (1 if jung, 0 if not).
     */
    double getJunkPDFValue() const { return m_JunkPDFValue; }

    /**
     * Get the log-likelihood (not normalized).
     * @param[in] pdg PDG code of the hypothesis.
     */
    double getLogL(int pdg) const { return m_LogL[Const::ChargedStable(pdg).getIndex()]; }

    /**
     * Get the log-likelihood (not normalized) for the muon hypothesis.
     */
    double getLogL_mu() const { return getLogL(Const::muon.getPDGCode()); }

    /**
     * Get the log-likelihood (not normalized) for the pion hypothesis.
     */
    double getLogL_pi() const { return getLogL(Const::pion.getPDGCode()); }

    /**
     * Get the log-likelihood (not normalized) for the kaon hypothesis.
     */
    double getLogL_K() const { return getLogL(Const::kaon.getPDGCode()); }

    /**
     * Get the log-likelihood (not normalized) for the proton hypothesis.
     */
    double getLogL_p() const { return getLogL(Const::proton.getPDGCode()); }

    /**
     * Get the log-likelihood (not normalized) for the deuteron hypothesis.
     */
    double getLogL_d() const { return getLogL(Const::deuteron.getPDGCode()); }

    /**
     * Get the log-likelihood (not normalized) for the electron hypothesis.
     */
    double getLogL_e() const { return getLogL(Const::electron.getPDGCode()); }

    /**
     * Get the chi-square of the extrapolation.
     */
    double getChiSquared() const { return m_ChiSquared; }

    /**
     * Get the number of degrees of freedom (= 2 times the number of KLM hits) for the chi-square computation.
     */
    int getDegreesOfFreedom() const { return m_DegreesOfFreedom; }

    /**
     * Get the outcome of this extrapolation.
     * All the possible outcome values are defined in MuidElementNumbers
     * (see enum Outcome and calculateExtrapolationOutcome).
     */
    unsigned int getOutcome() const { return m_Outcome; }

    /**
     * Return if this extrapolation is in forward or backward B/EKLM.
     */
    bool getIsForward() const { return m_IsForward; }

    /**
     * Get the outermost BKLM layer crossed in the extrapolation.
     */
    int getBarrelExtLayer() const { return m_BarrelExtLayer; }

    /**
     * Get the outermost EKLM layer crossed in the extrapolation.
     */
    int getEndcapExtLayer() const { return m_EndcapExtLayer; }

    /**
     * Get the outermost KLM layer crossed in the extrapolation.
     */
    int getExtLayer() const { return m_ExtLayer; }

    /**
     * Get the outermost BKLM layer actually crossed by the track.
     */
    int getBarrelHitLayer() const { return m_BarrelHitLayer; }

    /**
     * Get the outermost EKLM layer actually crossed by the track.
     */
    int getEndcapHitLayer() const { return m_EndcapHitLayer; }

    /**
     * Get the outermost KLM layer actually crossed by the track.
     */
    int getHitLayer() const { return m_HitLayer; }

    /**
     * Get the pattern of the layers crossed in the extrapolation.
     */
    unsigned int getExtLayerPattern() const { return m_ExtLayerPattern; }

    /**
     * Get the pattern of the layers actually crossed by the track.
     */
    unsigned int getHitLayerPattern() const { return m_HitLayerPattern; }

    /**
     * Get the total number of crossed BKLM layers.
     */
    unsigned int getTotalBarrelHits() const;

    /**
     * Get the total number of crossed EKLM layers.
     */
    unsigned int getTotalEndcapHits() const;

    /**
     * Get the efficiency of a given BKLM layer.
     * @param[in] layer BKLM layer.
     */
    float getExtBKLMEfficiencyValue(int layer) const { return m_ExtBKLMEfficiencyValue[layer]; }

    /**
     * Get the efficiency of a given EKLM layer.
     * @param[in] layer EKLM layer
     */
    float getExtEKLMEfficiencyValue(int layer) const { return m_ExtEKLMEfficiencyValue[layer]; }

    //! set PDG code of the hypothesis for this extrapolation
    //! @param pdg PDG code of the hypothesis for this extrapolation
    void setPDGCode(int pdg) { m_PDGCode = pdg; }

    //! assign normalized PDF value for this extrapolation
    //! @param pdfValue Normalized PDF value
    //! @param pdg PDG code of the hypothesis
    void setPDFValue(double pdfValue, int pdg) { m_PDFValue[Const::ChargedStable(pdg).getIndex()] = pdfValue; }

    //! assign muon PDF value for this extrapolation
    //! @param pdfValue muon PDF value (normalized) for this extrapolation
    void setMuonPDFValue(double pdfValue) { setPDFValue(pdfValue, Const::muon.getPDGCode()); }

    //! assign pion PDF value for this extrapolation
    //! @param pdfValue pion PDF value (normalized) for this extrapolation
    void setPionPDFValue(double pdfValue) { setPDFValue(pdfValue, Const::pion.getPDGCode()); }

    //! assign kaon PDF value for this extrapolation
    //! @param pdfValue kaon PDF value (normalized) for this extrapolation
    void setKaonPDFValue(double pdfValue) { setPDFValue(pdfValue, Const::kaon.getPDGCode()); }

    //! assign proton PDF value for this extrapolation
    //! @param pdfValue proton PDF value (normalized) for this extrapolation
    void setProtonPDFValue(double pdfValue) { setPDFValue(pdfValue, Const::proton.getPDGCode()); }

    //! assign deuteron PDF value for this extrapolation
    //! @param pdfValue deuteron PDF value (normalized) for this extrapolation
    void setDeuteronPDFValue(double pdfValue) { setPDFValue(pdfValue, Const::deuteron.getPDGCode()); }

    //! assign electron PDF value for this extrapolation
    //! @param pdfValue electron PDF value (normalized) for this extrapolation
    void setElectronPDFValue(double pdfValue) { setPDFValue(pdfValue, Const::electron.getPDGCode()); }

    //! assign junk flag for this extrapolation
    //! @param pdfValue junk flag for this extrapolation (0 if not junk, 1 if junk)
    void setJunkPDFValue(double pdfValue) { m_JunkPDFValue = pdfValue; }

    //! assign unnormalized log-likelihood for this extrapolation
    //! @param value Logarithm of PDF value (unnormalized) for this extrapolation
    //! @param pdg PDG code of the hypothesis
    void setLogL(double value, int pdg) { m_LogL[Const::ChargedStable(pdg).getIndex()] = value; }

    //! assign muon log-likelihood for this extrapolation
    //! @param value logarithm of muon PDF value (unnormalized) for this extrapolation
    void setLogL_mu(double value) { setLogL(value, Const::muon.getPDGCode()); }

    //! assign pion log-likelihood for this extrapolation
    //! @param value logarithm of pion PDF value (unnormalized) for this extrapolation
    void setLogL_pi(double value) { setLogL(value, Const::pion.getPDGCode()); }

    //! assign kaon log-likelihood for this extrapolation
    //! @param value logarithm of kaon PDF value (unnormalized) for this extrapolation
    void setLogL_K(double value) { setLogL(value, Const::kaon.getPDGCode()); }

    //! assign proton log-likelihood for this extrapolation
    //! @param value logarithm of proton PDF value (unnormalized) for this extrapolation
    void setLogL_p(double value) { setLogL(value, Const::proton.getPDGCode()); }

    //! assign deuteron log-likelihood for this extrapolation
    //! @param value logarithm of deuteron PDF value (unnormalized) for this extrapolation
    void setLogL_d(double value) { setLogL(value, Const::deuteron.getPDGCode()); }

    //! assign electron log-likelihood for this extrapolation
    //! @param value logarithm of electron PDF value (unnormalized) for this extrapolation
    void setLogL_e(double value) { setLogL(value, Const::electron.getPDGCode()); }

    //! assign status word (bit pattern) for this extrapolation
    //! @param status final state of this extrapolation
    void setStatus(unsigned int status) { m_Status = status; }

    //! assign chi-squared for this extrapolation
    //! @param chiSquared total chi-squared for this extrapolation
    void setChiSquared(double chiSquared) { m_ChiSquared = chiSquared; }

    //! assign number of degrees of freedom used in chi-squared calculation
    //! @param dof number of degrees of freedom for this extrapolation
    void setDegreesOfFreedom(int dof) { m_DegreesOfFreedom = dof; }

    //! assign outcome of this extrapolation
    //! @param outcome final outcome of this extrapolation
    void setOutcome(unsigned int outcome) { m_Outcome = outcome; }

    //! assign forward or backward for this extrapolation
    //! @param isForward fordward or backward for this extrapolation
    void setIsForward(bool isForward) { m_IsForward = isForward; }

    //! assign outermost BKLM layer crossed by track during extrapolation
    //! @param layer outermost BKLM layer crossed by track during extrapolation
    void setBarrelExtLayer(int layer) { m_BarrelExtLayer = layer; }

    //! assign outermost EKLM layer crossed by track during extrapolation
    //! @param layer outermost EKLM layer crossed by track during extrapolation
    void setEndcapExtLayer(int layer) { m_EndcapExtLayer = layer; }

    //! assign outermost BKLM layer with a matching hit
    //! @param layer outermost BKLM layer with a matching hit
    void setBarrelHitLayer(int layer) { m_BarrelHitLayer = layer; }

    //! assign outermost EKLM layer with a matching hit
    //! @param layer outermost EKLM layer with a matching hit
    void setEndcapHitLayer(int layer) { m_EndcapHitLayer = layer; }

    //! assign outermost BKLM or EKLM layer crossed by track during extrapolation
    //! @param layer outermost BKLM or EKLM crossed by track during extrapolation
    void setExtLayer(int layer) { m_ExtLayer = layer; }

    //! assign Outermost BKLM or EKLM layer with a matching hit
    //! @param layer outermost BKLM or EKLM layer with a matching hit
    void setHitLayer(int layer) { m_HitLayer = layer; }

    //! assign Layer-crossing bit pattern during extrapolation
    //! @param pattern bit pattern of crossed layers during extrapolation
    void setExtLayerPattern(unsigned int pattern) { m_ExtLayerPattern = pattern; }

    //! assign Matching-hit bit pattern
    //! @param pattern bit pattern of matching-hit layers during extrapolation
    void setHitLayerPattern(unsigned int pattern) { m_HitLayerPattern = pattern; }

    //! assign BKLMefficiency value
    void setExtBKLMEfficiencyValue(int index, float efficiencyValue) { m_ExtBKLMEfficiencyValue[index] = efficiencyValue; }

    //! assign EKLM efficiency value
    void setExtEKLMEfficiencyValue(int index, float efficiencyValue) { m_ExtEKLMEfficiencyValue[index] = efficiencyValue; }

  private:

    /** PDG code of the particle hypothesis used during the extrapolation. */
    int m_PDGCode;

    /** Array of normalized PDFs. */
    float m_PDFValue[Const::ChargedStable::c_SetSize];

    /** Junk flag (1 if junk, 0 if not). */
    bool m_JunkPDFValue;

    /** Array of log-likelihoods (not normalized). */
    float m_LogL[Const::ChargedStable::c_SetSize];

    /** Chi-square of the extrapolation. */
    float m_ChiSquared;

    /** Number of degrees of freedom (= 2 times the number of KLM hits) for the chi-square computation. */
    int m_DegreesOfFreedom;

    /** Outcome of this extrapolation. */
    unsigned int m_Outcome;

    /** Flag to determine if this extrapolation is in forward or backward B/EKLM. */
    bool m_IsForward;

    //! outermost BKLM layer crossed by track during extrapolation
    int m_BarrelExtLayer;

    //! outermost EKLM layer crossed by track during extrapolation
    int m_EndcapExtLayer;

    //! outermost BKLM layer with a matching hit
    int m_BarrelHitLayer;

    //! outermost EKLM layer with a matching hit
    int m_EndcapHitLayer;

    //! outermost BKLM or EKLM layer crossed by track during extrapolation
    int m_ExtLayer;

    //! outermost BKLM or EKLM layer with a matching hit
    int m_HitLayer;

    //! Layer-crossing bit pattern during extrapolation
    //! (bits 0..14 = barrel layers 1..15,
    //! bits 15..28 = endcap layers 1..14)
    unsigned int m_ExtLayerPattern;

    //! Matching-hit bit pattern
    //! (bits 0..14 = barrel layers 1..15,
    //! bits 15..28 = endcap layers 1..14)
    unsigned int m_HitLayerPattern;

    /** Array of BKLM layer efficiencies. */
    float m_ExtBKLMEfficiencyValue[BKLMElementNumbers::getMaximalLayerNumber()];

    /** Array of EKLM layer efficiencies. */
    float m_ExtEKLMEfficiencyValue[EKLMElementNumbers::getMaximalLayerNumber()];

    /** Class version. */
    ClassDef(KLMMuidLikelihood, 4)

  };
}
