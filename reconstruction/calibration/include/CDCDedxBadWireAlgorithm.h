/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include "TH1D.h"

#include <framework/database/DBObjPtr.h>
#include <calibration/CalibrationAlgorithm.h>
#include <reconstruction/dbobjects/CDCDedxBadWires.h>

namespace Belle2 {

  /**
   * A calibration algorithm for CDC dE/dx wire gains
   *
   */
  class CDCDedxBadWireAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxBadWireAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedxBadWireAlgorithm() {}

    /**
    * funtion to set dedx bins anv range
    */
    void setDedxValue(int nbin, double min, double max)
    {
      fdedxBin = nbin;
      fdedxMin = min;
      fdedxMax = max;
    }

    /**
    * funtion to set thershold for high dedx fraction (%)
    */
    void setFractionThers(double value) {ffracThers = value;}

    /**
    * funtion to set thershold for high dedx rms
    */
    void setRMSThers(double value) {frmsThers = value;}

    /**
    * funtion to set thershold for high dedx mean
    */
    void setMeanThers(double value) {fmeanThers = value;}

    /**
    * funtion to set flag active for plotting
    */
    void setPlots(bool value = false) {bmakePlots = value;}

    /**
    * funtion to get info about current exp and run
    */
    void getExpRunInfo();


  protected:

    /**
     * badwire algorithm
     */
    virtual EResult calibrate() override;

  private:

    int fdedxBin; /**< number of bins for wirededx */
    double fdedxMin; /**< min dedx range for wirededx */
    double fdedxMax; /**< max dedx range for wirededx */

    double fmeanThers; /**< min hist mean accepted for good wire */
    double frmsThers; /**< min hist rms accepted for good wire */
    double ffracThers; /**< min high-frac accepted for good wire */

    bool bmakePlots; /**< produce plots */
    bool bprintLog; /**< enable logs */
    std::string saddSfx; /**< suffix string to seperate plots */


    const unsigned int nwireCDC; /**< number of wires in CDC */

    DBObjPtr<CDCDedxBadWires> m_DBBadWires; /**< Bad wire DB object */

  };
} // namespace Belle2
