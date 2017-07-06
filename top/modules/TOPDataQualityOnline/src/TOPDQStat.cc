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
      sum.push_back(0.);
      sum2.push_back(0.);
    }
  }

  void TOPDQStat::SetSize(int v_size)
  {
    n.clear();
    sum.clear();
    sum2.clear();
    for (int i = 0; i < v_size; i++) {
      n.push_back(0);
      sum.push_back(0.);
      sum2.push_back(0.);
    }
  }

  void TOPDQStat::Add(int idx, double val)
  {
    sum[idx] += val;
    sum2[idx] += val * val;
    n[idx] += 1;
  }

  double TOPDQStat::GetMean(int idx)
  {
    if (n[idx] == 0) return 0;
    else return sum[idx] / n[idx];
  }

  double TOPDQStat::GetRMS(int idx)
  {
    if (n[idx] == 0) return 0;
    else return sqrt(sum2[idx] / n[idx] - pow(sum[idx] / n[idx], 2));
  }
} // end Belle2 namespace
