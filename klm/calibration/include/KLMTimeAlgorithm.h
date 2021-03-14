/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jicheng Mei                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMTimeCableDelay.h>
#include <klm/dbobjects/KLMTimeConstants.h>
#include <klm/eklm/geometry/GeometryData.h>

/* Belle 2 headers. */
#include <calibration/CalibrationAlgorithm.h>

/* ROOT headers. */
#include <Math/MinimizerOptions.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TH1F.h>
#include <TH1I.h>
#include <TH2F.h>
#include <TProfile.h>

namespace Belle2 {

  /**
   * KLM time calibration algorithm.
   */
  class KLMTimeAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Event data.
     */
    struct Event {

      /** EventT0 for the digit. */
      double t0 = 0;

      /** Particle flying time. */
      double flyTime = 0;

      /** Recosntruction time respected to the trigger time. */
      double recTime = 0;

      /** Propagation distance from hit to FEE. */
      double dist = 0;

      /** Global position difference between klmHit2d and ExtHit (X). */
      double diffDistX = 0;

      /** Global position difference between klmHit2d and ExtHit (Y). */
      double diffDistY = 0;

      /** Global position difference between klmHit2d and ExtHit (Z). */
      double diffDistZ = 0;

      /** Collect energy eV. */
      double eDep = 0;

      /** Number of photon electron. */
      double nPE = 0;

      /** Unique channel id Barral and endcap merged. */
      int channelId = 0;

      /** BKLM RPC flag, used for testing and not necessary. */
      bool inRPC = 0;

      /** If phi and z plane flipped, used for testing and not necessary. */
      bool isFlipped = 0;

      /**
       * Get propagation time + cableDelay time.
       */
      double time()
      {
        return recTime - flyTime;
      }

    };

    /**
     * Channel calibration status.
     */
    enum ChannelCalibrationStatus {

      /* Not enough data. */
      c_NotEnoughData = 0,

      /* Failed fit. */
      c_FailedFit = 1,

      /* Successful calibration. */
      c_SuccessfulCalibration = 2,

    };

    /**
     * Constructor.
     */
    KLMTimeAlgorithm();

    /**
     * Destructor.
     */
    ~KLMTimeAlgorithm();

    /**
     * Turn on debug mode (prints histograms and output running log).
     */
    void setDebug()
    {
      m_debug = true;
    }

    /**
     * Set flag indicating whether the input is MC sample. The histogram
     * ranges are different for data and MC. This setting cannot be determined
     * automatically, because the collector output does not contain metadata.
     */
    void setMC(bool mc)
    {
      m_mc = mc;
    }

    /**
     * Use event T0 as the initial time point or not.
     */
    void useEvtT0()
    {
      m_useEventT0 = true;
    }

    /**
     * Set minimal digit number (total).
     */
    void setMinimalDigitNumber(int minimalDigitNumber)
    {
      m_MinimalDigitNumber = minimalDigitNumber;
    }

    /**
     * Set the lower number of hits collected on one sigle strip. If the hit
     * number is lower than the limit, the strip will not be calibrated and
     * set the average value of the calibration constant.
     */
    void setLowerLimit(int counts)
    {
      m_lower_limit_counts = counts;
    }

    /**
     * Save histograms to file.
     */
    void saveHist();

    /**
     * Estimate value of calibration constant for uncalibrated channels.
     * @param[in] klmChannel KLM channel index.
     */
    double esti_timeShift(const KLMChannelIndex& klmChannel);

    /**
     * Tracing avaiable channels with increasing strip number.
     * @param[in] klmChannel KLM channel index.
     */
    std::pair<int, double> tS_upperStrip(const KLMChannelIndex& klmChannel);

    /**
     * Tracing avaiable channels with decreasing strip number.
     * @param[in] klmChannel KLM channel index.
     */
    std::pair<int, double> tS_lowerStrip(const KLMChannelIndex& klmChannel);


  protected:

