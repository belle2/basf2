/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <TH1D.h>

namespace Belle2 {

  /**
  * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
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

    /**
    * function to make flag active for method of sep
    */
    void setMethodSep(bool value = true) {isMethodSep = value;}

    /**
    * function to decide merge vs relative gains
    */
    void setMergePayload(bool value = true) {isMergePayload = value;}

    /**
    * function to store new payload after full calibration
    */
    void generateNewPayloads(std::vector<double> cosine);

    /**
    * function to make flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}

    /**
    * set sigma to restrict fir range around mean
    */
    void setFitWidth(double value = 2.5) {fSigLim = value;}

    /**
    * function to fit histogram in each cosine bin
    */
    void FitGaussianWRange(TH1D*& temphist, TString& status);

    /**
    * function to set number of cosine bins for calibration
    */
    void setCosineBins(unsigned int value = 100) {fCosbins = value;}

    /**
    * function to set number of cosine bins for calibration
    */
    void setCosineRange(double min = -1.0, double max = 1.0) {fCosMin = min; fCosMax = max;}

    /**
    * function to set nbins of dedx dist calibration
    */
    void setHistBins(int value = 600) {fHistbins = value;}

    /**
    * function to set min/max range of dedx dist calibration
    */
    void setHistRange(double min = 0.0, double max = 3.0) {fdEdxMin = min; fdEdxMax = max;}

  protected:

    /**
    * Cosine algorithm
    */
    virtual EResult calibrate() override;

  private:
    bool isMethodSep; /**< if e+e- need to be consider sep */
    bool isMakePlots; /**< produce plots for status */
    bool isMergePayload; /**< merge payload at the of calibration */
    double fSigLim; /**< gaussian fit sigma limit */
    unsigned int fCosbins; /**< number of bins across cosine range */
    double fCosMin; /**< min cosine angle for cal */
    double fCosMax; /**< max cosine angle for cal */
    int fHistbins; /**< number of bins for dedx histogram */
    double fdEdxMin; /**< min dedx range for gain cal */
    double fdEdxMax; /**< max dedx range for gain cal */
    int fStartRun; /**< boundary start at this run */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
  };
} // namespace Belle2
