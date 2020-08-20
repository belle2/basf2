/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dbobjects/BunchStructure.h>
#include <TRandom.h>


namespace Belle2 {

  void BunchStructure::setBucket(unsigned i)
  {
    if (m_fillPattern.empty()) m_fillPattern.resize(c_RFBuckets, false);

    m_fillPattern[i % c_RFBuckets] = true;
  }


  bool BunchStructure::getBucket(unsigned i) const
  {
    if (m_fillPattern.empty()) return (i % 2 == 0);

    return m_fillPattern[i % c_RFBuckets];
  }


  unsigned BunchStructure::getNumOfFilledBuckets() const
  {
    if (m_fillPattern.empty()) return (c_RFBuckets / 2);

    unsigned n = 0;
    for (auto x : m_fillPattern) {
      if (x) n++;
    }
    return n;
  }


  unsigned BunchStructure::generateBucketNumber() const
  {
    if (m_fillPattern.empty()) return (gRandom->Integer(c_RFBuckets / 2) * 2);

    if (m_cumulative.empty()) {
      m_cumulative.push_back(0);
      for (auto x : m_fillPattern) {
        if (x) m_cumulative.push_back(1 + m_cumulative.back());
        else m_cumulative.push_back(m_cumulative.back());
      }
    }

    double random = gRandom->Uniform(m_cumulative.back());
    int i1 = 0;
    int i2 = m_cumulative.size() - 1;
    while (i2 - i1 > 1) {
      int i = (i1 + i2) / 2;
      if (random > m_cumulative[i]) {
        i1 = i;
      } else {
        i2 = i;
      }
    }
    return i1;
  }

} //Belle2 namespace