    /**
     * Run algorithm on data.
     */
    virtual EResult calibrate() override;

  private:

    /**
     * Setup the database.
     */
    void setupDatabase();

    /**
     * Read calibration data.
     * @return
     * CalibrationAlgorithm::c_OK if the amount of data is sufficient,
     * CalibrationAlgorithm::c_NotEnoughData otherwise.
     */
    CalibrationAlgorithm::EResult readCalibrationData();

    /**
     * Create histograms.
     */
    void createHistograms();

    /**
     * Fill profiles of time versus distance.
     *
     * @param[out] profileRpcPhi
     * BKLM RPC phi plane.
     *
     * @param[out] profileRpcZ
     * BKLM RPC z plane.
     *
     * @param[out] profileBKLMScintillatorPhi
     * BKLM scintillator phi plane.
     *
     * @param[out] profileBKLMScintillatorZ
     * BKLM scintillator z plane.
     *
     * @param[out] profileEKLMScintillatorPlane1
     * EKLM scintillator plane1.
     *
     * @param[out] profileEKLMScintillatorPlane2
     * EKLM scintillator plane2.
     *
     * @param[in]  fill2dHistograms
     * Whether to fill 2d histograms.
     */
    void fillTimeDistanceProfiles(
      TProfile* profileRpcPhi, TProfile* profileRpcZ,
      TProfile* profileBKLMScintillatorPhi, TProfile* profileBKLMScintillatorZ,
      TProfile* profileEKLMScintillatorPlane1,
      TProfile* profileEKLMScintillatorPlane2, bool fill2dHistograms);

    /**
     * Two-dimensional fit for individual channels.
     * @param[in]  channels   Channels.
     * @param[out] delay      Delay (ns / cm).
     * @param[out] delayError Delay error.
     */
    void timeDistance2dFit(
      const std::vector< std::pair<uint16_t, unsigned int> > channels,
      double& delay, double& delayError);

    /**
     * Container of hit information.
     * the global element number of the strip is used as the key.
     */
    std::map<uint16_t, std::vector<struct Event> > m_evts;

    /**
     * Calibration flag if the channel has enough hits collected and
     * fitted OK.
     */
    std::map<uint16_t, int> m_cFlag;

    /** Shift values of ecah channel. */
    std::map<uint16_t, double> m_timeShift;

    /** Time distribution central value of each channel. */
    std::map<uint16_t, double> m_time_channel;

    /** Time distribution central value Error of each channel. */
    std::map<uint16_t, double> m_etime_channel;

    /** Lower time boundary for RPC. */
    double m_LowerTimeBoundaryRPC = -10.0;

    /** Upper time boundary for RPC. */
    double m_UpperTimeBoundaryRPC = 10.0;

    /** Lower time boundary for BKLM scintillators. */
    double m_LowerTimeBoundaryScintilltorsBKLM = 20.0;

    /** Upper time boundary for BKLM scintillators. */
    double m_UpperTimeBoundaryScintilltorsBKLM = 70.0;

    /** Lower time boundary for EKLM scintillators. */
    double m_LowerTimeBoundaryScintilltorsEKLM = 20.0;

    /** Upper time boundary for BKLM scintillators. */
    double m_UpperTimeBoundaryScintilltorsEKLM = 70.0;

    /** Lower time boundary for RPC (calibrated data). */
    double m_LowerTimeBoundaryCalibratedRPC = -10.0;

    /** Upper time boundary for RPC (calibrated data). */
    double m_UpperTimeBoundaryCalibratedRPC = 10.0;

    /** Lower time boundary for BKLM scintillators (calibrated data). */
    double m_LowerTimeBoundaryCalibratedScintilltorsBKLM = -20.0;

    /** Upper time boundary for BKLM scintillators (calibrated data). */
    double m_UpperTimeBoundaryCalibratedScintilltorsBKLM = 20.0;

    /** Lower time boundary for EKLM scintillators (calibrated data). */
    double m_LowerTimeBoundaryCalibratedScintilltorsEKLM = -20.0;

