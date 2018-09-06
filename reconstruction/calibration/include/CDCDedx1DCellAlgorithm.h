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

#include <reconstruction/dbobjects/CDCDedx1DCell.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

#include <TF1.h>
#include <TLine.h>
#include <TMath.h>
#include <TH1F.h>
#include <TString.h>
#include <TCanvas.h>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx electron: 1D enta cleanup correction
   *
   */
  class CDCDedx1DCellAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedx1DCellAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedx1DCellAlgorithm() {}

    /**<Local vs Global bin map settings for enta*/
    void GlobalToLocalEntaBinMap(Bool_t seeMap);

  protected:

    /**
     * 1D cell algorithm
     */
    virtual EResult calibrate();

    /**<Set etna angle bins, Global */
    void setGlobalEntaBins(int value) {fnEntaBinG = value;}

    /**<Set etna angle bins, locally */
    void setLocalEntaBins(int value) {fnEntaBinL = value;}

    /**<Set asym bins flag to on or off */
    void setAsymmetricBins(bool value) {IsLocalBin = value;}

    /*** funtion to set flag active for plotting*/
    void setMonitoringPlots(bool value = false) {IsMakePlots = value;}

  private:

    /** Save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param dedx              input values
     * @param removeLowest      lowest fraction of hits to remove (0.05)
     * @param removeHighest     highest fraction of hits to remove (0.25)
     */
    int fnEntaBinG; /**<etna angle bins, Global */
    int fnEntaBinL; /**<etna angle bins, Local */

    Double_t feaLE; /**< Lower edge of enta angle */
    Double_t feaUE; /**< Upper edge of enta angle */
    Double_t feaBS; /**< Binwidth edge of enta angle */

    std::vector<int> fEntaBinNums;  /**< Vector for enta asym bin values */
    std::vector<double> fEntaBinValues;  /**< Vector for doca asym bin values */

    bool IsLocalBin;  /**< if local asym bin  */
    bool IsPrintBinMap;  /**< print glocal to loca bin mapping for etna and doca */
    bool IsMakePlots; /**< produce plots for status */

  };
} // namespace Belle2
