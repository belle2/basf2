/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPDataQualityOnline/TOPDQStat.h>
#include <math.h>

using namespace std;

namespace Belle2 {
  TOPDQStat::TOPDQStat() {}

  TOPDQStat::TOPDQStat(int v_size)
  {
    for (int i = 0; i < v_size; i++) {
      n.push_back(0);
      mean.push_back(0.);
      mean2.push_back(0.);
    }
  }

  void TOPDQStat::SetSize(int v_size)
  {
    n.clear();
    mean.clear();
    mean2.clear();
    for (int i = 0; i < v_size; i++) {
      n.push_back(0);
      mean.push_back(0.);
      mean2.push_back(0.);
    }
  }

  void TOPDQStat::Add(int idx, double val)
  {
    mean[idx] = (n[idx] * mean[idx] + val) / (n[idx] + 1);
    mean2[idx] = (n[idx] * mean2[idx] + val * val) / (n[idx] + 1);
    n[idx] += 1;
  }

  double TOPDQStat::GetMean(int idx)
  {
    return mean[idx];
  }

  double TOPDQStat::GetRMS(int idx)
  {
    return sqrt(mean2[idx] - mean[idx] * mean[idx]);
  }
} // end Belle2 namespace
