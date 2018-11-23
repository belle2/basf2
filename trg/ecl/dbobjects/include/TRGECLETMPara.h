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
#include <TVector3.h>

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
                  int ADCto100MeV,
                  int ELow,
                  int EHigh,
                  int ELum,
                  int FWD2DBhabha[14],
                  int BWD2DBhabha[14],
                  int Bhabha3DThreshold[2],
                  int LowMultiThreshold[4],
                  int TriggerLatency,
                  int ETMDelay
                 ) :
      m_FPGAversion(FPGAversion), m_ADCto100MeV(ADCto100MeV), m_ELow(ELow), m_EHigh(EHigh), m_ELum(ELum),
      m_TriggerLatency(TriggerLatency), m_ETMDelay(ETMDelay)
    {
      for (int index = 0;  index < 14; index++) {
        m_2DBhabhaFWD[index] = FWD2DBhabha[index];
        m_2DBhabhaBWD[index] = BWD2DBhabha[index];
      }
      m_3DBhabhaThreshold[0] = Bhabha3DThreshold[0];
      m_3DBhabhaThreshold[1] = Bhabha3DThreshold[1];

      m_LowMultiThreshold[0] = LowMultiThreshold[0];
      m_LowMultiThreshold[1] = LowMultiThreshold[1];
      m_LowMultiThreshold[2] = LowMultiThreshold[2];
      m_LowMultiThreshold[3] = LowMultiThreshold[3];


    }


    //! Set FPGAversion
    void setFPGAversion(int FPGAversion) { m_FPGAversion = FPGAversion; }
    //! Set 100 MeV/ADC conversion factor
    void setADCto100MeV(int ADCto100MeV) {m_ADCto100MeV = ADCto100MeV;}
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
    //! Set 3D Bhabha Threshold
    void set3DBhabhaThreshold(int i, int Bhabha3DThreshold[2]) {m_3DBhabhaThreshold[i] = Bhabha3DThreshold[i];}
    //! Set Low Multiplicity bit Threshold
    void setLowMultiThreshold(int i, int LowMultiThreshold[4]) {m_LowMultiThreshold[i] = LowMultiThreshold[i];}
    //! Set Letency of Trigger Window (interval btw L1 and ETM TRG )
    void setTriggerLatency(int TriggerLatency) {m_TriggerLatency = TriggerLatency;}
    //! Set Delay of ETM to GDL
    void setETMDelay(int ETMDelay) {m_ETMDelay = ETMDelay;}


    //! Get FPGAversion
    int getFPGAversion() const
    { return m_FPGAversion ; }
    //! Get 100 MeV/ADC conversion factor
    int getADCto100MeV() const  {return m_ADCto100MeV;}
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
    int get3DBhabhaThreshold(int i) const {return m_3DBhabhaThreshold[i] ;}
    //! Get Low Multiplicity bit Threshold
    int getLowMultiThreshold(int i) const {return m_LowMultiThreshold[i];}
    //! Get Letency of Trigger Window (interval btw L1 and ETM TRG )
    int getTriggerLatency() const {return m_TriggerLatency ;}
    //! Get Delay of ETM to GDL
    int getETMDelay() const {return m_ETMDelay ;}


  private :


    //! FPGAversion
    int m_FPGAversion;
    //! 100 MeV/ADC conversion factor
    int m_ADCto100MeV;
    //! Total Energy > 500 MeV
    int m_ELow;
    //! Total Energy > 1000 MeV
    int m_EHigh;
    //! Total Energy > 3000 MeV
    int m_ELum;
    //! Belle 1 type(2D) Bhabha threshold in forward part (Higher)
    int m_2DBhabhaFWD[14];
    //! Belle 1 type(2D) Bhabha threshold in backward part (Lower)
    int m_2DBhabhaBWD[14];
    //! 3D Bhabha Threshold
    int m_3DBhabhaThreshold[2];
    //! Low Multiplicity bit Threshold
    int m_LowMultiThreshold[4];
    //! Letency of Trigger Window (Interval btw L1 and ETM TRG )
    int m_TriggerLatency;
    //! Delay of ETM to GDL
    int m_ETMDelay;

    /* //! ETM module ID */
    /* int m_module_ID; */

    //! the class title
    ClassDef(TRGECLETMPara, 1); /*< the class title */
  };
} //! end namespace Belle2

#endif
