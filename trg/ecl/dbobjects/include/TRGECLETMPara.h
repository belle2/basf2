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
// Filename : TRGECLETMPara.h
// Section  : TRG ECL
// Owner    : HanEol Cho/Insoo Lee/Yuuji Unno
// Email    : hecho@hep.hanyang.ac.kr/islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLETMPARA_H
#define TRGECLETMPARA_H

#include <TObject.h>

namespace Belle2 {

  //! Raw TC result nefor digitizing
  class TRGECLETMPara : public TObject {
  public:
    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLETMPara() :
      m_FPGAversion(1)
      // m_moduleID(0),

    {
    }
    //! Construction
    TRGECLETMPara(int FPGAversion,
                  double ADCto100MeV,
                  int ELow,
                  int EHigh,
                  int ELum,
                  int FWD2DBhabha[14],
                  int BWD2DBhabha[14],
                  int Bhabha3DSelectionThreshold[2],
                  int Bhabha3DVetoThreshold[2],
                  int Bhabha3DSelectionAngle[4],
                  int Bhabha3DVetoAngle[4],
                  int mumuThreshold,
                  int mumuAngle[4],
                  int LowMultiThreshold[4],
                  int Prescalefactor[3],
                  int TriggerLatency,
                  int ETMDelay,
                  int n300MeVCluster,
                  int ECLBurstThreshold
                 ) :
      m_FPGAversion(FPGAversion), m_ADCto100MeV(ADCto100MeV), m_ELow(ELow), m_EHigh(EHigh), m_ELum(ELum), m_mumuThreshold(mumuThreshold),
      m_TriggerLatency(TriggerLatency), m_ETMDelay(ETMDelay), m_n300MeVCluster(n300MeVCluster), m_ECLBurstThreshold(ECLBurstThreshold)
    {
      for (int index = 0;  index < 14; index++) {
        m_2DBhabhaFWD[index] = FWD2DBhabha[index];
        m_2DBhabhaBWD[index] = BWD2DBhabha[index];
      }
      m_3DBhabhaSelectionThreshold[0] = Bhabha3DSelectionThreshold[0];
      m_3DBhabhaSelectionThreshold[1] = Bhabha3DSelectionThreshold[1];
      m_3DBhabhaVetoThreshold[0] = Bhabha3DVetoThreshold[0];
      m_3DBhabhaVetoThreshold[1] = Bhabha3DVetoThreshold[1];

      m_3DBhabhaSelectionAngle[0] = Bhabha3DSelectionAngle[0];
      m_3DBhabhaSelectionAngle[1] = Bhabha3DSelectionAngle[1];
      m_3DBhabhaSelectionAngle[2] = Bhabha3DSelectionAngle[2];
      m_3DBhabhaSelectionAngle[3] = Bhabha3DSelectionAngle[3];

      m_3DBhabhaVetoAngle[0] = Bhabha3DVetoAngle[0];
      m_3DBhabhaVetoAngle[1] = Bhabha3DVetoAngle[1];
      m_3DBhabhaVetoAngle[2] = Bhabha3DVetoAngle[2];
      m_3DBhabhaVetoAngle[3] = Bhabha3DVetoAngle[3];

      m_Prescalefactor[0] = Prescalefactor[0];
      m_Prescalefactor[1] = Prescalefactor[1];
      m_Prescalefactor[2] = Prescalefactor[2];

      m_mumuAngle[0] = mumuAngle[0];
      m_mumuAngle[1] = mumuAngle[1];
      m_mumuAngle[2] = mumuAngle[2];
      m_mumuAngle[3] = mumuAngle[3];




      m_LowMultiThreshold[0] = LowMultiThreshold[0];
      m_LowMultiThreshold[1] = LowMultiThreshold[1];
      m_LowMultiThreshold[2] = LowMultiThreshold[2];
      m_LowMultiThreshold[3] = LowMultiThreshold[3];


    }