    /** Upper time boundary for BKLM scintillators (calibrated data). */
    double m_UpperTimeBoundaryCalibratedScintilltorsEKLM = 20.0;

    /** Central value of the global time distribution (BKLM RPC part). */
    double m_time_channelAvg_rpc = 0.0;

    /** Central value error of the global time distribution (BKLM RPC part). */
    double m_etime_channelAvg_rpc = 0.0;

    /**
     * Central value of the global time distribution (BKLM scintillator part).
     */
    double m_time_channelAvg_scint = 0.0;

    /**
     * Central value error of the global time distribution
     * (BKLM scintillator part).
     */
    double m_etime_channelAvg_scint = 0.0;

    /**
     * Central value of the global time distribution (EKLM scintillator part).
     */
    double m_time_channelAvg_scint_end = 0.0;

    /**
     * Central value error of the global time distribution
     * (EKLM scintillator part).
     */
    double m_etime_channelAvg_scint_end = 0.0;

    /** Minimal digit number (total). */
    int m_MinimalDigitNumber = 100000000;

    /** Lower limit of hits collected for on single channel. */
    int m_lower_limit_counts = 50;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** BKLM geometry data. */
    const bklm::GeometryPar* m_BKLMGeometry = nullptr;

    /** EKLM geometry data. */
    const EKLM::GeometryData* m_EKLMGeometry = nullptr;

    /** KLM ChannelIndex object. */
    KLMChannelIndex m_klmChannels;

    /** Minimization options. */
    ROOT::Math::MinimizerOptions m_minimizerOptions;

    /** DBObject of time cost on some parts of the detector. */
    KLMTimeConstants* m_timeConstants = nullptr;

    /**
     * DBObject of the calibration constant of
     * each channel due to cable decay.
     */
    KLMTimeCableDelay* m_timeCableDelay = nullptr;

    /** Debug mode. */
    bool m_debug = false;

    /** MC or data. */
    bool m_mc = false;

    /** Whether to use event T0 from CDC. */
    bool m_useEventT0 = true;

    /** Calibration statistics for each channel. */
    TH1I* h_calibrated = nullptr;

    /** Distance between global and local position. */
    TH1F* h_diff = nullptr;

    /* Monitor graphs of peak value of time distribution for each channel. */

    /** BKLM RPC. */
    TGraphErrors* gre_time_channel_rpc = nullptr;

    /** BKLM Scintillator. */
    TGraphErrors* gre_time_channel_scint = nullptr;

    /** EKLM. */
    TGraphErrors* gre_time_channel_scint_end = nullptr;

    /* Monitor graphs of calibration constant value of each channel. */

    /** BKLM RPC. */
    TGraph* gr_timeShift_channel_rpc = nullptr;

    /** BKLM scintillator. */
    TGraph* gr_timeShift_channel_scint = nullptr;

    /** EKLM */
    TGraph* gr_timeShift_channel_scint_end = nullptr;

    /* Profiles used for effective light speed estimation. */

    /** For BKLM RPC phi plane. */
    TProfile* m_ProfileRpcPhi = nullptr;

    /** For BKLM RPC z plane. */
    TProfile* m_ProfileRpcZ = nullptr;

    /** For BKLM scintillator phi plane. */
    TProfile* m_ProfileBKLMScintillatorPhi = nullptr;

    /** For BKLM scintillator z plane. */
    TProfile* m_ProfileBKLMScintillatorZ = nullptr;

    /** For EKLM scintillator plane1. */
    TProfile* m_ProfileEKLMScintillatorPlane1 = nullptr;

    /** For EKLM scintillator plane2. */
    TProfile* m_ProfileEKLMScintillatorPlane2 = nullptr;

    /* Profiles of time versus distance (after fit). */

    /** For BKLM RPC phi plane. */
    TProfile* m_Profile2RpcPhi = nullptr;

