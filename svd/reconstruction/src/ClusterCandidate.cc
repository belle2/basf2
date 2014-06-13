/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/ClusterCandidate.h>
#include <algorithm>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void ClusterCandidate::add(const Sample& sample)
    {
      const unsigned int strip = sample.getCellID();
      const unsigned int index = sample.getSampleIndex();
      const float charge = sample.getCharge();
      if (m_charges.count(strip) > 0) {
        if (charge > m_charges[strip]) {
          m_charge += charge - m_charges[strip];
          m_charges[strip] = charge;
          m_maxima[strip] = index;
        }
        m_counts[strip] += 1;
      } else {
        m_charges[strip] = charge;
        m_charge += charge;
        m_maxima[strip] = index;
        m_counts[strip] = 1;
      }
      m_samples.push_back(sample);
      if (m_seed.getCharge() < charge) {
        m_seed = sample;
      }

      const float prev_charge = sample.getPrevCharge();
      const float next_charge = sample.getNextCharge();
      const float time   = static_cast<float>(index);

      if (prev_charge < 0 && next_charge < 0) return;

      if (index == m_maxima[strip]) {
        const float a_0 = prev_charge;
        const float a_1 = charge;
        const float a_2 = next_charge;

        float t_0 = -9999.0;
        float q_0 = -9999.0;
        if (a_0 < 0) {
          t_0 = 0.0;
          q_0 = a_1;
        } else if (a_2 < 0) {
          t_0 = 5.0;
          q_0 = a_1;
        } else {
          float denominator = 2 * a_1 - a_0 - a_2;
          t_0 = (denominator > 0) ? 0.5 * (a_2 - a_1) / denominator : 0.0;
          q_0 = (denominator > 0) ? a_1 + 0.125 * (a_2 - a_0) * (a_2 - a_0) / denominator : a_1;
        }

        if (m_qfCharges.count(strip) > 0) {
          m_qfCharge += q_0 - m_qfCharges[strip];
        } else {
          m_qfCharge += q_0;
        }
        m_qfCharges[strip] = q_0;
        m_qfTimes[strip]   = time + t_0;
        struct adjacentCharge adCharge;
        adCharge.prev_charge = prev_charge;
        adCharge.next_charge = next_charge;
        m_adjacentCharges[strip] = adCharge;

        if (m_qfSeedCharge < q_0) {
          m_qfSeedCharge = q_0;
        }
      }//if (index==m_maxima[strip]) {

    }

    void ClusterCandidate::merge(ClusterCandidate& cls)
    {
      if (&cls == this) return;
      if (cls.m_seed.getCharge() > m_seed.getCharge()) {
        m_seed = cls.m_seed;
      }
      for (const Sample & sample : cls.m_samples) {
        unsigned int strip = sample.getCellID();
        unsigned int index = sample.getSampleIndex();
        unsigned int max_index = cls.m_maxima[strip];
        if (index == max_index) {
          this->add(sample);
        } else {
          this->add(sample);
        }
      }
      cls.m_samples.clear();
      cls.m_charges.clear();
      cls.m_counts.clear();
      cls.m_charge = 0;
      cls.m_qfCharges.clear();
      cls.m_qfTimes.clear();
      cls.m_adjacentCharges.clear();
      cls.m_qfCharge = 0;
      cls.m_qfSeedCharge = 0;
    }

  }
} //Belle2 namespace
