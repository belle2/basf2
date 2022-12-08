/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxBadWires.h>
#include <reconstruction/calibration/CDCDedxBadWireAlgorithm.h>
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
    void setMergePayload(bool value = true) {isMerge = value;}

    /**
    * funtion to set flag active for plotting
    */
    void enableExtraPlots(bool value = false) {isExtPlot = value;}

    /**
    * funtion to set trucation method (local vs global)
    */
    void setWireBasedTruction(bool value = false) {isWireTruc = value;}

    /**
    * funtion to set layer scaling
    */
    void setLayerScale(bool value = false) {isLayerScale = value;}

    /**
    * function to finally store new payload after full calibration
    */
    void createPayload(std::vector<double> vdedx_tmeans);

    /**
    * function to get bins of trunction from histogram
    */
    void getTrucationBins(TH1D* htemp, int& binlow, int& binhigh);

    /**
    * function to get layer avg from outer layers
    */
    double getLayerAverage(std::vector<double> tempWire);

    /**
      * funtion to get info about current exp and run
      */
    void getExpRunInfo();

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

    unsigned int nwireCDC; /**< number of wires in CDC */
    bool isExtPlot; /**< produce plots for status */
    bool isMerge; /**< merge payload at the of calibration */
    bool isWireTruc; /**< method of trunc range for mean */
    bool isLayerScale; /**< method of scaling layer avg */
    int fdEdxBins; /**< number of bins for dedx histogram */
    double fdEdxMin; /**< min dedx range for wiregain cal */
    double fdEdxMax; /**< max dedx range for wiregain cal */
    double fTrucMin; /**< min trunc range for mean */
    double fTrucMax; /**< max trunc range for mean */
    std::string saddSfx; /**< suffix string to seperate plots */

    std::vector<double> vlayerAvg; /**< layer wire avg of trun mean */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxBadWires> m_DBBadWires; /**< Bad wire DB object */
    CDCDedxBadWireAlgorithm* m_CalWireGain; /**< pointer of CDCDedxBadWireAlgorithm */

  };
} // namespace Belle2
