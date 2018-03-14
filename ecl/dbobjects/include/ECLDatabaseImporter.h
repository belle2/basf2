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

    /**
    * Import ECL corrections to showers energies.
    * Temperary - there will be additional improvements done to these corrections in the future.
    * Input file should be .txt file and have the format:
    * generated-energy bkg-scale-factor theta-min theta-max corr-factor.
    * The numbers should be seperated by spaces.
    * For each line, the theta value used is the average of theta-min and theta-max
    */
    void importShowerEnergyCorrectionTemporary();

  private:

    std::vector<std::string> m_inputFileNames; /**< Input file name */
    std::string m_name;  /**< Database object (output) file name */

    /**
     * Extract a root object rootObjName from a root file file. The file is assumed to be valid (pointer valid and not zombie). If rootObjName doesn't exist in file, do B2FATAL.
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
    std::string  m_bgFractionBinNumBranchName = "bgFractionBinNum"; /**< bgFractionBinNum branch name */
    std::string  m_regNumBranchName = "regNum"; /**< regNum branch name */
    std::string  m_phiBinNumBranchName = "phiBinNum"; /**< phiBinNum branch name */
    std::string  m_thetaBinNumBranchName = "thetaBinNum"; /**< thetaBinNum branch name */
    std::string  m_energyBinNumBranchName = "energyBinNum"; /**< energyBinNum branch name */
    std::string  m_correctionFactorBranchName = "correctionFactor"; /**< correctionFactor branch name */

    //"ConstantNtuple" tree branch names
    const int m_numAvgRecEnEntries = 15; /**< Number of entries in avgRecEn array. If this is wrong bad things will happen */
    std::string m_avgRecEnBranchName = "avgRecEn"; /**< avgRecEn branch name */
    std::string m_lReg1ThetaBranchName = "lReg1Theta"; /**< lReg1Theta branch name*/
    std::string m_hReg1ThetaBranchName = "hReg1Theta"; /**< hReg1Theta branch name */
    std::string m_lReg2ThetaBranchName = "lReg2Theta"; /**< lReg2Theta branch name */
    std::string m_hReg2ThetaBranchName = "hReg2Theta"; /**< hReg2Theta branch name */
    std::string m_lReg3ThetaBranchName = "lReg3Theta"; /**< lReg3Theta branch name */
    std::string m_hReg3ThetaBranchName = "hReg3Theta"; /**< hReg3Theta branch name */
    std::string m_numOfBfBinsBranchName = "numOfBfBins"; /**< numOfBfBins branch name */
    std::string m_numOfEnergyBinsBranchName = "numOfEnergyBins"; /**< numOfEnergyBins branch name */
    std::string m_numOfPhiBinsBranchName = "numOfPhiBins"; /**< numOfPhiBins branch name */
    std::string m_numOfReg1ThetaBinsBranchName = "numOfReg1ThetaBins"; /**< numOfReg1ThetaBins branch name */
    std::string m_numOfReg2ThetaBinsBranchName = "numOfReg2ThetaBins"; /**< numOfReg2ThetaBins branch name  */
    std::string m_numOfReg3ThetaBinsBranchName = "numOfReg3ThetaBins"; /**< numOfReg3ThetaBins branch name  */
    std::string m_phiPeriodicityBranchName = "phiPeriodicity"; /**< phiPeriodicity branch name  */
    /**
     * 1: Initial version.
     * 2: Added time calibration.
     * 3: Added Second moment importer.
     * 4: Added shower leakage corrections importer.
     * 5: Added ECLShowerEnergyCorrectionTemporary importer.
     */
    ClassDef(ECLDatabaseImporter, 5);
  };

} // Belle2 namespace
#endif
