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

using namespace std;

const unsigned int c_nwireCDC = c_nSenseWires;

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
    void setMergePayload(bool value = true) {m_isMerge = value;}

    /**
    * funtion to set flag active for plotting
    */
    void enableExtraPlots(bool value = false) {m_isMakePlots = value;}

    /**
    * funtion to set trucation method (local vs global)
    */
    void setWireBasedTruction(bool value = false) {m_isWireTruc = value;}

    /**
    * function to get extract calibration run/exp
    */
    void getExpRunInfo();

    /**
    * function to finally store new payload after full calibration
    */
    void createPayload(vector<double>& vdedx_tmeans);

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
    void plotLayerDist(array<TH1D*, 2> hdedxL);

    /**
    * function to draw dE/dx histograms for each wire
    */
    void plotWireDist(array<TH1D*, c_nwireCDC> hist, vector<double>& vrel_mean);

    /**
    * function to draw wire gains
    */
    void plotWireGain(vector<double>& vdedx_means, vector<double>& vrel_mean, double layeravg);

    /**
    * function to draw layer gains
    */
    void plotLayerGain(array<double, c_maxNSenseLayers>& layermean, double layeravg);

    /**
    * function to draw WG per layer
    */
    void plotWGPerLayer(vector<double>& vdedx_means, array<double, c_maxNSenseLayers>& layermean, double layeravg);

    /**
    * function to draw statstics
    */
    void plotEventStats();

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

    bool m_isMakePlots; /**< produce plots for status */
    bool m_isMerge; /**< merge payload at the time of calibration */
    bool m_isWireTruc; /**< method of trunc range for mean */

    int m_dedxBins; /**< number of bins for dedx histogram */
    double m_dedxMin; /**< min dedx range for wiregain cal */
    double m_dedxMax; /**< max dedx range for wiregain cal */
    double m_truncMin; /**< min trunc range for mean */
    double m_truncMax; /**< max trunc range for mean */

    string m_suffix; /**< suffix string to seperate plots */

    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxBadWires> m_DBBadWires; /**< Bad wire DB object */
    DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
  };
} // namespace Belle2
