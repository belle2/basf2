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

#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <calibration/CalibrationAlgorithm.h>
#include <cdc/dataobjects/WireID.h>
#include <framework/database/DBObjPtr.h>
#include <TH1D.h>
#include <TLine.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <TMath.h>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx wire gains
   *
   */
  class CDCDedxWireGainAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxWireGainAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedxWireGainAlgorithm() {}

    /**
    * function to decide merge vs relative gains
    */
    void setMergePayload(bool value = true) {isMergePayload = value;}

    /**
    * function to finally store new payload after full calibration
    */
    void generateNewPayloads(std::vector<double> dedxTruncmean);

    /**
    * funtion to set flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}


  protected:

    /**
     * Wire gain algorithm
     */
    virtual EResult calibrate() override;


  private:

    /** Save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param dedx              input values
     * @param removeLowest      lowest fraction of hits to remove (0.05)
     * @param removeHighest     highest fraction of hits to remove (0.25)
     */
    std::string m_badWireFPath; /**< path of bad wire file */
    std::string m_badWireFName; /**< name of bad wire file */
    bool isMakePlots; /**< produce plots for status */
    bool isMergePayload; /**< merge payload at the of calibration */

    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
  };
} // namespace Belle2
