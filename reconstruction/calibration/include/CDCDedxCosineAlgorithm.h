/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <TH1D.h>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
   *
   */
  class CDCDedxCosineAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxCosineAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedxCosineAlgorithm() {}

    /** * function to make flag active for method of sep */
    void setMethodSep(bool value = true) {isMethodSep = value;}

    /**
    * function to decide merge vs relative gains
    */
    void setMergePayload(bool value = true) {isMergePayload = value;}

    /**
    * function to store new payload after full calibration
    */
    void generateNewPayloads(std::vector<double> cosine);


    /** * function to make flag active for plotting */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}

    /** * set sigma to restrict fir range around mean */
    void setFitWidth(double value = 2.5) {fSigLim = value;}

    /** * function to fit histogram in each cosine bin */
    void FitGaussianWRange(TH1D*& temphist, TString& status);

  protected:

    /**
     * Cosine algorithm
     */
    virtual EResult calibrate() override;

  private:

    bool isMethodSep; /**< if e+e- need to be consider sep */
    bool isMakePlots; /**< produce plots for status */
    bool isMergePayload; /**< merge payload at the of calibration */
    double fSigLim = 2.5; /**< gaussian fit sigma limit */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */

  };
} // namespace Belle2
