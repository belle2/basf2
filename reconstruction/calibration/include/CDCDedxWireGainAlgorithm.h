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
#include <reconstruction/dbobjects/CDCDedxBadWires.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <string>
#include <vector>
#include "TH1D.h"
#include "TH2F.h"

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
    * funtion to set flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}

    /**
    * funtion to set trucation method (local vs global)
    */
    void setLocalTrucation(bool value = false) {isLTruc = value;}

    /**
    * funtion to set layer scaling
    */
    void setLayerScaling(bool value = false) {isLayerScale = value;}

    /**
    * function to finally store new payload after full calibration
    */
    void generateNewPayloads(std::vector<double> dedxTruncmean);

    /**
    * function to get bins of trunction from histogram
    */
    void getTrucationBins(TH1D* htemp, int& binlow, int& binhigh);

    /**
    * function to plot bad wire status (then and now)
    */
    void plotBadWires(int nDeadwires, int oDeadwires, int Badwires);

    /**
    * function to plot wires in hist with input file
    */
    TH2F* getHistoPattern(TString badFileName, TString suffix);

    /**
    * function to return various CDC indexing for given wire
    */
    double getIndexVal(int iWire, TString what);

    /**
    * function to get layer avg from outer layers
    */
    double getLayerAverage(std::vector<double> tempWire);

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
    bool isLTruc; /**< method of trunc range for mean */
    bool isLayerScale; /**< method of scaling layer avg */
    int fdEdxBins; /**< number of bins for dedx histogram */
    double fdEdxMin; /**< min dedx range for wiregain cal */
    double fdEdxMax; /**< max dedx range for wiregain cal */
    double fTrucMin; /**< min trunc range for mean */
    double fTrucMax; /**< max trunc range for mean */
    int fStartRun; /**< boundary start at this run */
    std::vector<double> flayerAvg; /**< layer wire avg of trun mean */

    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxBadWires> m_DBBadWires; /**< Bad wire DB object */
  };
} // namespace Belle2
