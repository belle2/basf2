/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <TH1D.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TLine.h>
#include <TStyle.h>
#include <TH1I.h>

#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxBadWires.h>
#include <cdc/dbobjects/CDCGeometry.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeometryParConstants.h>


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
    void setMergePayload(bool value = true) {isMerge = value;}

    /**
    * funtion to set flag active for plotting
    */
    void enableExtraPlots(bool value = false) {isMakePlots = value;}

    /**
    * funtion to set trucation method (local vs global)
    */
    void setWireBasedTruction(bool value = false) {isWireTruc = value;}

    /**
    * function to get extract calibration run/exp
    */
    void getExpRunInfo();

    /**
    * function to finally store new payload after full calibration
    */
    void createPayload(std::vector<double> vdedx_tmeans);

    /**
    * function to get bins of trunction from histogram
    */
    void  getTruncatedBins(TH1D* hdedxhit, unsigned int& binlow, unsigned int& binhigh);

    /**
    * function to get mean of trunction from histogram
    */
    double getTruncationMean(TH1D* hdedxhit, int binlow, int binhigh);

    /**
    * function to draw dE/dx for inner/outer layer
    */
    void plotLayerDist(TH1D*& hdedxIL, TH1D*& hdedxOL);

    /**
    * function to draw dE/dx histograms for each wire
    */
    void plotWireDist(TH1D* hist[], std::vector<double>vrel_mean);

    /**
    * function to draw wire gains
    */
    void plotWireGain(std::vector<double>vdedx_means, std::vector<double>vrel_mean, double layeravg);

    /**
    * function to draw layer gains
    */
    void plotLayerGain(std::array<double, 56> layermean, double layeravg);

    /**
    * function to draw WG per layer
    */
    void plotWGPerLayer(std::vector<double>vdedx_means, std::array<double, 56> layermean, double layeravg);

    /**
    * function to draw statstics
    */
    void plotStats();

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

    unsigned int c_nwireCDC; /**< number of wires in CDC */

    bool isMakePlots; /**< produce plots for status */
    bool isMerge; /**< merge payload at the time of calibration */
    bool isWireTruc; /**< method of trunc range for mean */

    int m_dedxBins; /**< number of bins for dedx histogram */
    double m_dedxMin; /**< min dedx range for wiregain cal */
    double m_dedxMax; /**< max dedx range for wiregain cal */
    double m_trucMin; /**< min trunc range for mean */
    double m_trucMax; /**< max trunc range for mean */

    std::string m_suffix; /**< suffix string to seperate plots */

    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxBadWires> m_DBBadWires; /**< Bad wire DB object */
    DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
  };
} // namespace Belle2
