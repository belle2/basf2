/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Leakage corrections for ECL showers (N1).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Alon Hershenhorn (hershen@physics.ubc.ca)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//Root
#include <TObject.h>
#include <TTree.h>
#include <TDirectory.h>

namespace Belle2 {

  /**
   * Corrections to the second moment shower shape.
   * The class takes ownership of m_correction and m_helper (i.e. it moves them to m_directory) so is responsible to delete them.
   */

  class ECLShowerCorrectorLeakageCorrection: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLShowerCorrectorLeakageCorrection() {};

    /**
     * Constructor
     */
    ECLShowerCorrectorLeakageCorrection(std::vector<int> bgFractionBinNum,
                                        std::vector<int> regNum,
                                        std::vector<int> phiBinNum,
                                        std::vector<int> thetaBinNum,
                                        std::vector<int> energyBinNum,
                                        std::vector<float> correctionFactor,
                                        std::vector<float> avgRecEn,
                                        std::vector<float> lReg1Theta,
                                        std::vector<float> hReg1Theta,
                                        std::vector<float> lReg2Theta,
                                        std::vector<float> hReg2Theta,
                                        std::vector<float> lReg3Theta,
                                        std::vector<float> hReg3Theta,
                                        std::vector<int> numOfBfBins,
                                        std::vector<int> numOfEnergyBins,
                                        std::vector<int> numOfPhiBins,
                                        std::vector<int> numOfReg1ThetaBins,
                                        std::vector<int> numOfReg2ThetaBins,
                                        std::vector<int> numOfReg3ThetaBins,
                                        std::vector<int> phiPeriodicity) :
      m_bgFractionBinNum(bgFractionBinNum),
      m_regNum(regNum),
      m_phiBinNum(phiBinNum),
      m_thetaBinNum(thetaBinNum),
      m_energyBinNum(energyBinNum),
      m_correctionFactor(correctionFactor),
      m_avgRecEn(avgRecEn),
      m_lReg1Theta(lReg1Theta),
      m_hReg1Theta(hReg1Theta),
      m_lReg2Theta(lReg2Theta),
      m_hReg2Theta(hReg2Theta),
      m_lReg3Theta(lReg3Theta),
      m_hReg3Theta(hReg3Theta),
      m_numOfBfBins(numOfBfBins),
      m_numOfEnergyBins(numOfEnergyBins),
      m_numOfPhiBins(numOfPhiBins),
      m_numOfReg1ThetaBins(numOfReg1ThetaBins),
      m_numOfReg2ThetaBins(numOfReg2ThetaBins),
      m_numOfReg3ThetaBins(numOfReg3ThetaBins),
      m_phiPeriodicity(phiPeriodicity)
    {

    }

    /**
     * Destructor
     *
     */
    ~ECLShowerCorrectorLeakageCorrection()
    {
    }

    std::vector<int> getBgFractionBinNum() const {return m_bgFractionBinNum;};
    std::vector<int> getRegNum() const {return m_regNum;};
    std::vector<int> getPhiBinNum() const {return m_phiBinNum;};
    std::vector<int> getThetaBinNum() const {return m_thetaBinNum;};
    std::vector<int> getEnergyBinNum() const {return m_energyBinNum;};
    std::vector<float> getCorrectionFactor() const {return m_correctionFactor;};
    std::vector<float> getAvgRecEn() const {return m_avgRecEn;};
    std::vector<float> getLReg1Theta() const {return m_lReg1Theta;};
    std::vector<float> getHReg1Theta() const {return m_hReg1Theta;};
    std::vector<float> getLReg2Theta() const {return m_lReg2Theta;};
    std::vector<float> getHReg2Theta() const {return m_hReg2Theta;};
    std::vector<float> getLReg3Theta() const {return m_lReg3Theta;};
    std::vector<float> getHReg3Theta() const {return m_hReg3Theta;};
    std::vector<int> getNumOfBfBins() const {return m_numOfBfBins;};
    std::vector<int> getNumOfEnergyBins() const {return m_numOfEnergyBins;};
    std::vector<int> getNumOfPhiBins()    const {return m_numOfPhiBins;};
    std::vector<int> getNumOfReg1ThetaBins() const {return m_numOfReg1ThetaBins;};
    std::vector<int> getNumOfReg2ThetaBins() const {return m_numOfReg2ThetaBins;};
    std::vector<int> getNumOfReg3ThetaBins() const {return m_numOfReg3ThetaBins;};
    std::vector<int> getPhiPeriodicity() const {return m_phiPeriodicity;};

  private:
    //"ParameterNtuple" tree
    std::vector<int> m_bgFractionBinNum;
    std::vector<int> m_regNum;
    std::vector<int> m_phiBinNum;
    std::vector<int> m_thetaBinNum;
    std::vector<int> m_energyBinNum;
    std::vector<float> m_correctionFactor;

    //"ConstantNtuple" tree
    std::vector<float> m_avgRecEn;
    std::vector<float> m_lReg1Theta;
    std::vector<float> m_hReg1Theta;
    std::vector<float> m_lReg2Theta;
    std::vector<float> m_hReg2Theta;
    std::vector<float> m_lReg3Theta;
    std::vector<float> m_hReg3Theta;
    std::vector<int>    m_numOfBfBins;
    std::vector<int>    m_numOfEnergyBins;
    std::vector<int>    m_numOfPhiBins;
    std::vector<int>    m_numOfReg1ThetaBins;
    std::vector<int>    m_numOfReg2ThetaBins;
    std::vector<int>    m_numOfReg3ThetaBins;
    std::vector<int> m_phiPeriodicity;

    // 1: Initial version
    ClassDef(ECLShowerCorrectorLeakageCorrection, 1); /**< ClassDef */
  };
} // end namespace Belle2

