/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <TH1D.h>

namespace Belle2 {

  /**
   * A calibration algorithm for CDC dE/dx run gains
   */
  class CDCDedxRunGainAlgorithm : public CalibrationAlgorithm {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxRunGainAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxRunGainAlgorithm() {}

    /**
    * function to decide merged vs relative run-gains
    */
    void setMergePayload(bool value = true) {isMergePayload = value;}

    /**
    * function to store new payload after full calibration
    */
    void generateNewPayloads(double RunGainConst, double ExistingRG);

    /**
    * function to hand flag for monitoring plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}

    /**
    * function to perform fit run by run
    */
    void FitGaussianWRange(TH1D*& temphist, TString& status);

    /**
    * function to make flag active for plotting
    */
    void setFitWidth(double value = 2.5) {fSigLim = value;}

    /**
    * function to set dedx hist bins
    */
    void setHistBins(int value = 600) {fdEdxBins = value;}

    /**
    * function to set dedx gain adjustment
    */
    void setAdjustment(double value = 1.0) {fAdjust = value;}

    /**
    * function to set dedx hist range
    */
    void setHistRange(double min = 0.0, double max = 3.0) {fdEdxMin = min; fdEdxMax = max;}

  protected:

    /**
    * Run algorithm
    */
    virtual EResult calibrate() override;

  private:

    bool isMakePlots; /**< produce plots for status */
    bool isMergePayload; /**< merge payload at the of calibration */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */

    TString fsrun; /**< flag to indentify low stats runs */
    double fSigLim; /**< fit range limit based on sigma */
    int fdEdxBins; /**< number of bins for dedx histogram */
    double fdEdxMin; /**< min dedx range for gain cal */
    double fdEdxMax; /**< max dedx range for gain cal */
    double fAdjust; /**< factor to adjust dedx gain  */
  };
} // namespace Belle2