    /** For BKLM RPC z plane. */
    TProfile* m_Profile2RpcZ = nullptr;

    /** For BKLM scintillator phi plane. */
    TProfile* m_Profile2BKLMScintillatorPhi = nullptr;

    /** For BKLM scintillator z plane. */
    TProfile* m_Profile2BKLMScintillatorZ = nullptr;

    /** For EKLM scintillator plane1. */
    TProfile* m_Profile2EKLMScintillatorPlane1 = nullptr;

    /** For EKLM scintillator plane2. */
    TProfile* m_Profile2EKLMScintillatorPlane2 = nullptr;

    /*
     * Histograms of global time distribution used for effective light speed
     * estimation.
     */

    /** BKLM RPC part. */
    TH1F* h_time_rpc_tc = nullptr;

    /** BKLM scintillator part. */
    TH1F* h_time_scint_tc = nullptr;

    /** EKLM part. */
    TH1F* h_time_scint_tc_end = nullptr;

    /* Histograms of global time distribution before calibration. */

    /** BKLM RPC part. */
    TH1F* h_time_rpc = nullptr;

    /** BKLM scintillator part. */
    TH1F* h_time_scint = nullptr;

    /** EKLM part. */
    TH1F* h_time_scint_end = nullptr;

    /* Histograms of global time distribution after calibration. */

    /** BKLM RPC part. */
    TH1F* hc_time_rpc = nullptr;

    /** BKLM scintillator part. */
    TH1F* hc_time_scint = nullptr;

    /** EKLM part. */
    TH1F* hc_time_scint_end = nullptr;

    /*
     * Histograms of time distribution for forward (backward)
     * before calibration.
     */

    /** BKLM RPC part. */
    TH1F* h_timeF_rpc[2] = {nullptr};

    /** BKLM scintillator part. */
    TH1F* h_timeF_scint[2] = {nullptr};

    /** EKLM part. */
    TH1F* h_timeF_scint_end[2] = {nullptr};

    /*
     * Histograms of time distribution for forward (backward)
     * after calibration.
     */

    /** BKLM RPC part. */
    TH1F* hc_timeF_rpc[2] = {nullptr};

    /** BKLM scintillator part. */
    TH1F* hc_timeF_scint[2] = {nullptr};

    /** EKLM part. */
    TH1F* hc_timeF_scint_end[2] = {nullptr};

    /*
     * Histograms of time dependent on sector for forward (backward)
     * before calibration.
     */

    /** BKLM RPC part. */
    TH2F* h2_timeF_rpc[2] = {nullptr};

    /** BKLM scintillator part. */
    TH2F* h2_timeF_scint[2] = {nullptr};

    /** EKLM part. */
    TH2F* h2_timeF_scint_end[2] = {nullptr};

    /*
     * Histograms of time dependent on sector for forward (backward)
     * after calibration.
     */

    /** BKLM RPC part. */
    TH2F* h2c_timeF_rpc[2] = {nullptr};

    /** BKLM scintillator part. */
    TH2F* h2c_timeF_scint[2] = {nullptr};

    /** EKLM part. */
    TH2F* h2c_timeF_scint_end[2] = {nullptr};

    /* Histograms of time distribution for sectors before calibration. */

    /** BKLM RPC part. */
    TH1F* h_timeFS_rpc[2][8] = {nullptr};

    /** BKLM scintillator part. */
    TH1F* h_timeFS_scint[2][8] = {nullptr};

    /** EKLM part. */
    TH1F* h_timeFS_scint_end[2][4] = {nullptr};

    /* Histograms of time distribution for sectors after calibration. */

    /** BKLM RPC part. */
    TH1F* hc_timeFS_rpc[2][8] = {nullptr};

    /** BKLM scintillator part. */
    TH1F* hc_timeFS_scint[2][8] = {nullptr};

    /** EKLM part. */
    TH1F* hc_timeFS_scint_end[2][4] = {nullptr};

    /*
     * Histograms of time distribution dependent on layer of sectors
     * before calibration.
     */

