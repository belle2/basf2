/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

#include <TH1F.h>

#include <iostream>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx electron 2D enta vs doca correction
   *
   */
  class CDCDedx2DCellAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description the properties and the parameters of the algorithm.
     */
    CDCDedx2DCellAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedx2DCellAlgorithm() {}

    /**
    *Set etna angle bins, Global in multiple of 8
    */
    void setGlobalEntaBins(int value) {fnEntaBinG = value;}

    /**
    *Set doca angle bins, Global anything is fine for the moment
    */
    void setGlobalDocaBins(int value) {fnDocaBinG = value;}

    /**
    *Set asym bins flag to on or off
    */
    void setAsymmetricBins(bool value) {IsLocalBin = value;}

    /**
    *Set asym bins flag to on or off
    */
    void setRotationSymmetry(bool value) {IsRS = value;}

    /**
    * adding prefix to filenae for uniqueness in each iter
    */
    void setOutFilePrefix(const std::string& value) {fSetPrefix = value;}

    /**
    funtion to set flag active for plotting
    */
    void setMonitoringPlots(bool value = false) {IsMakePlots = value;}
    /**
    * funtion to set rotation symmetry
    */
    int GetRotationSymmericBin(int nbin, int ibin)
    {

      if (nbin % 4 != 0)return -1;
      int jbin;
      if (ibin <= nbin / 4) jbin = ibin + nbin / 2 ;
      else if (ibin > 3 * nbin / 4) jbin = ibin - nbin / 2 ;
      else jbin = ibin;

      return jbin;
    }

    /**
    * funtion to set variable bins
    */
    void GetVariableBin(int nbin, std::vector<int>& nBinEnta0to100Per)
    {

      if (nbin % 8 != 0) {
        std::cout << "Please select global in multiple of 8 " << std::endl;
        return ;
      }

      int jbin = -1;
      std::vector<int> nBinEnta0to25Per;
      for (int ibin = 0; ibin < nbin / 4; ibin++) {
        if (ibin < nbin / 8) jbin++;
        else if (TMath::Abs(ibin - nbin / 8) % 2 == 0)jbin++;
        nBinEnta0to25Per.push_back(jbin);
      }

      std::vector<int> temp = nBinEnta0to25Per;
      std::reverse(temp.begin(), temp.end());

      std::vector<int> nBinEnta25to50Per; //second half (0 to pi/2)
      for (unsigned int it = 0; it < temp.size(); ++it)nBinEnta25to50Per.push_back(2 * jbin - temp.at(it) + 1);

      std::vector<int> nBinEnta0to50Per = nBinEnta0to25Per;
      nBinEnta0to50Per.insert(nBinEnta0to50Per.end(), nBinEnta25to50Per.begin(), nBinEnta25to50Per.end());

      std::vector<int> nBinEnta50to100Per;
      for (unsigned int it = 0; it < nBinEnta0to50Per.size(); ++it) {
        nBinEnta50to100Per.push_back(nBinEnta0to50Per.at(nBinEnta0to50Per.size() - 1) + nBinEnta0to50Per.at(it) + 1);
      }

      nBinEnta0to100Per = nBinEnta0to50Per;
      nBinEnta0to100Per.insert(nBinEnta0to100Per.end(), nBinEnta50to100Per.begin(), nBinEnta50to100Per.end());

      TH1F* tempEnta = new TH1F("tempEnta", "tempEnta", fnEntaBinG, feaLE, feaUE);
      fEntaBinValues.push_back(tempEnta->GetBinLowEdge(1)); //first and last manual
      for (unsigned int i = 0; i < nBinEnta0to100Per.size() - 1; ++i) {
        if (nBinEnta0to100Per.at(i) < nBinEnta0to100Per.at(i + 1)) {
          double binval = tempEnta->GetBinLowEdge(i + 1) + tempEnta->GetBinWidth(i + 1);
          if (TMath::Abs(binval) < 10e-5)binval = 0; //avoid infinite deep
          fEntaBinValues.push_back(binval);
        } else continue;
      }
      fEntaBinValues.push_back(tempEnta->GetBinLowEdge(fnEntaBinG) + tempEnta->GetBinWidth(fnEntaBinG));
      delete tempEnta;
    }


  protected:

    /**
     * 2D Cell algorithm algorithm
     */
    virtual EResult calibrate() override;

  private:

    /** Save arithmetic and truncated mean for the 'dedx' values.
     * @param dedx              input values
     * @param removeLowest      lowest fraction of hits to remove (0.05)
     * @param removeHighest     highest fraction of hits to remove (0.25)
     */
    int fnEntaBinG; /**<etna angle bins, Global */
    int fnDocaBinG; /**<doca angle bins, Global */
    int fnEntaBinL; /**<etna angle bins, Local */
    int fnDocaBinL;  /**<doca angle bins, Local */

    Double_t feaLE; /**< Lower edge of enta angle */
    Double_t feaUE; /**< Upper edge of enta angle */
    Double_t feaBS; /**< Binwidth edge of enta angle */

    Double_t fdocaLE; /**< Lower edge of doca */
    Double_t fdocaUE; /**< Upper edge of doca */
    Double_t fdocaBS; /**< Binwidth edge of doca */

    std::string fSetPrefix; /**< prefix to filename */
    std::vector<int> fDocaBinNums; /**< Vector for enta asym bin # */
    std::vector<double> fDocaBinValues;  /**< Vector for doca asym bin # */

    std::vector<int> fEntaBinNums;  /**< Vector for enta asym bin values */
    std::vector<double> fEntaBinValues;  /**< Vector for doca asym bin values */

    bool IsLocalBin;  /**< if local variable bin requested  */
    bool IsMakePlots; /**< produce plots for status */
    bool IsRS; /**< if rotation symmtery requested */
  };
} // namespace Belle2
