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

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <vector>

namespace Belle2::SVD {

  /**
   * Class implementing the MaxSum algorithm
   */
  class SVDMaxSumAlgorithm {

  public:


    /**
     * Constructor with the APVFloatSamples
     */
    explicit SVDMaxSumAlgorithm(const Belle2::SVDShaperDigit::APVFloatSamples samples)
      : m_samples(samples)
    {
      applyAlgorithm();
    };


    /**
     * virtual destructor
     */
    virtual ~SVDMaxSumAlgorithm() {};

    /**
     * @return the first frame of the 3 selected samples
     */
    int getFirstFrame() {return m_result.first;}

    /**
     * @return the 3 selected sample
     */
    std::vector<float> getSelectedSamples() {return m_result.second;}


  protected:

    /** strip samples*/
    Belle2::SVDShaperDigit::APVFloatSamples m_samples; /** ADC of the acquired samples*/

    /** first frame, 3 selected sample */
    std::pair<int, std::vector<float>> m_result;

    /**
     * applies the algorithm
     */
    void applyAlgorithm()
    {

      //Max Sum selection
      if (m_samples.size() < 3)
        B2ERROR("APV25 samples less than 3!?");

      std::vector<float> Sum2bin(m_samples.size() - 1, 0);

      for (int iBin = 0; iBin < static_cast<int>(m_samples.size()) - 1; ++iBin)
        Sum2bin.at(iBin) = m_samples.at(iBin) + m_samples.at(iBin + 1);

      auto itSum = std::max_element(std::begin(Sum2bin), std::end(Sum2bin));

      int ctrFrame = std::distance(std::begin(Sum2bin), itSum);

      if (ctrFrame == 0) ctrFrame = 1;

      std::vector<float> selectedSamples = {m_samples.at(ctrFrame - 1), m_samples.at(ctrFrame), m_samples.at(ctrFrame + 1)};

      m_result.first = ctrFrame - 1;
      m_result.second =  selectedSamples;

    };

  };

}