    /** BKLM part. */
    TH2F* h2_timeFS[2][8] = {nullptr};

    /** EKLM part. */
    TH2F* h2_timeFS_end[2][4] = {nullptr};

    /*
     * Histograms of time distribution dependent on layer of sectors
     * after calibration.
     */

    /** BKLM part. */
    TH2F* h2c_timeFS[2][8] = {nullptr};

    /** EKLM part. */
    TH2F* h2c_timeFS_end[2][4] = {nullptr};

    /* Histograms of time distribution of one layer before calibration. */

    /** BKLM part. */
    TH1F* h_timeFSL[2][8][15] = {nullptr};

    /** EKLM part. */
    TH1F* h_timeFSL_end[2][4][14] = {nullptr};

    /* Histograms of time distribution of one layer after calibration. */

    /** BKLM part. */
    TH1F* hc_timeFSL[2][8][15] = {nullptr};

    /** EKLM part. */
    TH1F* hc_timeFSL_end[2][4][14] = {nullptr};

    /* Histograms of time distribution of one plane before calibration. */

    /** BKLM part. */
    TH1F* h_timeFSLP[2][8][15][2] = {nullptr};

    /** EKLM part. */
    TH1F* h_timeFSLP_end[2][4][14][2] = {nullptr};

    /* Histograms of time distribution of one plane after calibration. */

    /** BKLM part. */
    TH1F* hc_timeFSLP[2][8][15][2] = {nullptr};

    /** EKLM part. */
    TH1F* hc_timeFSLP_end[2][4][14][2] = {nullptr};

    /*
     * Histograms of time distribution dependent on channels
     * before calibration.
     */

    /** BKLM part. */
    TH2F* h2_timeFSLP[2][8][15][2] = {nullptr};

    /** EKLM part. */
    TH2F* h2_timeFSLP_end[2][4][14][2] = {nullptr};

    /*
     * Histograms of time distribution dependent on channels
     * after calibration.
     */

    /** BKLM part. */
    TH2F* h2c_timeFSLP[2][8][15][2] = {nullptr};

    /** EKLM part. */
    TH2F* h2c_timeFSLP_end[2][4][14][2] = {nullptr};

    /* Histograms of time distribution of each channel before calibration. */

    /** BKLM part, used for effective light speed estimation. */
    TH1F* h_timeFSLPC_tc[2][8][15][2][54] = {nullptr};

    /** BKLM part. */
    TH1F* h_timeFSLPC[2][8][15][2][54] = {nullptr};

    /** Two-dimensional distributions of time versus propagation length. */
    TH2F* m_HistTimeLengthBKLM[2][8][15][2][54] = {nullptr};

    /** EKLM part, used for effective light speed estimation. */
    TH1F* h_timeFSLPC_tc_end[2][4][14][2][75] = {nullptr};

    /** EKLM part. */
    TH1F* h_timeFSLPC_end[2][4][14][2][75] = {nullptr};

    /** Two-dimensional distributions of time versus propagation length. */
    TH2F* m_HistTimeLengthEKLM[2][4][14][2][75] = {nullptr};

    /* Histograms of time distribution of each channel after calibration. */

    /** BKLM part. */
    TH1F* hc_timeFSLPC[2][8][15][2][54] = {nullptr};

    /** EKLM part. */
    TH1F* hc_timeFSLPC_end[2][4][14][2][75] = {nullptr};

    /* Formulas used for fitting. */

    /** Pol1 function. Effective light speed fitting. */
    TF1* fcn_pol1 = nullptr;

    /** Const function. Global time distribution fitting. */
    TF1* fcn_const = nullptr;

    /** Gaussian function. Scitillator time ditribution fitting. */
    TF1* fcn_gaus = nullptr;

    /** Landau function. RPC time ditribution fitting. */
    TF1* fcn_land = nullptr;

    /** Output file. */
    TFile* m_outFile = nullptr;

  };

}

