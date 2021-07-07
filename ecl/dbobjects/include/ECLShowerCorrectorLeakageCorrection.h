/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Root
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to hold the information for the ECL shower leakage corrections
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
    ECLShowerCorrectorLeakageCorrection(const std::vector<int>& bgFractionBinNum,
                                        const std::vector<int>& regNum,
                                        const std::vector<int>& phiBinNum,
                                        const std::vector<int>& thetaBinNum,
                                        const std::vector<int>& energyBinNum,
                                        const std::vector<float>& correctionFactor,
                                        const std::vector<float>& avgRecEn,
                                        const std::vector<float>& lReg1Theta,
                                        const std::vector<float>& hReg1Theta,
                                        const std::vector<float>& lReg2Theta,
                                        const std::vector<float>& hReg2Theta,
                                        const std::vector<float>& lReg3Theta,
                                        const std::vector<float>& hReg3Theta,
                                        const std::vector<int>& numOfBfBins,
                                        const std::vector<int>& numOfEnergyBins,
                                        const std::vector<int>& numOfPhiBins,
                                        const std::vector<int>& numOfReg1ThetaBins,
                                        const std::vector<int>& numOfReg2ThetaBins,
                                        const std::vector<int>& numOfReg3ThetaBins,
                                        const std::vector<int>& phiPeriodicity) :
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

    /*! Get background fraction bin numbers
     * @return BgFractionBinNum
     */
    std::vector<int> getBgFractionBinNum() const {return m_bgFractionBinNum;};

    /*! Get region numbers. Region can be 1,2 or 3.
     * @return RegNum
     */
    std::vector<int> getRegNum() const {return m_regNum;};

    /*! Get phi bin numbers
     * @return PhiBinNum
     */
    std::vector<int> getPhiBinNum() const {return m_phiBinNum;};

    /*! Get theta bin numbers
     * @return ThetaBinNum
     */
    std::vector<int> getThetaBinNum() const {return m_thetaBinNum;};

    /*! Get energy bin numbers
     * @return EnergyBinNum
     */
    std::vector<int> getEnergyBinNum() const {return m_energyBinNum;};

    /*! Get correction factors
     * @return CorrectionFactor
     */
    std::vector<float> getCorrectionFactor() const {return m_correctionFactor;};

    /*! Get average reconstructed energy per bin
     * @return AvgRecEn
     */
    std::vector<float> getAvgRecEn() const {return m_avgRecEn;};

    /*! Get lower bound for theta in region 1
     * @return LReg1Theta
     */
    std::vector<float> getLReg1Theta() const {return m_lReg1Theta;};

    /*! Get upper bound for theta in region 1
     * @return HReg1Theta
     */
    std::vector<float> getHReg1Theta() const {return m_hReg1Theta;};

    /*! Get lower bound for theta in region 2
     * @return LReg2Theta
     */
    std::vector<float> getLReg2Theta() const {return m_lReg2Theta;};

    /*! Get upper bound for theta in region 2
     * @return HReg2Theta
     */
    std::vector<float> getHReg2Theta() const {return m_hReg2Theta;};

    /*! Get lower bound for theta in region 3
     * @return LReg3Theta
     */
    std::vector<float> getLReg3Theta() const {return m_lReg3Theta;};

    /*! Get upper bound for theta in region 3
     * @return HReg3Theta
     */
    std::vector<float> getHReg3Theta() const {return m_hReg3Theta;};

    /*! Get number of background fraction bins
     * @return NumOfBfBins
     */
    std::vector<int> getNumOfBfBins() const {return m_numOfBfBins;};

    /*! Get number of energy bins
     * @return NumOfEnergyBins
     */
    std::vector<int> getNumOfEnergyBins() const {return m_numOfEnergyBins;};

    /*! Get number of phi bins
     * @return NumOfPhiBins
     */
    std::vector<int> getNumOfPhiBins()    const {return m_numOfPhiBins;};

    /*! Get number of theta bins in region 1
     * @return NumOfReg1ThetaBins
     */
    std::vector<int> getNumOfReg1ThetaBins() const {return m_numOfReg1ThetaBins;};

    /*! Get number of theta bins in region 2
     * @return NumOfReg2ThetaBins
     */
    std::vector<int> getNumOfReg2ThetaBins() const {return m_numOfReg2ThetaBins;};

    /*! Get number of theta bins in region 3
     * @return NumOfReg3ThetaBins
     */
    std::vector<int> getNumOfReg3ThetaBins() const {return m_numOfReg3ThetaBins;};

    /*! Get phi periodicity - repeating pattern in phi direction.
     * For barrel it is 72.
     * @return PhiPeriodicity
     */
    std::vector<int> getPhiPeriodicity() const {return m_phiPeriodicity;};

  private:
    //"ParameterNtuple" tree
    std::vector<int> m_bgFractionBinNum; /**< Background fraction bin number */
    std::vector<int> m_regNum; /**< Region numbers */
    std::vector<int> m_phiBinNum; /**< Phi bin numbers */
    std::vector<int> m_thetaBinNum; /**< Theta bin numbers */
    std::vector<int> m_energyBinNum; /**< Energy bin numbers */
    std::vector<float> m_correctionFactor; /**< Correction factors */

    //"ConstantNtuple" tree
    std::vector<float> m_avgRecEn; /**< Average reconstructed energy */
    std::vector<float> m_lReg1Theta; /**< Lower bound of theta in region 1 */
    std::vector<float> m_hReg1Theta; /**< Upper bound of theta in region 1 */
    std::vector<float> m_lReg2Theta; /**< Lower bound of theta in region 2 */
    std::vector<float> m_hReg2Theta; /**< Upper bound of theta in region 2 */
    std::vector<float> m_lReg3Theta; /**< Lower bound of theta in region 3 */
    std::vector<float> m_hReg3Theta; /**< Upper bound of theta in region 3 */
    std::vector<int>   m_numOfBfBins; /**< Number of background fraction bins */
    std::vector<int>   m_numOfEnergyBins; /**< Number of energy bins */
    std::vector<int>   m_numOfPhiBins; /**< Number of phi bins */
    std::vector<int>   m_numOfReg1ThetaBins; /**< Number of theta bins in region 1 */
    std::vector<int>   m_numOfReg2ThetaBins; /**< Number of theta bins in region 2 */
    std::vector<int>   m_numOfReg3ThetaBins; /**< Number of theta bins in region 3 */
    std::vector<int>   m_phiPeriodicity; /**< Periodicity in phi - repeating patter in phi direction */

    // 1: Initial version
    ClassDef(ECLShowerCorrectorLeakageCorrection, 1); /**< ClassDef */
  };
} // end namespace Belle2

