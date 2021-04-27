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
#include <algorithm>
#include <TRandom.h>


namespace Belle2 {

  void BunchStructure::setBucket(unsigned i)
  {
    if (m_fillPattern.empty()) m_fillPattern.resize(c_RFBuckets, false);

    m_fillPattern[i % c_RFBuckets] = true;
    m_filledBuckets.clear();
  }


  bool BunchStructure::getBucket(unsigned i) const
  {
    if (m_fillPattern.empty()) return (i % 2 == 0);

    return m_fillPattern[i % c_RFBuckets];
  }


  unsigned BunchStructure::getNumOfFilledBuckets() const
  {
    if (m_fillPattern.empty()) return (c_RFBuckets / 2);

    return std::count(m_fillPattern.begin(), m_fillPattern.end(), true);
  }


  unsigned BunchStructure::generateBucketNumber() const
  {
    if (m_fillPattern.empty()) return (gRandom->Integer(c_RFBuckets / 2) * 2);

    if (m_filledBuckets.empty()) {
      for (unsigned i = 0; i < c_RFBuckets; ++i) {
        if (m_fillPattern[i]) m_filledBuckets.push_back(i);
      }
    }

    int index = gRandom->Integer(m_filledBuckets.size());
    return m_filledBuckets[index];
  }

} //Belle2 namespace
