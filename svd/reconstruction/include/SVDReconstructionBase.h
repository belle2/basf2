/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <set>

namespace Belle2::SVD {

  /**
   * Class to check whether the reconstruction algorithms are available or not
   */
  class SVDReconstructionBase {

  public:

    /**
     * default copy constructor
     */
    SVDReconstructionBase(const SVDReconstructionBase&) = default;

    /**
     * default constructor
     */
    explicit SVDReconstructionBase() { buildAlgorithmSets(); };

    /**
     * virtual destructor
     */
    virtual ~SVDReconstructionBase() {};

    /**
     * @return true if the timeAlg is implemented and available for reconstruction
     */
    bool isTimeAlgorithmAvailable(TString timeAlg)
    {
      return m_timeAlgorithms.find(timeAlg) != m_timeAlgorithms.end();
    }

    /**
     * @return true if the chargeAlg is implemented and available for reconstruction
     */
    bool isChargeAlgorithmAvailable(TString chargeAlg)
    {
      return m_chargeAlgorithms.find(chargeAlg) != m_chargeAlgorithms.end();
    }

    /**
     * @return true if the positionAlg is implemented and available for reconstruction
     */
    bool isPositionAlgorithmAvailable(TString positionAlg)
    {
      return m_positionAlgorithms.find(positionAlg) != m_positionAlgorithms.end();
    }


  protected:


    /** set containing the available time algorithms */
    std::set<TString> m_timeAlgorithms;

    /** set containing the available charge algorithms */
    std::set<TString> m_chargeAlgorithms;

    /** set containing the available position algorithms */
    std::set<TString> m_positionAlgorithms;

    /** build the sets containing the available time, charge and position algorithms */
    void buildAlgorithmSets()
    {
      m_timeAlgorithms.insert("CoG6");
      m_timeAlgorithms.insert("CoG3");
      m_timeAlgorithms.insert("ELS3");

      m_chargeAlgorithms.insert("MaxSample");
      m_chargeAlgorithms.insert("SumSamples");
      m_chargeAlgorithms.insert("ELS3");

      m_positionAlgorithms.insert("CoGOnly");
      m_positionAlgorithms.insert("OldDefault");
    }

  };

}

