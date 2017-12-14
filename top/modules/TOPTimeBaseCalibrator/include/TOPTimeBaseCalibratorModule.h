/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Hiromichi Kichimi and Xiaolong Wang        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>

#include <string>
#include <vector>
#include <TMatrixDSym.h>
#include <TProfile.h>
#include <TH1F.h>


namespace Belle2 {

  /**
   * Structure to hold calpulse raw times expressed in samples since sample 0 of window 0.
   */
  struct TwoTimes {
    float t1 = 0; /**< time of the first pulse [samples] */
    float t2 = 0; /**< time of the second pulse [samples] */
    float sigma = 0; /**< uncertainty of time difference (r.m.s.) */
    bool good = false; /**< flag */

    /**
     * Full constructor
     */
    TwoTimes(double time1, double time2, double sig)
    {
      if (time1 < time2) {
        t1 = time1;
        t2 = time2;
      } else {
        t1 = time2;
        t2 = time1;
      }
      sigma = sig;
      good = true;
    }
  };


  /**
   * Time base calibrator
   */
  class TOPTimeBaseCalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPTimeBaseCalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPTimeBaseCalibratorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Sizes
     */
    enum {c_NumChannels = 512, /**< number of channels per module */
          c_NumBoardstacks = 4, /**< number of boardstacks per module */
          c_NumScrodChannels =  c_NumChannels / c_NumBoardstacks,
          c_WindowSize = 64,   /**< samples per window */
          c_TimeAxisSize = c_WindowSize * 4 /**< number of samples to calibrate */
         };

    /**
     * calibrate single channel
     * @param ntuple ntuple data
     * @param scrodID SCROD ID
     * @param scrodChannel channel number within SCROD (0 - 127)
     * @param Hchi2 histogram to store normalized chi^2
     * @param Hndf histogram to store degrees of freedom
     * @param HDeltaT histogram to store fittet double pulse delay
     * @return true on success
     */
    bool calibrateChannel(std::vector<TwoTimes>& ntuple,
                          unsigned scrodID, unsigned scrodChannel,
                          TH1F& Hchi2, TH1F& Hndf, TH1F& HDeltaT);

    /**
     * Method by matrix inversion
     * @param ntuple ntuple data
     * @param scrodID SCROD ID
     * @param scrodChannel channel number within SCROD (0 - 127)
     * @param meanTimeDifference average time difference [samples]
     * @param Hchi2 histogram to store normalized chi^2
     * @param Hndf histogram to store degrees of freedom
     * @param HDeltaT histogram to store fittet double pulse delay
     * @return true on success
     */
    bool matrixInversion(const std::vector<TwoTimes>& ntuple,
                         unsigned scrodID, unsigned scrodChannel,
                         double meanTimeDifference,
                         TH1F& Hchi2, TH1F& Hndf, TH1F& HDeltaT);

    /**
     * Method by iteration of chi2 minimization
     * @param ntuple ntuple data
     * @param scrodID SCROD ID
     * @param scrodChannel channel number within SCROD (0 - 127)
     * @param meanTimeDifference average time difference [samples]
     * @param Hchi2 histogram to store normalized chi^2
     * @param Hndf histogram to store degrees of freedom
     * @param HDeltaT histogram to store fittet double pulse delay
     * @return true on success
     */

    bool iterativeTBC(const std::vector<TwoTimes>& ntuple,
                      unsigned scrodID, unsigned scrodChannel,
                      double meanTimeDifference,
                      TH1F& Hchi2, TH1F& Hndf, TH1F& HDeltaT);

    /**
     * Iteration function called by iterativeTBC()
     * @param ntuple ntuple data
     * @param xval TBC constants of 256 samples, time interval is the
     *  difference of nearby xvals, xval[0]=0 and xval[256]=2*m_syncTimeBase
     */
    void Iteration(const std::vector<TwoTimes>& ntuple, std::vector<double>& xval);

