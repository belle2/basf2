/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MUID_H
#define MUID_H

#include <framework/datastore/RelationsObject.h>
#include <TMatrixD.h>

namespace Belle2 {

  //! Store the muon-identification information for an extrapolated track
  class Muid : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    Muid();

    //! Constructor with parameter (pdgCode)
    Muid(int);

    //! Destructor
    virtual ~Muid() {}

    //! returns muon PDF value for this extrapolation
    double getMuonPDFValue() const { return m_MuonPDFValue; }

    //! returns pion PDF value for this extrapolation
    double getPionPDFValue() const { return m_PionPDFValue; }

    //! returns kaon PDF value for this extrapolation
    double getKaonPDFValue() const { return m_KaonPDFValue; }

    //! returns miss PDF value for this extrapolation
    double getMissPDFValue() const { return m_MissPDFValue; }

    //! returns junk PDF value for this extrapolation
    double getJunkPDFValue() const { return m_JunkPDFValue; }

    //! returns status word (bit pattern) for this extrapolation
    unsigned int getStatus() const { return m_Status; }

    //! returns chi-squared for this extrapolation
    double getChiSquared() const { return m_ChiSquared; }

    //! returns # of degrees of freedom used in chi-squared calculation
    int getDegreesOfFreedom() const { return m_DegreesOfFreedom; }

    //! returns outcome of this extrapolation
    unsigned int getOutcome() const { return m_Outcome; }

    //! returns Final BKLM layer crossed by track during extrapolation
    int getBarrelExtLayer() const { return m_BarrelExtLayer; }

    //! returns Final EKLM layer crossed by track during extrapolation
    int getEndcapExtLayer() const { return m_EndcapExtLayer; }

    //! returns Outermost BKLM layer with a matching hit
    int getBarrelHitLayer() const { return m_BarrelHitLayer; }

    //! returns Outermost EKLM layer with a matching hit
    int getEndcapHitLayer() const { return m_EndcapHitLayer; }

    //! returns Final BKLM or EKLM layer crossed by track during extrapolation
    int getExtLayer() const { return m_ExtLayer; }

    //! returns Outermost BKLM or EKLM layer with a matching hit
    int getHitLayer() const { return m_HitLayer; }

    //! returns Layer-crossing bit pattern during extrapolation
    unsigned int getExtLayerPattern() const { return m_ExtLayerPattern; }

    //! returns Matching-hit bit pattern
    unsigned int getHitLayerPattern() const { return m_HitLayerPattern; }

    //! assigns muon PDF value for this extrapolation
    void setMuonPDFValue(double pdfValue) { m_MuonPDFValue = pdfValue; }

    //! assigns pion PDF value for this extrapolation
    void setPionPDFValue(double pdfValue) { m_PionPDFValue = pdfValue; }

    //! assigns kaon PDF value for this extrapolation
    void setKaonPDFValue(double pdfValue) { m_KaonPDFValue = pdfValue; }

    //! assigns miss PDF value for this extrapolation
    void setMissPDFValue(double pdfValue) { m_MissPDFValue = pdfValue; }

    //! assigns junk PDF value for this extrapolation
    void setJunkPDFValue(double pdfValue) { m_JunkPDFValue = pdfValue; }

    //! assigns status word (bit pattern) for this extrapolation
    void setStatus(unsigned int status) { m_Status = status; }

    //! assigns chi-squared for this extrapolation
    void setChiSquared(double chiSquared) { m_ChiSquared = chiSquared; }

    //! assigns # of degrees of freedom used in chi-squared calculation
    void setDegreesOfFreedom(int dof) { m_DegreesOfFreedom = dof; }

    //! assigns outcome of this extrapolation
    void setOutcome(unsigned int outcome) { m_Outcome = outcome; }

    //! assigns Final BKLM layer crossed by track during extrapolation
    void setBarrelExtLayer(int layer) { m_BarrelExtLayer = layer; }

    //! assigns Final EKLM layer crossed by track during extrapolation
    void setEndcapExtLayer(int layer) { m_EndcapExtLayer = layer; }

    //! assigns Outermost BKLM layer with a matching hit
    void setBarrelHitLayer(int layer) { m_BarrelHitLayer = layer; }

    //! assigns Outermost EKLM layer with a matching hit
    void setEndcapHitLayer(int layer) { m_EndcapHitLayer = layer; }

    //! assigns Final BKLM or EKLM layer crossed by track during extrapolation
    void setExtLayer(int layer) { m_ExtLayer = layer; }

    //! assigns Outermost BKLM or EKLM layer with a matching hit
    void setHitLayer(int layer) { m_HitLayer = layer; }

    //! assigns Layer-crossing bit pattern during extrapolation
    void setExtLayerPattern(unsigned int pattern) { m_ExtLayerPattern = pattern; }

    //! assigns Matching-hit bit pattern
    void setHitLayerPattern(unsigned int pattern) { m_HitLayerPattern = pattern; }

  private:

    //! PDG particleID hypothesis used for this extrapolation (typically muon)
    int m_pdgCode;

    //! Muon PDF value for this extrapolation
    double m_MuonPDFValue;

    //! Pion PDF value for this extrapolation
    double m_PionPDFValue;

    //! Kaon PDF value for this extrapolation
    double m_KaonPDFValue;

    //! Miss PDF value for this extrapolation
    double m_MissPDFValue;

    //! Junk PDF value for this extrapolation
    double m_JunkPDFValue;

    //! status word (bit pattern) for this extrapolation
    unsigned int m_Status;

    //! chi-squared for this extrapolation
    double m_ChiSquared;

    //! # of degrees of freedom used in chi-squared calculation
    int m_DegreesOfFreedom;

    //! outcome of this extrapolation
    //! 0=track didn't reach KLM during extrapolation
    //! 1=track stopped in BKLM during extrapolation
    //! 2=track stopped in EKLM during extrapolation
    //! 3=track exited BKLM during extrapolation
    //! 4=track exited EKLM during extrapolation
    unsigned int m_Outcome;

    //! Final BKLM layer crossed by track during extrapolation
    int m_BarrelExtLayer;

    //! Final EKLM layer crossed by track during extrapolation
    int m_EndcapExtLayer;

    //! Outermost BKLM layer with a matching hit
    int m_BarrelHitLayer;

    //! Outermost EKLM layer with a matching hit
    int m_EndcapHitLayer;

    //! Final BKLM or EKLM layer crossed by track during extrapolation
    int m_ExtLayer;

    //! Outermost BKLM or EKLM layer with a matching hit
    int m_HitLayer;

    //! Layer-crossing bit pattern during extrapolation
    //! bits 0..14 = barrel layers 1..15
    //! bits 15..28 = endcap layers 1..14
    unsigned int m_ExtLayerPattern;

    //! Matching-hit bit pattern
    //! bits 0..14 = barrel layers 1..15
    //! bits 15..28 = endcap layers 1..14
    unsigned int m_HitLayerPattern;

    //! Needed to make the ROOT object storable
    ClassDef(Muid, 1)

  };

}
#endif
