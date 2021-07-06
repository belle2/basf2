/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Class to store the result of the TOP LL scan (output of TOPLLScanner).
   */

  class TOPLikelihoodScanResult : public RelationsObject {

  public:

    /**
     * default constructor
     */
    TOPLikelihoodScanResult()
    {}

    /**
     * Sets other data members for a given particle hypothesis
     */
    void set(float mostLikelyMass,
             float mostLikelyMassIntervalLow,
             float mostLikelyMassIntervalUp,
             float threshold,
             float mostLikelySignalPhotonCount,
             float mostLikelyBackgroundPhotonCount,
             float mostLikelyDeltaPhotonCount,
             const std::vector<float>& coarseScanMassPoints,
             const std::vector<float>& fineScanMassPoints,
             const std::vector<float>& coarseScanLLValues,
             const std::vector<float>& fineScanLLValues,
             const std::vector<float>& coarseScanExpectedPhotons,
             const std::vector<float>& fineScanExpectedPhotons)
    {
      m_mostLikelyMass =  mostLikelyMass;
      m_mostLikelyMassIntervalLow =  mostLikelyMassIntervalLow;
      m_mostLikelyMassIntervalUp =  mostLikelyMassIntervalUp;
      m_threshold = threshold;
      m_mostLikelySignalPhotonCount =  mostLikelySignalPhotonCount;
      m_mostLikelyBackgroundPhotonCount =  mostLikelyBackgroundPhotonCount;
      m_mostLikelyDeltaPhotonCount =  mostLikelyDeltaPhotonCount;
      m_coarseScanMassPoints =  coarseScanMassPoints;
      m_fineScanMassPoints = fineScanMassPoints;
      m_coarseScanLLValues = coarseScanLLValues;
      m_fineScanLLValues = fineScanLLValues;
      m_coarseScanExpectedPhotons = coarseScanExpectedPhotons;
      m_fineScanExpectedPhotons = fineScanExpectedPhotons;
    }

    /**
     * Return the most likely mass form the scan
     * @return mostLikelyMass the location of the LL scan maximum
     */
    float getMostLikelyMass() const {return m_mostLikelyMass;}

    /**
     * Return the lower limit of the mass interval
     * @return mostLikelyMass the location of the LL scan maximum
     */
    float getMostLikelyMassIntervalLow() const {return m_mostLikelyMassIntervalLow;}

    /**
     * Return the upper limit of the mass interval
     * @return mostLikelyMass the location of the LL scan maximum
     */
    float getMostLikelyMassIntervalUp() const {return m_mostLikelyMassIntervalUp;}

    /**
     * Return the position of the Cherenkov threshold from the LL scan
     * @return threshold the Cherenkov threshold
     */
    float getThreshold() const {return m_threshold;}


    /**
     * Return the number of expected photons at the LL maximum
     * @return mostLikelyPhotonCount the expected number of photons at the LL maximum
     */
    float getMostLikelySignalPhotonCount() const {return m_mostLikelySignalPhotonCount;}


    /**
      * Return the number of expected photons at the LL maximum
      * @return mostLikelyPhotonCount the expected number of photons at the LL maximum
      */
    float getMostLikelyBackgroundPhotonCount() const {return m_mostLikelyBackgroundPhotonCount;}


    /**
     * Return the number of expected photons at the LL maximum
     * @return mostLikelyPhotonCount the expected number of photons at the LL maximum
     */
    float getMostLikelyDeltaPhotonCount() const {return m_mostLikelyDeltaPhotonCount;}


    /**
     * Return a std::vector containing the mass points used for the coarse LL scane
     * @return coarseScanMassPoints the scan points
     */
    std::vector<float> getCoarseScanMassPoints() const {return m_coarseScanMassPoints;}

    /**
     * Return a std::vector containing the mass points used for the fine LL scane
     * @return fineScanMassPoints the scan points
     */
    std::vector<float> getFineScanMassPoints() const {return m_fineScanMassPoints;}


    /**
     * Return a std::vector containing the LL values of the coarse scan
     * @return coarseScanLLValues the scan points
     */
    std::vector<float> getCoarseScanLLValues() const {return m_coarseScanLLValues;}

    /**
     * Return a std::vector containing the LL values of the fine scan
     * @return fineScanLLValues the scan points
     */
    std::vector<float> getFineScanLLValues() const {return m_fineScanLLValues;}

    /**
     * Return a std::vector containing the number of expected photons of the coarse scan
     * @return coarseScanExpectedPhotons the scan points
     */
    std::vector<float> getCoarseScanExpectedPhotons() const {return m_coarseScanExpectedPhotons;}

    /**
     * Return a std::vector containing the number of expected photons of the fine scan
     * @return fineScanExpectedPhotons the scan points
     */
    std::vector<float> getFineScanExpectedPhotons() const {return m_fineScanExpectedPhotons;}

  private:
    float m_mostLikelyMass = 0;     /**< mass that maximizes the LL  */
    float m_mostLikelyMassIntervalLow = 0;     /**< lower edge of the mass interval from  the LL  */
    float m_mostLikelyMassIntervalUp = 0;     /**< upper edge of the mass interval from the LL  */
    float m_threshold = 0;     /**< location of the Chrerenkov threshold from the LL scan */

    float m_mostLikelySignalPhotonCount = 0; /**< Expected number of signal photons at the LL max */
    float m_mostLikelyBackgroundPhotonCount = 0; /**< Expected number of background photons at the LL max */
    float m_mostLikelyDeltaPhotonCount = 0; /**< Expected number of delta rays photons at the LL max */

    std::vector<float> m_coarseScanMassPoints; /**< mass points used in the coarse scan */
    std::vector<float> m_fineScanMassPoints; /**< mass points used in the fine scan */

    std::vector<float> m_coarseScanLLValues; /**< LL values of the coarse scan */
    std::vector<float> m_fineScanLLValues; /**< LL values of the fine scan */

    std::vector<float> m_coarseScanExpectedPhotons; /**< expected signal photons from the coarse scan */
    std::vector<float> m_fineScanExpectedPhotons; /**< expected signal photons from the fine scan */

    ClassDef(TOPLikelihoodScanResult, 1); /**< ClassDef */

  };


} // end namespace Belle2

