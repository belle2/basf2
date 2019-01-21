/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jikumar, jvbennett                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx run gains
   *
   */
  class CDCDedxRunGainAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxRunGainAlgorithm();
    /**
     * Destructor
     */
    virtual ~CDCDedxRunGainAlgorithm() {}

    /**
    * reading input file and storing values in vectors
    */
    void setBadRunsdatafile(const std::string& fPath, const std::string& fName)
    {

      m_badRunFPath = fPath;
      m_badRunFName = fName;
      isRmBadruns = true;

      printf("INF0: Taking bad run list file from: %s/%s \n", m_badRunFPath.data(), m_badRunFName.data());

      std::ifstream inputfile;
      inputfile.open(Form("%s/%s", m_badRunFPath.data(), m_badRunFName.data()));
      if (inputfile.fail()) {
        printf("%s\n", "input file of bad runs does not exits or corrupted!");
        return;
      }

      int badrun = -999, nBadruns = 0;
      double avgmean = 1.0;
      printf("--- List of Bad runs \n");
      while (true) {
        nBadruns++;
        inputfile >> badrun >> avgmean;
        if (inputfile.eof()) break;
        printf("%d) Bad Run # = %d, Running avg mean = %0.03f\n", nBadruns, badrun, avgmean);
        listofbadruns.push_back(badrun);
        rmeanOfbadrun.push_back(avgmean);
      }
      inputfile.close();
    }

    /**
    * get path of input bad run file
    */
    std::string getBadRunFilePath() const {return m_badRunFPath;}

    /**
    * get name of input bad run file
    */
    std::string getBadRunFileName() const {return m_badRunFName;}

    /**
    * Check if run is listed as bad
    */
    void CheckRunStatus(Int_t irun, bool& status, double& rmean)
    {
      for (unsigned int j = 0; j < listofbadruns.size(); ++j) {
        if (listofbadruns.at(j) == irun) {
          printf("This is bad run %d \n", irun);
          rmean = rmeanOfbadrun.at(j);
          status = true;
          break;
        }
      }
    }

    /**
    * function to make flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}


  protected:

    /**
     * Run algorithm
     */
    virtual EResult calibrate() override;


  private:

    std::string m_badRunFPath; /**< path of bad run file */
    std::string m_badRunFName; /**< name of bad run file */
    bool isRmBadruns; /**< if bad runs consideration */
    bool isMakePlots; /**< produce plots for status */

    std::vector<int> listofbadruns; /**< vector of bad ru list */
    std::vector<double> rmeanOfbadrun; /**< vector of runing avg mean*/


  };
} // namespace Belle2