    /**
     * Return the chisqure of one set of TBC constants (xval) in iTBC calculaton
     * @param ntuple ntuple data
     * @param xxval TBC constants of 256 samples, and xxval[0]=0, xxval[256]=2*FTSW
     */
    double Chisq(const std::vector<TwoTimes>& ntuple, std::vector<double>& xxval);

    /**
     * Save vector to histogram and write it out
     * @param vec vector of bin values
     * @param name histogram name
     * @param title histogram title
     * @param xTitle x-axis title
     * @param yTitle y-axis title
     */
    void saveAsHistogram(const std::vector<double>& vec,
                         const std::string& name,
                         const std::string& title,
                         const std::string& xTitle = "",
                         const std::string& yTitle = "") const;

    /**
     * Save vector and errors to histogram and write it out
     * @param vec vector of bin values
     * @param err vector of bin errors
     * @param name histogram name
     * @param title histogram title
     * @param xTitle x-axis title
     * @param yTitle y-axis title
     */
    void saveAsHistogram(const std::vector<double>& vec,
                         const std::vector<double>& err,
                         const std::string& name,
                         const std::string& title,
                         const std::string& xTitle = "",
                         const std::string& yTitle = "") const;

    /**
     * Save matrix to histogram and write it out
     * @param M matrix
     * @param name histogram name
     * @param title histogram title
     */
    void saveAsHistogram(const TMatrixDSym& M,
                         const std::string& name,
                         const std::string& title) const;


    int m_moduleID = 0;    /**< slot ID */
    double m_minTimeDiff = 0;  /**< lower bound for time difference [samples] */
    double m_maxTimeDiff = 0;  /**< upper bound for time difference [samples] */
    unsigned m_minHits = 0;    /**< minimal required hits per channel */
    unsigned m_numIterations = 100;    /**< Number of Iterations of iTBC */
    std::string m_directoryName; /**< directory name for the output root files */
    unsigned m_method = 0; /**< method to use */
    bool m_useFallingEdge = false; /**< if true, use falling edge instead of rising */

    std::vector<TwoTimes> m_ntuples[c_NumChannels]; /**< channel wise data */
    double m_syncTimeBase = 0; /**< synchronization time (two ASIC windows) */
    StoreArray<TOPDigit> m_digits; /**< collection of digits */

    /**
     * parameters for iTBC
     *
     */


    int m_good = 0;   /**<   good events used for chisq cal. */

    double m_dt_min = 20.0; /**<   minimum Delta T of raw calpulse */
    double m_dt_max = 24.0; /**<   maximum Delta T of raw calpulse */
    double m_dev_step = 0.001; /**< a step size to calculate the value of d(chisq)/dxval*/
    double m_xstep = 0.020; /**<   unit for an interation of delta(X_s) */
    double m_dchi2dxv = 0.0;/**< rms of 255 dchi2/dxval values */
    double m_change_xstep = 0.015;/**< update m_xstep if m_dchi2dxv < m_change_step*/
    double m_new_xstep = 2.0 * m_xstep; /**< m_xstep = m_new_xstep if m_dchi2dxv < m_change_step*/
    double m_min_binwidth = 0.05; /**<   minimum time interval of one sample */
    double m_max_binwidth = 2.0; /**<   maximum time interval of one sample */
    double m_dchi2_min = 0.2; /**< quit if chisq increase in iteratons is larger than this value. */
    unsigned   m_conv_iter = 100; /**< Number of iteration with chisq changes less than deltamin. */
    double m_deltamin = 0.01 * 0.01; /**< minumum chisq change in an iteration. */
    double  m_DtSigma = 0.0424; /**< a reference resolution of sigma_0(dT)=42.4 ps from Run3524*/
    double m_sigm2_exp = m_DtSigma * m_DtSigma;/**< (sigma_0(dT))**2 for nomarlization of chisq = sum{dT^2/sigma^2}*/

  };

} // Belle2 namespace

