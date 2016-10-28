/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber    (ferber@physics.ubc.ca)                   *
 *               Alon Hershenhorn (hershen@phsa.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ECLDATABASEIMPORTER_H
#define ECLDATABASEIMPORTER_H

//framework
#include <framework/logging/Logger.h>

//Root
#include <TObject.h>
#include <TGraph.h>
#include <TFile.h>

namespace Belle2 {

  //! ECL database importer.
  /*!
    This module writes data from e.g. a ROOT histogram to database.
  */
  class ECLDatabaseImporter {

  public:

    /**
     * Constructor.
     */
    ECLDatabaseImporter(std::vector<std::string> inputFileNames, std::string m_name);

    /**
     * Destructor.
     */
    virtual ~ECLDatabaseImporter() {};

    /**
     * Import ECL energy calibration constants to the database
     */
    void importDigitEnergyCalibration();

    /**
     * Import ECL time calibration constants to the database
     */
    void importDigitTimeCalibration();

    /**
     * Import ECL shower shape corrections to second moment to the database
     */
    void importShowerShapesSecondMomentCorrections();

    /**
    * Import ECL leakage corrections to showers
    */
    void importShowerCorrectorLeakageCorrections();

  private:

    std::vector<std::string> m_inputFileNames; /**< Input file name */
    std::string m_name;  /**< Database object (output) file name */

    //TODO update comment if this works
    /**
     * Extract a TGraph from file with graphName. The file is assumed to be valid (pointer valid and not zombie). If graphName doesn't exist in file, do B2FATAL.
     */
    template <class rootClass> rootClass getRootObjectFromFile(TFile* file, const std::string& rootObjName) const
    {
      rootClass rootObj = (rootClass)file->Get(rootObjName.data());
      if (!rootObj) {
        std::string filename = file->GetName();
        delete file;
        B2FATAL("Could not find " << rootObjName << " in " << filename);
      }
      return rootObj;
    }


    /**
     * Branch names for shower corrector leakage correction root file.
     */
    //"ParameterNtuple" tree branch names
    std::string  m_bgFractionBinNumBranchName = "bgFractionBinNum";
    std::string  m_regNumBranchName = "regNum";
    std::string  m_phiBinNumBranchName = "phiBinNum";
    std::string  m_thetaBinNumBranchName = "thetaBinNum";
    std::string  m_energyBinNumBranchName = "energyBinNum";
    std::string  m_correctionFactorBranchName = "correctionFactor";

    //"ConstantNtuple" tree branch names
    const int m_numAvgRecEnEntries = 15;
    std::string m_avgRecEnBranchName = "avgRecEn";
    std::string m_lReg1ThetaBranchName = "lReg1Theta";
    std::string m_hReg1ThetaBranchName = "hReg1Theta";
    std::string m_lReg2ThetaBranchName = "lReg2Theta";
    std::string m_hReg2ThetaBranchName = "hReg2Theta";
    std::string m_lReg3ThetaBranchName = "lReg3Theta";
    std::string m_hReg3ThetaBranchName = "hReg3Theta";
    std::string m_numOfBfBinsBranchName = "numOfBfBins";
    std::string m_numOfEnergyBinsBranchName = "numOfEnergyBins";
    std::string m_numOfPhiBinsBranchName = "numOfPhiBins";
    std::string m_numOfReg1ThetaBinsBranchName = "numOfReg1ThetaBins";
    std::string m_numOfReg2ThetaBinsBranchName = "numOfReg2ThetaBins";
    std::string m_numOfReg3ThetaBinsBranchName = "numOfReg3ThetaBins";
    std::string m_phiPeriodicityBranchName = "phiPeriodicity";
    /**
     * 1: Initial version.
     * 2: Added time calibration.
     * 3: Added Second moment importer.
     * 4: Added shower leakage corrections importer.
     */
    ClassDef(ECLDatabaseImporter, 4);
  };

} // Belle2 namespace
#endif