    //! Set FPGAversion
    void setFPGAversion(int FPGAversion) { m_FPGAversion = FPGAversion; }
    //! Set 100 MeV/ADC conversion factor
    void setADCto100MeV(double ADCto100MeV) {m_ADCto100MeV = ADCto100MeV;}
    //! Set Total Energy > 500 MeV
    void setELow(int ELow) {m_ELow = ELow;}
    //! Set Total Energy > 1000 MeV
    void setEHigh(int EHigh) {m_EHigh = EHigh;}
    //! Set Total Energy > 3000 MeV
    void setELum(int ELum) {m_ELum = ELum;}
    //! Set Belle 1 type(2D) Bhabha threshold in forward part (Higher)
    void set2DBhabhaFWD(int i, int FWD2DBhabha[14]) {m_2DBhabhaFWD[i] = FWD2DBhabha[i];}
    //! Set Belle 1 type(2D) Bhabha threshold in backward part (Lower)
    void set2DBhabhaBWD(int i, int BWD2DBhabha[14]) {m_2DBhabhaBWD[i] = BWD2DBhabha[i];}
    //! Set 3D Selection Bhabha Threshold
    void set3DBhabhaSelectionThreshold(int i, int Bhabha3DSelectionThreshold[2]) {m_3DBhabhaSelectionThreshold[i] = Bhabha3DSelectionThreshold[i];}
    //! Set 3D Veto Bhabha Threshold
    void set3DBhabhaVetoThreshold(int i, int Bhabha3DVetoThreshold[2]) {m_3DBhabhaVetoThreshold[i] = Bhabha3DVetoThreshold[i];}
    //! Set 3D Selection Bhabha Angle
    void set3DBhabhaSelectionAngle(int i, int Bhabha3DSelectionAngle[4]) {m_3DBhabhaSelectionAngle[i] = Bhabha3DSelectionAngle[i];}
    //! Set 3D Veto Bhabha Angle
    void set3DBhabhaVetoAngle(int i, int Bhabha3DVetoAngle[4]) {m_3DBhabhaVetoAngle[i] = Bhabha3DVetoAngle[i];}
    //! Set mumu Threshold
    void setmumuThreshold(int mumuThreshold) {m_mumuThreshold = mumuThreshold;}
    //! Set mumu Threshold
    void setmumuAngle(int i, int mumuAngle[4]) {m_mumuAngle[i] = mumuAngle[i];}
    //! Set Bhabha Prescale Factor
    void setPrescalefactor(int i, int Prescalefactor[3]) {m_Prescalefactor[i] = Prescalefactor[i];}

    //! Set Low Multiplicity bit Threshold
    void setLowMultiThreshold(int i, int LowMultiThreshold[4]) {m_LowMultiThreshold[i] = LowMultiThreshold[i];}
    //! Set Letency of Trigger Window (interval btw L1 and ETM TRG )
    void setTriggerLatency(int TriggerLatency) {m_TriggerLatency = TriggerLatency;}
    //! Set Delay of ETM to GDL
    void setETMDelay(int ETMDelay) {m_ETMDelay = ETMDelay;}

    //! Set the number of cluster exceeding 300 MeV.
    void setn300MeVCluster(int n300MeVCluster) {m_n300MeVCluster = n300MeVCluster;}

    //! Set ECL Burst Threshold
    void setECLBurstThreshold(int ECLBurstThreshold) {m_ECLBurstThreshold = ECLBurstThreshold;}



