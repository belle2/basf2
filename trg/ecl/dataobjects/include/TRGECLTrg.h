//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLTrg.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLTRG_H
#define TRGECLTRG_H


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLTrg : public TObject {

  public:

    //! The cell id of this hit.
    int m_eventId;

    //! Phi Ring Sum (17 rings in total)
    //! theta id 0
    double m_prs01;
    //! theta id 1
    double m_prs02;
    //! theta id 2
    double m_prs03;
    //! theta id 3
    double m_prs04;
    //! theta id 4
    double m_prs05;
    //! theta id 5
    double m_prs06;
    //! theta id 6
    double m_prs07;
    //! theta id 7
    double m_prs08;
    //! theta id 8
    double m_prs09;
    //! theta id 9
    double m_prs10;
    //! theta id 10
    double m_prs11;
    //! theta id 11
    double m_prs12;
    //! theta id 12
    double m_prs13;
    //! theta id 13
    double m_prs14;
    //! theta id 14
    double m_prs15;
    //! theta id 15
    double m_prs16;
    //! theta id 16
    double m_prs17;
    //! Etot : Total Energy Sum
    double m_etot;
    //! Bhabha* (11 phi ring combination in total)
    //! Bhabha cobibation 1
    double m_bhabha01;
    //! Bhabha cobibation 2
    double m_bhabha02;
    //! Bhabha cobibation 3
    double m_bhabha03;
    //! Bhabha cobibation 4
    double m_bhabha04;
    //! Bhabha cobibation 5
    double m_bhabha05;
    //! Bhabha cobibation 6
    double m_bhabha06;
    //! Bhabha cobibation 7
    double m_bhabha07;
    //! Bhabha cobibation 8
    double m_bhabha08;
    //! Bhabha cobibation 9
    double m_bhabha09;
    //! Bhabha cobibation 10
    double m_bhabha10;
    //! Bhabha cobibation 11
    double m_bhabha11;
    //! ICN
    //! Total ICN
    int m_icn;
    //! Barrel ICN
    int m_icn_br;
    //! Forward endcap ICN
    int m_icn_fw;
    //! Backward endcap ICN
    int m_icn_bw;
    //! bit output to GDL
    int  m_bitECLtoGDL;
    //! time bin
    int  m_itimebin;
    //! beambkf veto
    int m_BeamBkgVeto;
    //! max TC Id
    int m_maxTCId;
    //! The number of TC Hit
    int m_NofTCHit;
    //! max TC Energy
    double m_maxTCEnergy;
    //! max TC Timing
    double m_eventtiming;


    //! Set event id
    void setEventId(int eventId) { m_eventId = eventId; }
    //! Set PRM
    void setPRS01(double prs01) { m_prs01 = prs01; }
    //!
    void setPRS02(double prs02) { m_prs02 = prs02; }
    //!
    void setPRS03(double prs03) { m_prs03 = prs03; }
    //!
    void setPRS04(double prs04) { m_prs04 = prs04; }
    //!
    void setPRS05(double prs05) { m_prs05 = prs05; }
    //!
    void setPRS06(double prs06) { m_prs06 = prs06; }
    //!
    void setPRS07(double prs07) { m_prs07 = prs07; }
    //!
    void setPRS08(double prs08) { m_prs08 = prs08; }
    //!
    void setPRS09(double prs09) { m_prs09 = prs09; }
    //!
    void setPRS10(double prs10) { m_prs10 = prs10; }
    //!
    void setPRS11(double prs11) { m_prs11 = prs11; }
    //!
    void setPRS12(double prs12) { m_prs12 = prs12; }
    //!
    void setPRS13(double prs13) { m_prs13 = prs13; }
    //!
    void setPRS14(double prs14) { m_prs14 = prs14; }
    //!
    void setPRS15(double prs15) { m_prs15 = prs15; }
    //!
    void setPRS16(double prs16) { m_prs16 = prs16; }
    //!
    void setPRS17(double prs17) { m_prs17 = prs17; }
    //! Set Etot
    void setEtot(double etot) { m_etot = etot; }
    //! Set Bhabha
    void setBhabha01(double bhabha01) { m_bhabha01 = bhabha01; }
    //!
    void setBhabha02(double bhabha02) { m_bhabha02 = bhabha02; }
    //!
    void setBhabha03(double bhabha03) { m_bhabha03 = bhabha03; }
    //!
    void setBhabha04(double bhabha04) { m_bhabha04 = bhabha04; }
    //!
    void setBhabha05(double bhabha05) { m_bhabha05 = bhabha05; }
    //!
    void setBhabha06(double bhabha06) { m_bhabha06 = bhabha06; }
    //!
    void setBhabha07(double bhabha07) { m_bhabha07 = bhabha07; }
    //!
    void setBhabha08(double bhabha08) { m_bhabha08 = bhabha08; }
    //!
    void setBhabha09(double bhabha09) { m_bhabha09 = bhabha09; }
    //!
    void setBhabha10(double bhabha10) { m_bhabha10 = bhabha10; }
    //!
    void setBhabha11(double bhabha11) { m_bhabha11 = bhabha11; }
    //! Set ICN
    void setICN(int icn) { m_icn = icn; }
    //!
    void setICNFw(int icn_fw) { m_icn_fw = icn_fw; }
    //!
    void setICNBr(int icn_br) { m_icn_br = icn_br; }
    //!
    void setICNBw(int icn_bw) { m_icn_bw = icn_bw; }
    //!

    //! Set bit for GDL
    void setECLtoGDL(int bitECLtoGDL) { m_bitECLtoGDL = bitECLtoGDL; }
    //!
    void setitimebin(int itimebin) { m_itimebin = itimebin; }
    //!
    void setBeamBkgVeto(int BeamBkgVeto) { m_BeamBkgVeto = BeamBkgVeto; }
    //!
    void setEventTiming(double eventtiming) { m_eventtiming = eventtiming; }
    //!
    void setMaxTCId(int maxTCId) { m_maxTCId = maxTCId; }
    //!
    void setMaxTCEnergy(double maxTCEnergy) { m_maxTCEnergy = maxTCEnergy; }
    //!
    void setNofTCHit(double NofTCHit) { m_NofTCHit = NofTCHit; }




//! Empty constructor
    //! Recommended for ROOT IO
    TRGECLTrg()
    {
      m_eventId = 0;
      //!
      m_prs01 = 0;
      m_prs02 = 0;
      m_prs03 = 0;
      m_prs04 = 0;
      m_prs05 = 0;
      m_prs06 = 0;
      m_prs07 = 0;
      m_prs08 = 0;
      m_prs09 = 0;
      m_prs10 = 0;
      m_prs11 = 0;
      m_prs12 = 0;
      m_prs13 = 0;
      m_prs14 = 0;
      m_prs15 = 0;
      m_prs16 = 0;
      m_prs17 = 0;
      //!
      m_etot = 0;
      //!
      m_bhabha01 = 0;
      m_bhabha02 = 0;
      m_bhabha03 = 0;
      m_bhabha04 = 0;
      m_bhabha05 = 0;
      m_bhabha06 = 0;
      m_bhabha07 = 0;
      m_bhabha08 = 0;
      m_bhabha09 = 0;
      m_bhabha10 = 0;
      m_bhabha11 = 0;
      //!
      m_icn = 0;
      m_icn_br = 0;
      m_icn_fw = 0;
      m_icn_bw = 0;
      //!
      m_bitECLtoGDL = 0;
      m_BeamBkgVeto = 0;
      m_eventtiming = 0;
      m_itimebin = 0;
      m_maxTCId = 0;
      m_maxTCEnergy = 0;
      m_NofTCHit = 0;




    }

    //!! Useful Constructor
    TRGECLTrg(
      int eventId,
      //!
      double prs01,
      double prs02,
      double prs03,
      double prs04,
      double prs05,
      double prs06,
      double prs07,
      double prs08,
      double prs09,
      double prs10,
      double prs11,
      double prs12,
      double prs13,
      double prs14,
      double prs15,
      double prs16,
      double prs17,
      //!
      double etot,
      //!
      double bhabha01,
      double bhabha02,
      double bhabha03,
      double bhabha04,
      double bhabha05,
      double bhabha06,
      double bhabha07,
      double bhabha08,
      double bhabha09,
      double bhabha10,
      double bhabha11,
      //!
      int icn,
      int icn_br,
      int icn_fw,
      int icn_bw,
      //!
      int bitECLtoGDL,
      int BeamBkgVeto,
      int itimebin,
      double eventtiming,
      int maxTCId,
      double maxTCEnergy


    )
    {
      m_eventId = eventId;
      //!
      m_prs01 = prs01;
      m_prs02 = prs02;
      m_prs03 = prs03;
      m_prs04 = prs04;
      m_prs05 = prs05;
      m_prs06 = prs06;
      m_prs07 = prs07;
      m_prs08 = prs08;
      m_prs09 = prs09;
      m_prs10 = prs10;
      m_prs11 = prs11;
      m_prs12 = prs12;
      m_prs13 = prs13;
      m_prs14 = prs14;
      m_prs15 = prs15;
      m_prs16 = prs16;
      m_prs17 = prs17;
      //!
      m_etot = etot;
      //!
      m_bhabha01 = bhabha01;
      m_bhabha02 = bhabha02;
      m_bhabha03 = bhabha03;
      m_bhabha04 = bhabha04;
      m_bhabha05 = bhabha05;
      m_bhabha06 = bhabha06;
      m_bhabha07 = bhabha07;
      m_bhabha08 = bhabha08;
      m_bhabha09 = bhabha09;
      m_bhabha10 = bhabha10;
      m_bhabha11 = bhabha11;
      //!
      m_icn = icn;
      m_icn_br = icn_br;
      m_icn_fw = icn_fw;
      m_icn_bw = icn_bw;
      //!
      m_bitECLtoGDL = bitECLtoGDL;
      m_BeamBkgVeto = BeamBkgVeto;
      m_eventtiming = eventtiming;
      m_itimebin = itimebin;
      m_maxTCId = maxTCId;
      m_maxTCEnergy = maxTCEnergy;

    }
    //! the class title
    ClassDef(TRGECLTrg, 1); /*< the class title */

  };

} // end namespace Belle2

#endif
