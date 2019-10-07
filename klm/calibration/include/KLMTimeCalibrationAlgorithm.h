/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jicheng Mei                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <klm/dbobjects/KLMTimeCableDelay.h>
#include <klm/dataobjects/KLMElementNumbers.h>
//#include <bklm/dataobjects/BKLMStatus.h>


class TH1D;
class TH2D;
class TH1I;
class TGraphErrors;
class TF1;
class TProfile;


namespace Belle2 {
  /**
   * KLM time calibration algorithm.
   */
  class KLMTimeCalibrationAlgorithm : public CalibrationAlgorithm {

  public:

    struct Event {
      double   t0;          // eventT0 for the digit.
      double   flyTime;     // particle flying time.
      double   recTime;     // recosntruction time respect the trigger time.
      double   dist;        // propagation distance from hit to FEE.
      double   diffDistX;   // globel position difference between
      double   diffDistY;   // bklmHit2d and ExtHit
      double   diffDistZ;   //
      double   eDep;        // energy eV.
      double   nPE;         // number of photon electron.
      int      channelId;   // unique channel id Barral and endcap combined.
      // BKLM or EKLM local mapping
      // BKLM RPC flag, used for testing and not necessary
      bool     inRPC;
      // If phi and z plane flipped, used for testing and not necessary
      bool     isFlipped;

      double   time() { return recTime - flyTime; } // return propagation time + cableDelay time.
    };

    /**
     * Constructor.
     */
    KLMTimeCalibrationAlgorithm();

    /**
     * Destructor.
     */
    ~KLMTimeCalibrationAlgorithm();

    /**
     * Turn on debug mode (prints histograms and output running log).
     */
    void setDebug() { m_debug = true; }
    void useFit()   { m_fitted = true; }
    void isMC()     { m_mc = true; }
    void useEvtT0() { m_useEventT0 = true; }
    void setLowerLimit(int counts) { m_lower_limit_counts = counts; }
    void saveHist();

  protected:

    // Run algo on data
    virtual EResult calibrate() override;

  private:
    struct Event ev;
    std::shared_ptr<TTree> t_tin;

    std::map<uint16_t, std::vector<struct Event> > m_evts;
    std::vector<struct Event> m_evtsChannel;

    std::map<uint16_t, int>   m_cFlag;
    std::map<uint16_t, double> m_timeShift;

    std::map<uint16_t, double> m_time_channel;
    std::map<uint16_t, double> m_etime_channel;

    double m_time_channelAvg_rpc;
    double m_etime_channelAvg_rpc;
    double m_time_channelAvg_scint;
    double m_etime_channelAvg_scint;
    double m_time_channelAvg_scint_end;
    double m_etime_channelAvg_scint_end;

    int m_lower_limit_counts;
    int iSub;
    int iSet;
    int iSec;
    int iLay;
    int iPla;
    int iStr;

    KLMTimeCableDelay* m_timeCableDelay;
    const KLMElementNumbers* m_elementNum;

    /** Debug mode. */
    bool m_debug;

    /** calibration for scintilltors only */
    bool m_onlySci;

    /** calibration for RPCs only */
    bool m_onlyRpc;

    /** Method Choice Fit or Simple Mean **/
    bool m_fitted;

    /** if MC **/
    bool m_mc;

    /** if use event T0 from CDC **/
    bool m_useEventT0;

    /** Monitor Hists **/
    TH1D* h_diff;
    TH1I* h_calibrated;

    /** Time distribution for each level **/
    TGraphErrors* gre_time_channel_rpc;
    TGraphErrors* gre_time_channel_scint;
    TGraphErrors* gre_time_channel_scint_end;
    TProfile* hprf_rpc_phi_effC;
    TProfile* hprf_rpc_z_effC;
    TProfile* hprf_scint_phi_effC;
    TProfile* hprf_scint_z_effC;
    TProfile* hprf_scint_phi_effC_end;
    TProfile* hprf_scint_z_effC_end;

    TH1D* h_time_rpc_tc;
    TH1D* h_time_scint_tc;
    TH1D* h_time_scint_tc_end;

    TH1D* h_time_rpc;
    TH1D* h_time_scint;
    TH1D* h_time_scint_end;

    TH1D* hc_time_rpc;
    TH1D* hc_time_scint;
    TH1D* hc_time_scint_end;

    TH1D* h_timeF_rpc[2];
    TH1D* h_timeF_scint[2];
    TH1D* h_timeF_scint_end[2];

    TH1D* hc_timeF_rpc[2];
    TH1D* hc_timeF_scint[2];
    TH1D* hc_timeF_scint_end[2];

    TH2D* h2_timeF_rpc[2];
    TH2D* h2_timeF_scint[2];
    TH2D* h2_timeF_scint_end[2];

    TH2D* h2c_timeF_rpc[2];
    TH2D* h2c_timeF_scint[2];
    TH2D* h2c_timeF_scint_end[2];

    TH1D* h_timeFS_rpc[2][8];
    TH1D* h_timeFS_scint[2][8];
    TH1D* h_timeFS_scint_end[2][4];

    TH1D* hc_timeFS_rpc[2][8];
    TH1D* hc_timeFS_scint[2][8];
    TH1D* hc_timeFS_scint_end[2][4];

    TH2D* h2_timeFS[2][8];
    TH2D* h2c_timeFS[2][8];
    TH2D* h2_timeFS_end[2][4];
    TH2D* h2c_timeFS_end[2][4];

    TH1D* h_timeFSL[2][8][15];
    TH1D* hc_timeFSL[2][8][15];
    TH1D* h_timeFSL_end[2][4][14];
    TH1D* hc_timeFSL_end[2][4][14];

    TH1D* h_timeFSLP[2][8][15][2];
    TH1D* hc_timeFSLP[2][8][15][2];
    TH1D* h_timeFSLP_end[2][4][14][2];
    TH1D* hc_timeFSLP_end[2][4][14][2];

    TH2D* h2_timeFSLP[2][8][15][2];
    TH2D* h2c_timeFSLP[2][8][15][2];
    TH2D* h2_timeFSLP_end[2][4][14][2];
    TH2D* h2c_timeFSLP_end[2][4][14][2];

    TH1D* h_timeFSLPC_tc[2][8][15][2][54];
    TH1D* h_timeFSLPC[2][8][15][2][54];
    TH1D* hc_timeFSLPC[2][8][15][2][54];
    TH1D* h_timeFSLPC_tc_end[2][4][14][2][75];
    TH1D* h_timeFSLPC_end[2][4][14][2][75];
    TH1D* hc_timeFSLPC_end[2][4][14][2][75];

    TF1* fcn;
    TF1* fcn_pol1;
    TF1* fcn_const;
    TF1* fcn_gaus;
    TF1* fcn_land;

    TFile* m_outFile;
  };
}

