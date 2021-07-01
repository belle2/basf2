/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen, Giacomo De Pietro                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

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
    int getPDGCode() const
    {
      return m_PDGCode;
    }

    /**
     * Get the charge of the particle hypothesis used during the extrapolation.
     */
    int getCharge() const;

    /**
     * Get the normalized PDF.
     * @param[in] pdg PDG code of the hypothesis.
     */
    double getPDFValue(int pdg) const
    {
      return m_PDFValue[Const::ChargedStable(pdg).getIndex()];
    }

    /**
     * Get the normalized PDF for the muon hypothesis.
     */
    double getMuonPDFValue() const
    {
      return getPDFValue(Const::muon.getPDGCode());
    }

    /**
     * Get the normalized PDF for the pion hypothesis.
     */
    double getPionPDFValue() const
    {
      return getPDFValue(Const::pion.getPDGCode());
    }

    /**
     * Get the normalized PDF for the kaon hypothesis.
     */
    double getKaonPDFValue() const
    {
      return getPDFValue(Const::kaon.getPDGCode());
    }

    /**
     * Get the normalized PDF for the proton hypothesis.
     */
    double getProtonPDFValue() const
    {
      return getPDFValue(Const::proton.getPDGCode());
    }

    /**
     * Get the normalized PDF for the deuteron hypothesis.
     */
    double getDeuteronPDFValue() const
    {
      return getPDFValue(Const::deuteron.getPDGCode());
    }

    /**
     * Get the normalized PDF for the electron hypothesis.
     */
    double getElectronPDFValue() const
    {
      return getPDFValue(Const::electron.getPDGCode());
    }

    /**
     * Get the junk flag (1 if junk, 0 if not).
     */
    bool getJunkPDFValue() const
    {
      return m_JunkPDFValue;
    }

    /**
     * Get the log-likelihood.
     * @param[in] pdg PDG code of the hypothesis.
     */
    double getLogL(int pdg) const
    {
      return m_LogL[Const::ChargedStable(pdg).getIndex()];
    }

    /**
     * Get the log-likelihood for the muon hypothesis.
     */
    double getLogL_mu() const
    {
      return getLogL(Const::muon.getPDGCode());
    }

    /**
     * Get the log-likelihood for the pion hypothesis.
     */
    double getLogL_pi() const
    {
      return getLogL(Const::pion.getPDGCode());
    }

    /**
     * Get the log-likelihood for the kaon hypothesis.
     */
    double getLogL_K() const
    {
      return getLogL(Const::kaon.getPDGCode());
    }

    /**
     * Get the log-likelihood for the proton hypothesis.
     */
    double getLogL_p() const
    {
      return getLogL(Const::proton.getPDGCode());
    }

    /**
     * Get the log-likelihood for the deuteron hypothesis.
     */
    double getLogL_d() const
    {
      return getLogL(Const::deuteron.getPDGCode());
    }

    /**
     * Get the log-likelihood for the electron hypothesis.
     */
    double getLogL_e() const
    {
      return getLogL(Const::electron.getPDGCode());
    }

    /**
     * Get the chi-squared of the extrapolation.
     */
    double getChiSquared() const
    {
      return m_ChiSquared;
    }

    /**
     * Get the number of degrees of freedom (= 2 times the number of KLM hits) for the chi-squared computation.
     */
    int getDegreesOfFreedom() const
    {
      return m_DegreesOfFreedom;
    }

    /**
     * Get the outcome of this extrapolation.
     * All the possible outcome values are defined in MuidElementNumbers
     * (see enum Outcome and calculateExtrapolationOutcome).
     */
    unsigned int getOutcome() const
    {
      return m_Outcome;
    }

    /**
     * Return if this extrapolation is in forward or backward B/EKLM.
     */
    bool getIsForward() const
    {
      return m_IsForward;
    }

    /**
     * Get the outermost BKLM layer crossed in the extrapolation.
     */
    int getBarrelExtLayer() const
    {
      return m_BarrelExtLayer;
    }

    /**
     * Get the outermost EKLM layer crossed in the extrapolation.
     */
    int getEndcapExtLayer() const
    {
      return m_EndcapExtLayer;
    }

    /**
     * Get the outermost KLM layer crossed in the extrapolation.
     */
    int getExtLayer() const
    {
      return m_ExtLayer;
    }

    /**
     * Get the outermost BKLM layer actually crossed by the track.
     */
    int getBarrelHitLayer() const
    {
      return m_BarrelHitLayer;
    }

    /**
     * Get the outermost EKLM layer actually crossed by the track.
     */
    int getEndcapHitLayer() const
    {
      return m_EndcapHitLayer;
    }

    /**
     * Get the outermost KLM layer actually crossed by the track.
     */
    int getHitLayer() const
    {
      return m_HitLayer;
    }

    /**
     * Get the pattern of the layers crossed in the extrapolation.
     */
    unsigned int getExtLayerPattern() const
    {
      return m_ExtLayerPattern;
    }

    /**
     * Get the pattern of the layers actually crossed by the track.
     */
    unsigned int getHitLayerPattern() const
    {
      return m_HitLayerPattern;
    }

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
    float getExtBKLMEfficiencyValue(int layer) const
    {
      return m_ExtBKLMEfficiencyValue[layer];
    }

    /**
     * Get the efficiency of a given EKLM layer.
     * @param[in] layer EKLM layer
     */
    float getExtEKLMEfficiencyValue(int layer) const
    {
      return m_ExtEKLMEfficiencyValue[layer];
    }

    /**
     * Set the PDG code of the particle hypothesis used during the extrapolation.
     * @param[in] pdg PDG code of the particle hypothesis.
     */
    void setPDGCode(int pdg)
    {
      m_PDGCode = pdg;
    }

    /**
     * Set the normalized PDF.
     * @param[in] pdfValue Normalized PDF.
     * @param[in] pdg      PDG code of the particle hypothesis.
     */
    void setPDFValue(double pdfValue, int pdg)
    {
      m_PDFValue[Const::ChargedStable(pdg).getIndex()] = pdfValue;
    }

    /**
     * Set the normalized PDF for the muon hypothesis.
     * @param[in] pdfValue Normalized PDF.
     */
    void setMuonPDFValue(double pdfValue)
    {
      setPDFValue(pdfValue, Const::muon.getPDGCode());
    }

    /**
     * Set the normalized PDF for the pion hypothesis.
     * @param[in] pdfValue Normalized PDF.
     */
    void setPionPDFValue(double pdfValue)
    {
      setPDFValue(pdfValue, Const::pion.getPDGCode());
    }

    /**
     * Set the normalized PDF for the kaon hypothesis.
     * @param[in] pdfValue Normalized PDF.
     */
    void setKaonPDFValue(double pdfValue)
    {
      setPDFValue(pdfValue, Const::kaon.getPDGCode());
    }

    /**
     * Set the normalized PDF for the proton hypothesis.
     * @param[in] pdfValue Normalized PDF.
     */
    void setProtonPDFValue(double pdfValue)
    {
      setPDFValue(pdfValue, Const::proton.getPDGCode());
    }

    /**
     * Set the normalized PDF for the deuteron hypothesis.
     * @param[in] pdfValue Normalized PDF.
     */
    void setDeuteronPDFValue(double pdfValue)
    {
      setPDFValue(pdfValue, Const::deuteron.getPDGCode());
    }

    /**
     * Set the normalized PDF for the electron hypothesis.
     * @param[in] pdfValue Normalized PDF.
     */
    void setElectronPDFValue(double pdfValue)
    {
      setPDFValue(pdfValue, Const::electron.getPDGCode());
    }

    /**
     * Set the junk flag (1 if junk, 0 if not).
     * @param[in] flag 1 if junk, 0 if not.
     */
    void setJunkPDFValue(bool flag)
    {
      m_JunkPDFValue = flag;
    }

    /**
     * Set the log-likelihood.
     * @param[in] logL Log-likelihood.
     * @param[in] pdg  PDG code of the hypothesis.
     */
    void setLogL(double logL, int pdg)
    {
      m_LogL[Const::ChargedStable(pdg).getIndex()] = logL;
    }

    /**
     * Set the log-likelihood for the muon hypothesis.
     * @param[in] logL Log-likelihood.
     */
    void setLogL_mu(double logL)
    {
      setLogL(logL, Const::muon.getPDGCode());
    }

    /**
     * Set the log-likelihood for the pion hypothesis.
     * @param[in] logL Log-likelihood.
     */
    void setLogL_pi(double logL)
    {
      setLogL(logL, Const::pion.getPDGCode());
    }

    /**
     * Set the log-likelihood for the kaon hypothesis.
     * @param[in] logL Log-likelihood.
     */
    void setLogL_K(double logL)
    {
      setLogL(logL, Const::kaon.getPDGCode());
    }

    /**
     * Set the log-likelihood for the proton hypothesis.
     * @param[in] logL Log-likelihood.
     */
    void setLogL_p(double logL)
    {
      setLogL(logL, Const::proton.getPDGCode());
    }

    /**
     * Set the log-likelihood for the deuteron hypothesis.
     * @param[in] logL Log-likelihood.
     */
    void setLogL_d(double logL)
    {
      setLogL(logL, Const::deuteron.getPDGCode());
    }

    /**
     * Set the log-likelihood for the electron hypothesis.
     * @param[in] logL Log-likelihood.
     */
    void setLogL_e(double logL)
    {
      setLogL(logL, Const::electron.getPDGCode());
    }

    /**
     * Set the chi-squared of the extrapolation.
     * @param[in] chiSquared Chi-squared of the extrapolation.
     */
    void setChiSquared(double chiSquared)
    {
      m_ChiSquared = chiSquared;
    }

    /**
     * Set the number of degrees of freedom (= 2 times the number of KLM hits) for the chi-square computation.
     * @param[in] dof Number of degrees of freedom.
     */
    void setDegreesOfFreedom(int dof)
    {
      m_DegreesOfFreedom = dof;
    }

    /**
     * Set the outcome of this extrapolation.
     * All the possible outcome values are defined in MuidElementNumbers
     * (see enum Outcome and calculateExtrapolationOutcome).
     * @param[in] outcome Outcome of this extrapolation.
     */
    void setOutcome(unsigned int outcome)
    {
      m_Outcome = outcome;
    }

    /**
     * Set if this extrapolation is in forward or backward B/EKLM.
     * @param[in] isForward True for forward, false for backward.
     */
    void setIsForward(bool isForward)
    {
      m_IsForward = isForward;
    }

    /**
     * Set the outermost BKLM layer crossed in the extrapolation.
     * @param[in] layer Outermost BKLM layer.
     */
    void setBarrelExtLayer(int layer)
    {
      m_BarrelExtLayer = layer;
    }

    /**
     * Set the outermost EKLM layer crossed in the extrapolation.
     * @param[in] layer Outermost EKLM layer.
     */
    void setEndcapExtLayer(int layer)
    {
      m_EndcapExtLayer = layer;
    }

    /**
     * Set the outermost EKLM layer crossed in the extrapolation.
     * @param[in] layer Outermost KLM layer.
     */
    void setExtLayer(int layer)
    {
      m_ExtLayer = layer;
    }

    /**
     * Set the outermost BKLM layer actually crossed by the track.
     * @param[in] layer Outermost BKLM layer.
     */
    void setBarrelHitLayer(int layer)
    {
      m_BarrelHitLayer = layer;
    }

    /**
     * Set the outermost EKLM layer actually crossed by the track.
     * @param[in] layer Outermost EKLM layer.
     */
    void setEndcapHitLayer(int layer)
    {
      m_EndcapHitLayer = layer;
    }

    /**
     * Set the outermost KLM layer actually crossed by the track.
     * @param[in] layer Outermost KLM layer.
     */
    void setHitLayer(int layer)
    {
      m_HitLayer = layer;
    }

    /**
     * Set the pattern of the layers crossed in the extrapolation.
     * @param[in] pattern Pattern of the extrapolation.
     */
    void setExtLayerPattern(unsigned int pattern)
    {
      m_ExtLayerPattern = pattern;
    }

    /**
     * Set the pattern of the layers actually crossed by the track.
     * @param[in] pattern Pattern of the layers actually crossed.
     */
    void setHitLayerPattern(unsigned int pattern)
    {
      m_HitLayerPattern = pattern;
    }

    /**
     * Set the efficiency of a given BKLM layer.
     * @param[in] layer      BKLM layer.
     * @param[in] efficiency Efficiency of the given layer.
     */
    void setExtBKLMEfficiencyValue(int layer, float efficiency)
    {
      m_ExtBKLMEfficiencyValue[layer] = efficiency;
    }

    /**
     * Set the efficiency of a given EKLM layer.
     * @param[in] layer      EKLM layer.
     * @param[in] efficiency Efficiency of the given layer.
     */
    void setExtEKLMEfficiencyValue(int layer, float efficiency)
    {
      m_ExtEKLMEfficiencyValue[layer] = efficiency;
    }

    /**
     * Check whether the given BKLM layer is crossed during extrapolation.
     * @param[in] layer  BKLM layer (0-based).
     */
    bool isExtrapolatedBarrelLayerCrossed(int layer) const;

    /**
     * Check whether the given EKLM layer is crossed during extrapolation.
     * @param[in] layer  EKLM layer (0-based).
     */
    bool isExtrapolatedEndcapLayerCrossed(int layer) const;

  private:

    /** PDG code of the particle hypothesis used during the extrapolation. */
    int m_PDGCode;

    /** Array of normalized PDFs. */
    float m_PDFValue[Const::ChargedStable::c_SetSize];

    /** Junk flag (1 if junk, 0 if not). */
    bool m_JunkPDFValue;

    /** Array of log-likelihoods. */
    float m_LogL[Const::ChargedStable::c_SetSize];

    /** Chi-squared of the extrapolation. */
    float m_ChiSquared;

    /** Number of degrees of freedom (= 2 times the number of KLM hits) for the chi-squared computation. */
    int m_DegreesOfFreedom;

    /** Outcome of this extrapolation. */
    unsigned int m_Outcome;

    /** Flag to determine if this extrapolation is in forward or backward B/EKLM. */
    bool m_IsForward;

    /** Outermost BKLM layer crossed in the extrapolation. */
    int m_BarrelExtLayer;

    /** Outermost EKLM layer crossed in the extrapolation. */
    int m_EndcapExtLayer;

    /** Outermost KLM layer crossed in the extrapolation. */
    int m_ExtLayer;

    /** Outermost BKLM layer actually crossed by the track. */
    int m_BarrelHitLayer;

    /** Outermost EKLM layer actually crossed by the track. */
    int m_EndcapHitLayer;

    /** Outermost KLM layer actually crossed by the track. */
    int m_HitLayer;

    /**
     * Pattern of the layers crossed in the extrapolation
     * (bits 0..14 = BKLM layers 1..15, bits 15..28 = EKLM layers 1..14).
     */
    unsigned int m_ExtLayerPattern;

    /**
     * Pattern of the layers actually crossed by the track
     * (bits 0..14 = BKLM layers 1..15, bits 15..28 = EKLM layers 1..14).
     */
    unsigned int m_HitLayerPattern;

    /** Array of BKLM layer efficiencies. */
    float m_ExtBKLMEfficiencyValue[BKLMElementNumbers::getMaximalLayerNumber()];

    /** Array of EKLM layer efficiencies. */
    float m_ExtEKLMEfficiencyValue[EKLMElementNumbers::getMaximalLayerNumber()];

    /** Class version. */
    ClassDef(KLMMuidLikelihood, 4)

  };
}
