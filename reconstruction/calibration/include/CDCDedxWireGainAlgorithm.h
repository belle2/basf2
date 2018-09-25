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
    * reading input file and storing bad wire values in vectors
    */
    void setBadWiresdatafile(const std::string& fPath, const std::string& fName)
    {

      m_badWireFPath = fPath;
      m_badWireFName = fName;
      isRmBadwires = true;

      printf("INF0: Taking bad wire list file from: %s/%s \n", m_badWireFPath.data(), m_badWireFName.data());

      std::ifstream inputfile;
      inputfile.open(Form("%s/%s", m_badWireFPath.data(), m_badWireFName.data()));
      if (inputfile.fail()) {
        printf("%s\n", "input file of bad wires does not exits or corrupted!");
      }

      int ibadwire = -999, nBadwires = 0;
      printf("--- List of Bad wires \n");
      while (true) {
        nBadwires++;
        inputfile >> ibadwire;
        if (inputfile.eof()) break;
        printf("%d),  Global Wire ID # = %d\n", nBadwires, ibadwire);
        listofbadwires.push_back(ibadwire);
      }

      std::sort(listofbadwires.begin(), listofbadwires.end());
      inputfile.close();
    }

    /**
    * funtion to set flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}


  protected:

    /**
     * Wire gain algorithm
     */
    virtual EResult calibrate();


  private:

    /** Save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param dedx              input values
     * @param removeLowest      lowest fraction of hits to remove (0.05)
     * @param removeHighest     highest fraction of hits to remove (0.25)
     */
    std::string m_badWireFPath; /**< path of bad wire file */
    std::string m_badWireFName; /**< name of bad wire file */
    bool isRmBadwires; /**< if bad wire consideration */
    bool isMakePlots; /**< produce plots for status */

    std::vector<int> listofbadwires; /**< vector of bad ru list */

  };
} // namespace Belle2
