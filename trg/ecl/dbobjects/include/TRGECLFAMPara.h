/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLFAMPara.h
// Section  : TRG ECL
// Owner    : HanEol Cho/InSu Lee/Yuuji Unno
// Email    : hecho@hep.hanyang.ac.kr / islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLFAMPARA_H
#define TRGECLFAMPARA_H

#include <TObject.h>

namespace Belle2 {

  //! Raw TC result nefor digitizing
  class TRGECLFAMPara : public TObject {
  public:
    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLFAMPara() :
      m_FPGAversion(1),
      m_TCId(0),
      m_FAMId(0),
      m_ChannelId(0),
      m_TEreconstruct(0),
      m_ConversionFactor(0),
      m_Toffset(0),
      m_Threshold(0),
      m_Wavemean(0),
      m_Wavesigma(0)
    {
      m_SignalPDF.clear();

      m_NoiseCovarianceMatrix.clear();

    }
    //! Construction
    TRGECLFAMPara(
      int FPGAversion,
      int TCId,
      int FAMId,
      int ChannelId,
      int TEreconstruct,
      double ConversionFactor,
      int Toffset,
      int Threshold,
      int Wavemean,
      int Wavesigma,
      const std::vector<double>&  SignalPDF,
      const std::vector<double>&  NoiseCovarianceMatrix

    ):
      m_FPGAversion(FPGAversion),
      m_TCId(TCId),
      m_FAMId(FAMId),
      m_ChannelId(ChannelId),
      m_TEreconstruct(TEreconstruct),
      m_ConversionFactor(ConversionFactor),
      m_Toffset(Toffset),
      m_Threshold(Threshold),
      m_Wavemean(Wavemean),
      m_Wavesigma(Wavesigma),
      m_SignalPDF(SignalPDF),
      m_NoiseCovarianceMatrix(NoiseCovarianceMatrix)
    { }

    //! Set FPGAversion
    void setFPGAversion(int FPGAversion) { m_FPGAversion = FPGAversion; }

    //! Set TCId
    void setTCId(int TCId) { m_TCId = TCId; }

    //! Set FAMId
    void setFAMId(int FAMId) { m_FAMId = FAMId; }

    //! Set TCId
    void setChannelId(int ChannelId) { m_ChannelId = ChannelId; }

    //! Set T&E reconstruction method
    void setTEreconstruct(int TEreconstruct) { m_TEreconstruct = TEreconstruct; }

    //! Set ConversionFactor CC
    void setConversionFactor(double ConversionFactor) { m_ConversionFactor = ConversionFactor; }

    //! Set T offset
    void setToffset(int Toffset) { m_Toffset = Toffset; }

    //! Set Threshold
    void setThreshold(int Threshold) { m_Threshold = Threshold; }

    //! Set Wavemean
    void setWavemean(int Wavemean) { m_Wavemean = Wavemean; }

    //! Set Wavesigma
    void setWavesigma(int Wavesigma) { m_Wavesigma = Wavesigma; }

    //! set Fitter Amplitude Coefficient
    void setSignalPDF(const std::vector<double>& SignalPDF) { m_SignalPDF = SignalPDF;}
    //! set Fitter Timing Coefficient
    void setNoiseCovarianceMatrix(const std::vector<double>& NoiseCovarianceMatrix) { m_NoiseCovarianceMatrix = NoiseCovarianceMatrix;}

    //! Get FPGAversion
    int getFPGAversion() const
    { return m_FPGAversion ; }

    //! Get TCId
    int getTCId() const
    { return m_TCId ; }

    //! Get FAMId
    int getFAMId() const
    { return m_FAMId ; }

    //! Get ChannelId
    int getChannelId() const
    { return m_ChannelId ; }

    //! Get T&E reconstruction method
    int getTEreconstruct() const
    { return m_TEreconstruct ; }

    //! Get ConversionFactor CC
    int getConversionFactor() const
    { return m_ConversionFactor ; }

    //! Get T offset
    int getToffset() const
    { return m_Toffset ; }

    //! Get Threshold
    int getThreshold() const
    { return m_Threshold ; }

    //! Get Wavemean
    int getWavemean() const
    { return m_Wavemean ; }

    //! Get Wavesigma
    int getWavesigma() const
    { return m_Wavesigma ; }
    //! Get Fitter Amplitude Coefficient
    std::vector<double>  setSignalPDF() { return m_SignalPDF ;}
    //! Get Fitter Timing Coefficient
    std::vector<double>  setNoiseCovarianceMatrix() { return m_NoiseCovarianceMatrix ;}


    //  private :

    //! FPGAversion
    int m_FPGAversion;

    //! TCId
    int m_TCId;

    //! FAM ID
    int m_FAMId;
    //! FAM Channel ID
    int m_ChannelId;
    //! T&E reconstruction method
    int m_TEreconstruct;

    //! ConversionFactor CC
    double m_ConversionFactor;

    //! T offset
    int m_Toffset;

    //! Threshold
    int m_Threshold;

    //! Wavemean
    int m_Wavemean;

    //! Wavesigma
    int m_Wavesigma;
    //! Fitter Amplitude Coefficient
    std::vector<double> m_SignalPDF;
    //! Fitter Timing Coefficient
    std::vector<double> m_NoiseCovarianceMatrix;


    //! the class title
    ClassDef(TRGECLFAMPara, 2); /*< the class title */
  };
} //! end namespace Belle2

#endif