    //! Get FPGAversion
    int getFPGAversion() const
    { return m_FPGAversion ; }
    //! Get 100 MeV/ADC conversion factor
    double getADCto100MeV() const  {return m_ADCto100MeV;}
    //! Get Total Energy > 500 MeV
    int getELow() const {return m_ELow;}
    //! Get Total Energy > 1000 MeV
    int getEHigh() const {return m_EHigh ;}
    //! Get Total Energy > 3000 MeV
    int getELum() const {return m_ELum ;}
    //! Get Belle 1 type(2D) Bhabha threshold in forward part (Higher)
    int get2DBhabhaFWD(int i) const {return m_2DBhabhaFWD[i] ;}
    //! Get Belle 1 type(2D) Bhabha threshold in backward part (Lower)
    int get2DBhabhaBWD(int i) const {return m_2DBhabhaBWD[i] ;}
    //! Get 3D Bhabha Threshold
    int get3DBhabhaSelectionThreshold(int i) const {return m_3DBhabhaSelectionThreshold[i] ;}
    //! Get 3D Bhabha Threshold
    int get3DBhabhaVetoThreshold(int i) const {return m_3DBhabhaVetoThreshold[i] ;}
    //! Get 3D Bhabha Threshold
    int get3DBhabhaSelectionAngle(int i) const {return m_3DBhabhaSelectionAngle[i] ;}
    //! Get 3D Bhabha Threshold
    int get3DBhabhaVetoAngle(int i) const {return m_3DBhabhaVetoAngle[i] ;}
    //! Set mumu Threshold
    int getmumuThreshold() const {return m_mumuThreshold ;}
    //! get mumu Threshold
    int getmumuAngle(int i) const {return m_mumuAngle[i] ;}
    //! get Bhabha Prescale Factor
    int getPrescalefactor(int i) const {return m_Prescalefactor[i] ;}
    //! get the number of cluster exceeding 300 MeV.
    int getn300MeVCluster() const {return m_n300MeVCluster;}
    //! get ECL Burst Threshold
    int getECLBurstThreshold() const {return m_ECLBurstThreshold ;}

    //! Get Low Multiplicity bit Threshold
    int getLowMultiThreshold(int i) const {return m_LowMultiThreshold[i];}
    //! Get Letency of Trigger Window (interval btw L1 and ETM TRG )
    int getTriggerLatency() const {return m_TriggerLatency ;}
    //! Get Delay of ETM to GDL
    int getETMDelay() const {return m_ETMDelay ;}


  private :


    //! FPGAversion
    int m_FPGAversion = 0;
    //! 100 MeV/ADC conversion factor
    double m_ADCto100MeV = 0.0;
    //! Total Energy > 500 MeV
    int m_ELow = 0;
    //! Total Energy > 1000 MeV
    int m_EHigh = 0;
    //! Total Energy > 3000 MeV
    int m_ELum = 0;
    //! Belle 1 type(2D) Bhabha threshold in forward part (Higher)
    int m_2DBhabhaFWD[14] = {0};
    //! Belle 1 type(2D) Bhabha threshold in backward part (Lower)
    int m_2DBhabhaBWD[14] = {0};
    //! 3D Selection Bhabha Threshold
    int m_3DBhabhaSelectionThreshold[2] = {0};
    //! 3D Veto Bhabha Threshold
    int m_3DBhabhaVetoThreshold[2] = {0};
    //! 3D Selection Bhabha Angle Selection
    int m_3DBhabhaSelectionAngle[4] = {0};
    //! 3D Veto Bhabha Angle Selection
    int m_3DBhabhaVetoAngle[4] = {0};
    //! mumu bit Threshold
    int m_mumuThreshold = 0;
    //! mumu bit angle selection
    int m_mumuAngle[4] = {0};
    //! Low Multiplicity bit Threshold
    int m_LowMultiThreshold[4] = {0};
    //! Prescale factor
    int m_Prescalefactor[3] = {0};
    //! Letency of Trigger Window (Interval btw L1 and ETM TRG )
    int m_TriggerLatency = 0;
    //! Delay of ETM to GDL
    int m_ETMDelay = 0;
    //! The number of cluster exceeding 300 MeV.
    int m_n300MeVCluster = 0;
    //! ECL Burst bit Threshold
    int m_ECLBurstThreshold = 0;

    //! the class title
    ClassDef(TRGECLETMPara, 2); /*< the class title */
  };
} //! end namespace Belle2

#endif
