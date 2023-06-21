/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {

  /** class to contain the cut on SVDSpacePoint */
  class SVDSpacePointSelectionFunction : public TObject {

  public:

    /** constructor */
    SVDSpacePointSelectionFunction() {};

    /** Determine if the combination of U and V clusters passes the SNR fraction cuts */
    bool passSNRFractionSelection(const std::vector<float>& inputU,
                                  const std::vector<float>& inputV) const
    {
      if (inputU.size() < 3 || inputV.size() < 3) {
        B2ERROR("The size of input vectors for SVDSpacePointSelectionFunction::passSNRFractionSelection is invalid. " <<
                "It must be grater than 2, but inputU.size() = " << inputU.size() << ", inputV.size() = " << inputV.size());
        return false;
      }

      const float totalSNR_U = inputU[0] + inputU[1] + inputU[2];
      const float fraction0_U = inputU[0] / totalSNR_U;
      const float fraction1_U = inputU[1] / totalSNR_U;

      if (fraction0_U > m_fraction0_max) return false;
      if (fraction1_U < m_fraction1_min && fraction0_U > m_fraction0_max_for_fraction1_cut) return false;

      const float totalSNR_V = inputV[0] + inputV[1] + inputV[2];
      const float fraction0_V = inputV[0] / totalSNR_V;
      const float fraction1_V = inputV[1] / totalSNR_V;

      if (fraction0_V > m_fraction0_max) return false;
      if (fraction1_V < m_fraction1_min && fraction0_V > m_fraction0_max_for_fraction1_cut) return false;

      return true;
    };

    /** Set max of SNR0 fraction  */
    void setFraction0Max(float fraction0_max) {m_fraction0_max = fraction0_max;};
    /** Set min of SNR1 fraction  */
    void setFraction1Min(float fraction1_min) {m_fraction1_min = fraction1_min;};
    /** Set max of SNR0 fraction that is used together with SNR1 fraction cut  */
    void setFraction0MaxForFraction1Cut(float fraction0_max_for_fraction1_cut)
    {m_fraction0_max_for_fraction1_cut = fraction0_max_for_fraction1_cut;};

    /** Get max of SNR0 fraction  */
    float getFraction0Max() const {return m_fraction0_max;};
    /** Get min of SNR1 fraction  */
    float getFraction1Min() const {return m_fraction1_min;};
    /** Get max of SNR0 fraction that is used together with SNR1 fraction cut  */
    float getFraction0MaxForFraction1Cut() const {return m_fraction0_max_for_fraction1_cut;};


  private:
    float m_fraction0_max = 0.45; /**< Max of SNR0 fraction */
    float m_fraction1_min = 0.30; /**< Min of SNR1 fraction */

    /** Max of SNR0 fraction that is used together with SNR1 fraction cut */
    float m_fraction0_max_for_fraction1_cut = 0.10;

    ClassDef(SVDSpacePointSelectionFunction, 1) /**< needed by root*/
  };
}
