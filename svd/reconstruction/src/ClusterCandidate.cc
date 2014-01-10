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
      float charge = sample.getCharge();
      unsigned int strip = sample.getCellID();
      unsigned int time = sample.getSampleIndex();
      if (m_charges.count(strip) > 0) {
        if (charge > m_charges[strip]) {
          m_charge += charge - m_charges[strip];
          m_charges[strip] = charge;
          m_maxima[strip] = time;
        }
        m_counts[strip] += 1;
      } else {
        m_charges[strip] = charge;
        m_charge += charge;
        m_maxima[strip] = time;
        m_counts[strip] = 1;
      }
      m_samples.push_back(sample);
      if (m_seed.getCharge() < charge) {
        m_seed = sample;
      }
    }

    void ClusterCandidate::merge(ClusterCandidate& cls)
    {
      if (&cls == this) return;

      if (cls.m_seed.getCharge() > m_seed.getCharge()) {
        m_seed = cls.m_seed;
      }
      for (const Sample & sample : cls.m_samples) {
        add(sample);
      }
      cls.m_samples.clear();
      cls.m_charges.clear();
      cls.m_counts.clear();
      cls.m_charge = 0;
    }

  }
} //Belle2 namespace
