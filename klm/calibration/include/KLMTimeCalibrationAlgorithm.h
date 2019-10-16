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
#include <klm/dbobjects/KLMTimeDetParts.h>
#include <klm/dataobjects/KLMElementNumbers.h>


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
      /** EventT0 for the digit. */
      double   t0;
      /** Particle flying time. */
      double   flyTime;
      /** Recosntruction time respected to the trigger time. */
      double   recTime;
      /** Propagation distance from hit to FEE. */
      double   dist;
      /** Globel position difference between klmHit2d and ExtHit (X)*/
      double   diffDistX;
      /** Globel position difference between klmHit2d and ExtHit (Y)*/
      double   diffDistY;
      /** Globel position difference between klmHit2d and ExtHit (Z)*/
      double   diffDistZ;
      /** Collect energy eV. */
      double   eDep;
      /** Number of photon electron. */
      double   nPE;
      /** Unique channel id Barral and endcap merged. */
      int      channelId;
      /** BKLM RPC flag, used for testing and not necessary */
      bool     inRPC;
      /** If phi and z plane flipped, used for testing and not necessary */
      bool     isFlipped;
      /** Return propagation time + cableDelay time. */
      double   time() { return recTime - flyTime; }
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

    /**
     * If the input is MC sample.
     * Different histogram scale of data and mc
     */
    void isMC()     { m_mc = true; }

    /**
     * Use event T0 as the initial time point or not
     */
    void useEvtT0() { m_useEventT0 = true; }

    /**
     * Set the lower number of hits collected on one sigle strip.
     * If the hit number is lower than the limit, the strip will not be calibrated
     * and set the average value of the calibration constant.
     */
    void setLowerLimit(int counts) { m_lower_limit_counts = counts; }

    /**
     * Save histograms to file
     */
    void saveHist();


  protected:
    // Run algo on data
    virtual EResult calibrate() override;


  private:
    /**
     * data struct used in collector and alogrithm.
     */
    struct Event ev;

    /**
     * TTree obtained from the collector.
     */
    std::shared_ptr<TTree> t_tin;

    /**
     * Container of hit information.
     * the global element number of the strip is used as the key.
     */
    std::map<uint16_t, std::vector<struct Event> > m_evts;

    /** Container of hit information of one sigle strip. */
    std::vector<struct Event> m_evtsChannel;

    /** Calibration flag if the channel has enough hits collected and fitted OK. */
    std::map<uint16_t, int>   m_cFlag;

    /** Shift values of ecah channel. */
    std::map<uint16_t, double> m_timeShift;

    /** Time distribution central value of each channel. */
    std::map<uint16_t, double> m_time_channel;
    /** Time distribution central value Error of each channel. */
    std::map<uint16_t, double> m_etime_channel;

    /** Central value of the global time distribution (BKLM RPC part). */
    double m_time_channelAvg_rpc;
    /** Central value error of the global time distribution (BKLM RPC part). */
    double m_etime_channelAvg_rpc;
    /** Central value of the global time distribution (BKLM scintillator part). */
    double m_time_channelAvg_scint;
    /** Central value error of the global time distribution (BKLM scintillator part). */
    double m_etime_channelAvg_scint;
    /** Central value of the global time distribution (EKLM scintillator part). */
    double m_time_channelAvg_scint_end;
    /** Central value error of the global time distribution (EKLM scintillator part). */
    double m_etime_channelAvg_scint_end;

    /** Lower limit of hits collected for on single channel. */
    int m_lower_limit_counts;

    /** Element number handler */
    const KLMElementNumbers* m_elementNum;
    /** Element number */
    /** BKLM or EKLM */
    int iSub;
    /** Forward or Backward */
    int iFor;
    /** Section number */
    int iSec;
    /** Layer number */
    int iLay;
    /** Plane number */
    int iPla;
    /** Channel number */
    int iStr;

    /**
     * DBObject of time cost on some parts of
     * the detector.
     */
    KLMTimeDetParts* m_timeDetParts;

    /**
     * DBObject of the calibration constant of
     * each channel due to cable decay.
     */
    KLMTimeCableDelay* m_timeCableDelay;

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

    /** Calibration Statistics for each channel **/
    TH1I* h_calibrated;

    /** distance between global and local position **/
    TH1D* h_diff;

    /** Monitor Graphe of calibration constant value of each channel **/
    /** BKLM RPC */
    TGraphErrors* gre_time_channel_rpc;
    /** BKLM Scintillator */
    TGraphErrors* gre_time_channel_scint;
    /** EKLM */
    TGraphErrors* gre_time_channel_scint_end;

    /** Profiles used for effected light speed estimation **/
    /** For BKLM RPC phi plane */
    TProfile* hprf_rpc_phi_effC;
    /** For BKLM RPC z plane */
    TProfile* hprf_rpc_z_effC;
    /** For BKLM scintillator phi plane */
    TProfile* hprf_scint_phi_effC;
    /** For BKLM scintillator z plane */
    TProfile* hprf_scint_z_effC;
    /** For EKLM scintillator phi plane */
    TProfile* hprf_scint_phi_effC_end;
    /** For EKLM scintillator z plane */
    TProfile* hprf_scint_z_effC_end;

    /** Histograms of Global time distribution used for effected light speed estimation **/
    /** BKLM RPC part */
    TH1D* h_time_rpc_tc;
    /** BKLM scintillator part */
    TH1D* h_time_scint_tc;
    /** EKLM part */
    TH1D* h_time_scint_tc_end;

    /** Histograms of Global time distribution before calibration **/
    /** BKLM RPC part */
    TH1D* h_time_rpc;
    /** BKLM scintillator part */
    TH1D* h_time_scint;
    /** EKLM part */
    TH1D* h_time_scint_end;

    /** Histograms of Global time distribution after calibration **/
    /** BKLM RPC part */
    TH1D* hc_time_rpc;
    /** BKLM scintillator part */
    TH1D* hc_time_scint;
    /** EKLM part */
    TH1D* hc_time_scint_end;

    /** Histograms of Time distribution for forward(backward) before calibration **/
    /** BKLM RPC part */
    TH1D* h_timeF_rpc[2];
    /** BKLM scintillator part */
    TH1D* h_timeF_scint[2];
    /** EKLM part */
    TH1D* h_timeF_scint_end[2];

    /** Histograms of Time distribution for forward(backward) after calibration **/
    /** BKLM RPC part */
    TH1D* hc_timeF_rpc[2];
    /** BKLM scintillator part */
    TH1D* hc_timeF_scint[2];
    /** EKLM part */
    TH1D* hc_timeF_scint_end[2];

    /** Histograms of Time dependent on sector for forward(backward) before calibration **/
    /** BKLM RPC part */
    TH2D* h2_timeF_rpc[2];
    /** BKLM scintillator part */
    TH2D* h2_timeF_scint[2];
    /** EKLM part */
    TH2D* h2_timeF_scint_end[2];

    /** Histograms of Time dependent on sector for forward(backward) after calibration **/
    /** BKLM RPC part */
    TH2D* h2c_timeF_rpc[2];
    /** BKLM scintillator part */
    TH2D* h2c_timeF_scint[2];
    /** EKLM part */
    TH2D* h2c_timeF_scint_end[2];

    /** Histograms of Time distribution for sectors before calibration **/
    /** BKLM RPC part */
    TH1D* h_timeFS_rpc[2][8];
    /** BKLM scintillator part */
    TH1D* h_timeFS_scint[2][8];
    /** EKLM part */
    TH1D* h_timeFS_scint_end[2][4];

    /** Histograms of Time distribution for sectors after calibration **/
    /** BKLM RPC part */
    TH1D* hc_timeFS_rpc[2][8];
    /** BKLM scintillator part */
    TH1D* hc_timeFS_scint[2][8];
    /** EKLM part */
    TH1D* hc_timeFS_scint_end[2][4];

    /** Histograms of Time distribution dependent on layer
     * of sectors before calibration
     **/
    /** BKLM part */
    TH2D* h2_timeFS[2][8];
    /** EKLM part */
    TH2D* h2_timeFS_end[2][4];

    /** Histograms of Time distribution dependent on layer
     * of sectors after calibration **/
    /** BKLM part */
    TH2D* h2c_timeFS[2][8];
    /** EKLM part */
    TH2D* h2c_timeFS_end[2][4];

    /** Histograms of Time distribution of one layer before calibration **/
    /** BKLM part */
    TH1D* h_timeFSL[2][8][15];
    /** EKLM part */
    TH1D* h_timeFSL_end[2][4][14];

    /** Histograms of Time distribution of one layer after calibration **/
    /** BKLM part */
    TH1D* hc_timeFSL[2][8][15];
    /** EKLM part */
    TH1D* hc_timeFSL_end[2][4][14];

    /** Histograms of Time distribution of one plane before calibration **/
    /** BKLM part */
    TH1D* h_timeFSLP[2][8][15][2];
    /** EKLM part */
    TH1D* h_timeFSLP_end[2][4][14][2];

    /** Histograms of Time distribution of one plane after calibration */
    /** BKLM part */
    TH1D* hc_timeFSLP[2][8][15][2];
    /** EKLM part */
    TH1D* hc_timeFSLP_end[2][4][14][2];

    /** Histograms of Time distribution dependent on channels before calibration */
    /** BKLM part */
    TH2D* h2_timeFSLP[2][8][15][2];
    /** EKLM part */
    TH2D* h2_timeFSLP_end[2][4][14][2];

    /** Histograms of Time distribution dependent on channels after calibration */
    /** BKLM part */
    TH2D* h2c_timeFSLP[2][8][15][2];
    /** EKLM part */
    TH2D* h2c_timeFSLP_end[2][4][14][2];

    /** Histograms of Time distribution of each channel befor calibration */
    /** BKLM part, used for effected light speed estimation */
    TH1D* h_timeFSLPC_tc[2][8][15][2][54];
    /** BKLM part */
    TH1D* h_timeFSLPC[2][8][15][2][54];
    /** EKLM part, used for effected light speed estimation */
    TH1D* h_timeFSLPC_tc_end[2][4][14][2][75];
    /** EKLM part */
    TH1D* h_timeFSLPC_end[2][4][14][2][75];

    /** Histograms of Time distribution of each channel after calibration */
    /** BKLM part */
    TH1D* hc_timeFSLPC[2][8][15][2][54];
    /** EKLM part */
    TH1D* hc_timeFSLPC_end[2][4][14][2][75];

    /** Formulas used for fitting */
    /** Pol1 function. Effected light speed fitting. */
    TF1* fcn_pol1;
    /** Const function. Global time distribution fitting. */
    TF1* fcn_const;
    /** Gaussian function. Scitillator time ditribution fitting. */
    TF1* fcn_gaus;
    /** Landau function. RPC time ditribution fitting. */
    TF1* fcn_land;

    /** output file. **/
    TFile* m_outFile;
  };
}

