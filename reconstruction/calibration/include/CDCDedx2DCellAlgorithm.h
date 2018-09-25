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

#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <TF1.h>
#include <TLine.h>
#include <TMath.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TString.h>
#include <TCanvas.h>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx electron 2D enta vs doca correction
   *
   */
  class CDCDedx2DCellAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description the properties and the parameters of the algorithm.
     */
    CDCDedx2DCellAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedx2DCellAlgorithm() {}

    /**
    * Local vs Global bin map settings for doca
    */
    void GlobalToLocalDocaBinMap(Bool_t seeMap);

    /**
    *
    Local vs Global bin map settings for enta
    */
    void GlobalToLocalEntaBinMap(Bool_t seeMap);

    /**
    *Set etna angle bins, Global
    */
    void setGlobalEntaBins(int value) {fnEntaBinG = value;}

    /**
    *Set doca angle bins, Global
    */
    void setGlobalDocaBins(int value) {fnDocaBinG = value;}

    /**
    *Set etna angle bins, locally
    */
    void setLocalEntaBins(int value) {fnEntaBinL = value;}

    /**
    *Set doca angle bins, locally
    */
    void setLocalDocaBins(int value) {fnDocaBinL = value;}

    /**
    *Set asym bins flag to on or off
    */
    void setAsymmetricBins(bool value) {IsLocalBin = value;}

    /**
    funtion to set flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {IsMakePlots = value;}

  protected:

    /**
     * 2D Cell algorithm algorithm
     */
    virtual EResult calibrate();

  private:

    /** Save arithmetic and truncated mean for the 'dedx' values.
     * @param dedx              input values
     * @param removeLowest      lowest fraction of hits to remove (0.05)
     * @param removeHighest     highest fraction of hits to remove (0.25)
     */
    int fnEntaBinG; /**<etna angle bins, Global */
    int fnDocaBinG; /**<doca angle bins, Global */
    int fnEntaBinL; /**<etna angle bins, Local */
    int fnDocaBinL;  /**<doca angle bins, Local */

    Double_t feaLE; /**< Lower edge of enta angle */
    Double_t feaUE; /**< Upper edge of enta angle */
    Double_t feaBS; /**< Binwidth edge of enta angle */

    Double_t fdocaLE; /**< Lower edge of doca */
    Double_t fdocaUE; /**< Upper edge of doca */
    Double_t fdocaBS; /**< Binwidth edge of doca */

    std::vector<int> fDocaBinNums; /**< Vector for enta asym bin # */
    std::vector<double> fDocaBinValues;  /**< Vector for doca asym bin # */

    std::vector<int> fEntaBinNums;  /**< Vector for enta asym bin values */
    std::vector<double> fEntaBinValues;  /**< Vector for doca asym bin values */

    bool IsLocalBin;  /**< if local asym bin  */
    bool IsPrintBinMap;  /**< print glocal to loca bin mapping for etna and doca */
    bool IsMakePlots; /**< produce plots for status */

  };
} // namespace Belle2
