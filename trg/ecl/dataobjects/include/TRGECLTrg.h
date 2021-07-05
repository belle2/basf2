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
// Filename : TRGECLTrg.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#pragma once

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLTrg : public TObject {

  public:

    //! Set event id
    void setEventId(int eventId) { m_eventId = eventId; }
    //! Set Phi ring sum thetaid == 01
    void setPRS01(double prs01) { m_prs01 = prs01; }
    //! Set Phi ring sum thetaid == 02
    void setPRS02(double prs02) { m_prs02 = prs02; }
    //! Set Phi ring sum thetaid == 03
    void setPRS03(double prs03) { m_prs03 = prs03; }
    //! Set Phi ring sum thetaid == 04
    void setPRS04(double prs04) { m_prs04 = prs04; }
    //! Set Phi ring sum thetaid == 05
    void setPRS05(double prs05) { m_prs05 = prs05; }
    //! Set Phi ring sum thetaid == 06
    void setPRS06(double prs06) { m_prs06 = prs06; }
    //! Set Phi ring sum thetaid == 07
    void setPRS07(double prs07) { m_prs07 = prs07; }
    //! Set Phi ring sum thetaid == 08
    void setPRS08(double prs08) { m_prs08 = prs08; }
    //! Set Phi ring sum thetaid == 09
    void setPRS09(double prs09) { m_prs09 = prs09; }
    //! Set Phi ring sum thetaid == 10
    void setPRS10(double prs10) { m_prs10 = prs10; }
    //! Set Phi ring sum thetaid == 11
    void setPRS11(double prs11) { m_prs11 = prs11; }
    //! Set Phi ring sum thetaid == 12
    void setPRS12(double prs12) { m_prs12 = prs12; }
    //! Set Phi ring sum thetaid == 13
    void setPRS13(double prs13) { m_prs13 = prs13; }
    //! Set Phi ring sum thetaid == 14
    void setPRS14(double prs14) { m_prs14 = prs14; }
    //! Set Phi ring sum thetaid == 15
    void setPRS15(double prs15) { m_prs15 = prs15; }
    //! Set Phi ring sum thetaid == 16
    void setPRS16(double prs16) { m_prs16 = prs16; }
    //! Set Phi ring sum thetaid == 17
    void setPRS17(double prs17) { m_prs17 = prs17; }
    //! Set Etot (total energy in ThetaID 2-15)
    void setEtot(double etot) { m_etot = etot; }
    //! Set Bhabha combination 01
    void setBhabha01(double bhabha01) { m_bhabha01 = bhabha01; }
    //! Set Bhabha combination 02
    void setBhabha02(double bhabha02) { m_bhabha02 = bhabha02; }
    //! Set Bhabha combination 03
    void setBhabha03(double bhabha03) { m_bhabha03 = bhabha03; }
    //! Set Bhabha combination 04
    void setBhabha04(double bhabha04) { m_bhabha04 = bhabha04; }
    //! Set Bhabha combination 05
    void setBhabha05(double bhabha05) { m_bhabha05 = bhabha05; }
    //! Set Bhabha combination 06
    void setBhabha06(double bhabha06) { m_bhabha06 = bhabha06; }
    //! Set Bhabha combination 07
    void setBhabha07(double bhabha07) { m_bhabha07 = bhabha07; }
    //! Set Bhabha combination 08
    void setBhabha08(double bhabha08) { m_bhabha08 = bhabha08; }
    //! Set Bhabha combination 09
    void setBhabha09(double bhabha09) { m_bhabha09 = bhabha09; }
    //! Set Bhabha combination 10
    void setBhabha10(double bhabha10) { m_bhabha10 = bhabha10; }
    //! Set Bhabha combination 11
    void setBhabha11(double bhabha11) { m_bhabha11 = bhabha11; }
    //! Set Bhabha combination 12
    void setBhabha12(double bhabha12) { m_bhabha12 = bhabha12; }
    //! Set Bhabha combination 13
    void setBhabha13(double bhabha13) { m_bhabha13 = bhabha13; }
    //! Set Bhabha combination 14
    void setBhabha14(double bhabha14) { m_bhabha14 = bhabha14; }
    //! Set Bhabha combination 15
    void setBhabha15(double bhabha15) { m_bhabha15 = bhabha15; }
    //! Set Bhabha combination 16
    void setBhabha16(double bhabha16) { m_bhabha16 = bhabha16; }
    //! Set Bhabha combination 17
    void setBhabha17(double bhabha17) { m_bhabha17 = bhabha17; }
    //! Set Bhabha combination 18
    void setBhabha18(double bhabha18) { m_bhabha18 = bhabha18; }
    //! Set ICN
    void setICN(int icn) { m_icn = icn; }
    //! Set ICN in forward Endcap
    void setICNFw(int icn_fw) { m_icn_fw = icn_fw; }
    //! Set ICN in barrel
    void setICNBr(int icn_br) { m_icn_br = icn_br; }
    //! Set ICN in backward Endcap
    void setICNBw(int icn_bw) { m_icn_bw = icn_bw; }

    //! Set bit for GDL
    void setECLtoGDL(int bitECLtoGDL, int i)
    {
      m_bitECLtoGDL[i] = bitECLtoGDL;
    }
    //! set bin #
    void setitimebin(int itimebin) { m_itimebin = itimebin; }
    //! Set beam background veto
    void setBeamBkgVeto(int BeamBkgVeto) { m_BeamBkgVeto = BeamBkgVeto; }
    //! Set Bhabha veto
    void setBhabhaVeto(int bhabhaveto) { m_BhabhaVeto = bhabhaveto; }
    //! Set Bhabha veto
    void setBhabhaPrescaleBit(int prebit) { m_BhabhaPrescaleBit = prebit; }
    //! Set Event Timing
    void setEventTiming(double eventtiming) { m_eventtiming = eventtiming; }
    //! Set No of TC Hit
    void setNofTCHit(double NofTCHit) { m_NofTCHit = NofTCHit; }
    //! Set No of TC Hit
    void setBrNofTCHit(double BrNofTCHit) { m_BrNofTCHit = BrNofTCHit; }
    //! Set No of TC Hit
    void setFwdNofTCHit(double FwdNofTCHit) { m_FwdNofTCHit = FwdNofTCHit; }
    //! Set No of TC Hit
    void setBwdNofTCHit(double BwdNofTCHit) { m_BwdNofTCHit = BwdNofTCHit; }
    //! Set hit
    void setHit(int hit) { m_hit = hit; }
    //! Set Revoclk
    void setRevoclk(int Revoclk) { m_Revoclk = Revoclk; }
    //! Set TimingSource
    void setTimingSource(int TimingSource) { m_TimingSource = TimingSource; }
    //! Set physics
    void setPhysics(int physics) { m_physics = physics; }
    //! Set 2DBhabha
    void set2DBhabha(int v2DBhabha) { m_2DBhabha = v2DBhabha; }
    //! Set 3DBhabha veto
    void set3DBhabha(int v3DBhabha) { m_3DBhabha = v3DBhabha; }
    //! Set 3DBhabha selection
    // Selection condition of this parameter is loower than "3DBhabha veto"
    // Purpose of this parameter is calibration of ECL(and other detectors)
    void set3DBhabhaSel(int v3DBhabha_sel) { m_3DBhabha_sel = v3DBhabha_sel; }
    //! Set ELow
    void setELow(int ELow) { m_ELow = ELow; }
    //! Set EHihg
    void setEHihg(int EHigh) { m_EHigh = EHigh; }
    //! Set ELum
    void setELum(int ELum) { m_ELum = ELum; }
    //! Set ClusterOverflow
    void setClusterOverflow(int ClusterOverflow) { m_ClusterOverflow = ClusterOverflow; }
    //! Set LowMultiBit
    void setLowMultiBit(int LowMultiBit) { m_LowMultiBit = LowMultiBit; }
    //! Set LowMultiBit
    void setmumuBit(int mumuBit) { m_mumuBit = mumuBit; }
    // Set trigger bit of flag(1bit) whether two clusters statisfy 3D Bhabha veto
    // are in CDCTRG region in theta (="InTrack") or not
    // Default "InTrack" is set to be TC ThetaID=3-15
    void set3DBhabhaVetoInTrackFlag(int v3DBhabhaVetoInTrackFlag)
    {
      m_3DBhabhaVetoInTrackFlag = v3DBhabhaVetoInTrackFlag;
    }
    // Set each TCID(most energetic TC in a cluster) of two clusters of 3D Bhabha veto
    void set3DBhabhaVetoClusterTCId(int v3DBhabhaVetoClusterTCId, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaVetoClusterTCId1 = v3DBhabhaVetoClusterTCId;
      } else {
        m_3DBhabhaVetoClusterTCId2 = v3DBhabhaVetoClusterTCId;
      }
    }
    // Set each cluster energy of two clusters of 3D Bhabha veto (GeV)
    void set3DBhabhaVetoClusterEnergy(double v3DBhabhaVetoClusterEnergy, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaVetoClusterEnergy1 = v3DBhabhaVetoClusterEnergy;
      } else {
        m_3DBhabhaVetoClusterEnergy2 = v3DBhabhaVetoClusterEnergy;
      }
    }
    // Set each cluster timing of two clusters of 3D Bhabha veto (ns)
    void set3DBhabhaVetoClusterTiming(double v3DBhabhaVetoClusterTiming, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaVetoClusterTiming1 = v3DBhabhaVetoClusterTiming;
      } else {
        m_3DBhabhaVetoClusterTiming2 = v3DBhabhaVetoClusterTiming;
      }
    }
    // Set each TC theta ID(most energetic TC in a cluster) of
    // two clusters of 3D Bhabha veto
    void set3DBhabhaVetoClusterThetaId(int v3DBhabhaVetoClusterThetaId, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaVetoClusterThetaId1 = v3DBhabhaVetoClusterThetaId;
      } else {
        m_3DBhabhaVetoClusterThetaId2 = v3DBhabhaVetoClusterThetaId;
      }
    }
    // Set trigger bit(2bits) of flag which shows theta position of clusters
    // of 3DBhabha Selection.
    // flag=0 : one of clusters goes to ThetaID=1
    // flag=1 : one of clusters goes to ThetaID=2
    // flag=2 : one of clusters goes to ThetaID=3
    // flag=3 : none of clusters fly to ThetaID=1-3
    // Based on this flag, pre-scale is applied on GDL to have flat entry of
    // Bhabha event in theta for calibration purpose
    void set3DBhabhaSelectionThetaFlag(int v3DBhabhaSelectionThetaFlag)
    {
      m_3DBhabhaSelectionThetaFlag = v3DBhabhaSelectionThetaFlag;
    }
    // Set each TCID(most energetic TC in a cluster) of two clusters of 3D Bhabha selection
    void set3DBhabhaSelectionClusterTCId(int v3DBhabhaSelectionClusterTCId, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaSelectionClusterTCId1 = v3DBhabhaSelectionClusterTCId;
      } else {
        m_3DBhabhaSelectionClusterTCId2 = v3DBhabhaSelectionClusterTCId;
      }
    }
    // Set each TC theta ID(most energetic TC in a cluster) of
    // two clusters of 3D Bhabha selection
    void set3DBhabhaSelectionClusterThetaId(int v3DBhabhaSelectionClusterThetaId, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaSelectionClusterThetaId1 = v3DBhabhaSelectionClusterThetaId;
      } else {
        m_3DBhabhaSelectionClusterThetaId2 = v3DBhabhaSelectionClusterThetaId;
      }
    }
    // Set each cluster energy of two clusters of 3D Bhabha selection (GeV)
    void set3DBhabhaSelectionClusterEnergy(double v3DBhabhaSelectionClusterEnergy, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaSelectionClusterEnergy1 = v3DBhabhaSelectionClusterEnergy;
      } else {
        m_3DBhabhaSelectionClusterEnergy2 = v3DBhabhaSelectionClusterEnergy;
      }
    }
    // Set each cluster timing of two clusters of 3D Bhabha selection (ns)
    void set3DBhabhaSelectionClusterTiming(double v3DBhabhaSelectionClusterTiming, int idx)
    {
      if (idx == 0) {
        m_3DBhabhaSelectionClusterTiming1 = v3DBhabhaSelectionClusterTiming;
      } else {
        m_3DBhabhaSelectionClusterTiming2 = v3DBhabhaSelectionClusterTiming;
      }
    }
    // Set trigger bit(2 bits) of event timing quality flag for SVD DAQ
    // The timing quality is determined by resolution of event timing
    // based on energy of most energetic TC in an event
    // flag=0 : no event timing
    // flag=1 : event timing with poor timing resolution
    // flag=2 : fine timing
    // flag=3 : super-fine timing
    void setEventTimingQualityFlag(int vEventTimingQualityFlag)
    {
      m_EventTimingQualityFlag = vEventTimingQualityFlag;
    }
    // Set TCId used to determine event timming
    void setEventTimingTCId(int vEventTimingTCId)
    {
      m_EventTimingTCId = vEventTimingTCId;
    }
    // Set ThetaID of TC used to determine event timming
    void setEventTimingTCThetaId(int vEventTimingTCThetaId)
    {
      m_EventTimingTCThetaId = vEventTimingTCThetaId;
    }
    // Set energy of TC used to determine event timming (GeV)
    void setEventTimingTCEnergy(double vEventTimingTCEnergy)
    {
      m_EventTimingTCEnergy = vEventTimingTCEnergy;
    }
    //! Set Etot1to17 (total energy in ThetaID 1-17)
    void setEtot1to17(double etot1to17) { m_etot1to17 = etot1to17; }
    //! Set taub2b bit flag
    void setTaub2bFlag(int taub2bFlag) { m_taub2bFlag = taub2bFlag; }
    //! Set taub2b angle flag
    void setTaub2bAngleFlag(int taub2bAngleFlag) { m_taub2bAngleFlag = taub2bAngleFlag; }
    //! Set taub2b total energy flag (total energy sum cut for taub2b bit)
    void setTaub2bEtotFlag(int taub2bEtotFlag) { m_taub2bEtotFlag = taub2bEtotFlag; }
    //! Set taub2b cluster energy cut flag
    void setTaub2bClusterEFlag(int taub2bClusterEFlag) { m_taub2bClusterEFlag = taub2bClusterEFlag; }
    //! set the number of cluster in all theta region (thetaID=1-17)
    void setNofCluster1to17(int NofCluster1to17) { m_NofCluster1to17 = NofCluster1to17; }
    // Set data clock start timing (ns)
    void setDataClockWindowStartTime(double vDataClockWindowStartTime)
    {
      m_DataClockWindowStartTime = vDataClockWindowStartTime;
    }
    //! Get No of TC Hit
    double getNofTCHit() { return m_NofTCHit; }
    //! Get Etot (total energy in ThetaID 2-15)
    double getEtot() { return m_etot; }
    //! Get Event Timing
    double getEventTiming() { return m_eventtiming; }
    //! Get bits for GDL
    int getECLtoGDL(int iii)
    {
      if (iii < 4) { return m_bitECLtoGDL[iii]; }
      else         { return 0; }
    }
    //! Get 3D Bhabha bit
    int get3DBhabha() { return m_3DBhabha; }


  private:

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
    //! Etot (total energy in ThetaID 2-15)
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
    //! Bhabha cobibation 12
    double m_bhabha12;
    //! Bhabha cobibation 13
    double m_bhabha13;
    //! Bhabha cobibation 14
    double m_bhabha14;
    //! Bhabha cobibation 15
    double m_bhabha15;
    //! Bhabha cobibation 16
    double m_bhabha16;
    //! Bhabha cobibation 17
    double m_bhabha17;
    //! Bhabha cobibation 18
    double m_bhabha18;
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
    int m_bitECLtoGDL[4];
    //! time bin
    int m_itimebin;
    //! beambkf veto
    int m_BeamBkgVeto;
    //! bhabha veto
    int m_BhabhaVeto;
    //! Event Timing
    double m_eventtiming;
    //! The number of TC Hit
    int m_NofTCHit;
    //! The number of TC Hit in Barrel
    int m_BrNofTCHit ;
    //! The number of TC Hit in Forward
    int m_FwdNofTCHit;
    //! The number of TC Hit in Backward
    int m_BwdNofTCHit;
    //! TC Hit or not
    int m_hit ;
    //! Revo clk
    int m_Revoclk;
    //! Timing source
    int m_TimingSource;
    //! Physics trigger
    int m_physics ;
    //! 2D Bhabha
    int m_2DBhabha;
    //! 3D Bhabha for veto
    int m_3DBhabha;
    //! 3D Bhabha for selection
    int m_3DBhabha_sel;
    //! Prescale bit for selection Bhabha
    int m_BhabhaPrescaleBit;
    //! E low
    int m_ELow;
    //! E high
    int m_EHigh;
    //! E lom
    int m_ELum;
    //! Cluster overflow
    int m_ClusterOverflow;
    //! Low multi bit
    int m_LowMultiBit;
    //! mumu bit
    int m_mumuBit;
    // trigger bit of flag(1bit) whether two clusters statisfy 3D Bhabha veto
    // are in CDCTRG region in theta (="InTrack") or not
    int    m_3DBhabhaVetoInTrackFlag;
    // TCIDs of two clusters of 3D Bhabha veto
    int    m_3DBhabhaVetoClusterTCId1;
    int    m_3DBhabhaVetoClusterTCId2;
    // ThetaIds of two clusters of 3D Bhabha veto
    int    m_3DBhabhaVetoClusterThetaId1;
    int    m_3DBhabhaVetoClusterThetaId2;
    // Energies of two clusters of 3D Bhabha veto (GeV)
    double m_3DBhabhaVetoClusterEnergy1;
    double m_3DBhabhaVetoClusterEnergy2;
    // Timings of two clusters of 3D Bhabha veto (ns)
    double m_3DBhabhaVetoClusterTiming1;
    double m_3DBhabhaVetoClusterTiming2;
    // flag which shows theta position of clusters of 3DBhabha Selection.
    int    m_3DBhabhaSelectionThetaFlag;
    // TCIDs of two clusters of 3D Bhabha selection
    int    m_3DBhabhaSelectionClusterTCId1;
    int    m_3DBhabhaSelectionClusterTCId2;
    // ThetaIDs of two clusters used for 3D Bhabha selection
    int    m_3DBhabhaSelectionClusterThetaId1;
    int    m_3DBhabhaSelectionClusterThetaId2;
    // Energies of two clusters used for 3D Bhabha selection (GeV)
    double m_3DBhabhaSelectionClusterEnergy1;
    double m_3DBhabhaSelectionClusterEnergy2;
    // Timings of two clusters used for 3D Bhabha selection (ns)
    double m_3DBhabhaSelectionClusterTiming1;
    double m_3DBhabhaSelectionClusterTiming2;
    // event timing quality flag for SVD DAQ
    int m_EventTimingQualityFlag;
    // TCID of event timing
    int m_EventTimingTCId;
    // TC theta ID of event timing
    int m_EventTimingTCThetaId;
    // Energy of TC of event timing (GeV)
    double m_EventTimingTCEnergy;
    // Total Energy (total energy in ThetaID 1-17)
    double m_etot1to17;
    // taub2b trigger flag
    int m_taub2bFlag = 0;
    // taub2b angle cut flag
    int m_taub2bAngleFlag = 0;
    // taub2b total enrgy cut flag
    int m_taub2bEtotFlag = 0;
    // taub2b cluster energy cut flag
    int m_taub2bClusterEFlag = 0;
    // the number of cluster in all thata region (theta ID=1-17)
    int m_NofCluster1to17 = 0;
    // Start timing of data clock (ns)
    double m_DataClockWindowStartTime;

  public:

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
      m_bhabha12 = 0;
      m_bhabha13 = 0;
      m_bhabha14 = 0;
      m_bhabha15 = 0;
      m_bhabha16 = 0;
      m_bhabha17 = 0;
      m_bhabha18 = 0;
      //!
      m_icn = 0;
      m_icn_br = 0;
      m_icn_fw = 0;
      m_icn_bw = 0;
      //!
      m_bitECLtoGDL[0] = 0;
      m_bitECLtoGDL[1] = 0;
      m_bitECLtoGDL[2] = 0;
      m_bitECLtoGDL[3] = 0;
      m_hit = 0;
      m_Revoclk = 0;
      m_TimingSource = 0;
      m_physics = 0;
      m_2DBhabha = 0;
      m_3DBhabha = 0;
      m_3DBhabha_sel = 0;
      m_BhabhaPrescaleBit = 0;
      m_ELow = 0;
      m_EHigh = 0;
      m_ELum = 0;
      m_ClusterOverflow = 0;
      m_LowMultiBit = 0;
      m_mumuBit = 0;
      m_BeamBkgVeto = 0;
      m_BhabhaVeto = 0;
      m_eventtiming = 0;
      m_itimebin = 0;
      m_NofTCHit = 0;
      m_BrNofTCHit = 0;
      m_FwdNofTCHit = 0;
      m_BwdNofTCHit = 0;
      m_3DBhabhaVetoInTrackFlag = 0;
      m_3DBhabhaVetoClusterTCId1 = 0;
      m_3DBhabhaVetoClusterTCId2 = 0;
      m_3DBhabhaVetoClusterThetaId1 = 0;
      m_3DBhabhaVetoClusterThetaId2 = 0;
      m_3DBhabhaVetoClusterEnergy1 = 0;
      m_3DBhabhaVetoClusterEnergy2 = 0;
      m_3DBhabhaVetoClusterTiming1 = 0;
      m_3DBhabhaVetoClusterTiming2 = 0;
      m_3DBhabhaSelectionThetaFlag = 0;
      m_3DBhabhaSelectionClusterTCId1 = 0;
      m_3DBhabhaSelectionClusterTCId2 = 0;
      m_3DBhabhaSelectionClusterThetaId1 = 0;
      m_3DBhabhaSelectionClusterThetaId2 = 0;
      m_3DBhabhaSelectionClusterEnergy1 = 0;
      m_3DBhabhaSelectionClusterEnergy2 = 0;
      m_3DBhabhaSelectionClusterTiming1 = 0;
      m_3DBhabhaSelectionClusterTiming2 = 0;
      m_EventTimingQualityFlag = 0;
      m_EventTimingTCId = 0;
      m_EventTimingTCThetaId = 0;
      m_EventTimingTCEnergy = 0;
      m_etot1to17 = 0;
      m_taub2bFlag = 0;
      m_taub2bAngleFlag = 0;
      m_taub2bEtotFlag = 0;
      m_taub2bClusterEFlag = 0;
      m_NofCluster1to17 = 0;
      m_DataClockWindowStartTime = 0;
    }
    //! the class title
    ClassDef(TRGECLTrg, 4);

  };

} // end namespace Belle2
