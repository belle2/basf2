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
#include "TH2F.h"

#include <TPaveText.h>
#include <framework/database/DBObjPtr.h>
#include <calibration/CalibrationAlgorithm.h>
#include <reconstruction/dbobjects/CDCDedxBadWires.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>

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
    * funtion to set dedx histogram param
    */
    void setDedxPars(int nbin, double min, double max)
    {
      fdedxBin = nbin; fdedxMin = min; fdedxMax = max;
    }

    /**
    * funtion to set adc histogram param
    */
    void setADCPars(int nbin, double min, double max)
    {
      fadcBin = nbin; fadcMin = min; fadcMax = max;
    }

    /**
    * funtion to set thershold for high dedx fraction (%)
    */
    void setHighFracThers(double value) {ffracThers = value;}

    /**
    * funtion to set thershold for high dedx rms
    */
    void setRMSThers(double value) {frmsThers = value;}

    /**
    * funtion to set thershold for high dedx mean
    */
    void setMeanThers(double value) {fmeanThers = value;}

    /**
     * function to set flag active to use adc
     */
    void setADC(bool value = false) {isadc = value;}

    /**
    * funtion to get info about current exp and run
    */
    void getExpRunInfo();

    /**
     * function to plot bad wire status (then and now)
     */
    void createBadWireMap(int ndead[2], int nbad[2]);

    /**
     * function to plot wires in hist with input file
     */
    TH2F* getHistoPattern(std::string badFileName, std::string suffix);

    /**
     * function to return various CDC indexing for given wire                                                                                                                                                                                     */
    double getIndexVal(int iWire, std::string what);

    /**
     * function to change text styles                                                                                                                                                                                    */
    void setTextCosmetics(TPaveText*& pt);

  protected:

    /**
     * badwire algorithm
     */
    virtual EResult calibrate() override;

  private:

    const unsigned int nwireCDC; /**< number of wires in CDC */

    int fdedxBin; /**< number of bins for wirededx */
    double fdedxMin; /**< min dedx range for wirededx */
    double fdedxMax; /**< max dedx range for wirededx */

    int fadcBin; /**< number of bins for adc */
    double fadcMin; /**< min dedx range for adc */
    double fadcMax; /**< max dedx range for adc */

    double fmeanThers; /**< min hist mean accepted for good wire */
    double frmsThers; /**< min hist rms accepted for good wire */
    double ffracThers; /**< min high-frac accepted for good wire */

    bool isadc; /**< Use adc for calibration*/
    std::string saddSfx; /**< suffix string to seperate plots */

    DBObjPtr<CDCDedxBadWires> m_DBBadWires; /**< Bad wire DB object */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Bad wire DB object */

  };
} // namespace Belle2
