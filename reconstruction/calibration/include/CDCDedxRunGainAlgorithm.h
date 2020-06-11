/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jikumar, jvbennett                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <TH1D.h>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx run gains
   *
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


  protected:
    /**
     * Run algorithm
     */
    virtual EResult calibrate() override;


  private:

    bool isMakePlots; /**< produce plots for status */
    bool isMergePayload; /**< merge payload at the of calibration */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */

    double fSigLim = 2.5; /**< fit range limit based on sigma */
    double RunGainAbs = 1.0; /**< calculated Run gain */
  };

} // namespace Belle2
