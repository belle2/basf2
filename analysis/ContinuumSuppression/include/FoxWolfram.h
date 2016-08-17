/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Mikihiko Nakao (KEK), Pablo Goldenzweig (KIT)           *
 *   Original module writen by M. Nakao for Belle                         *
 *   Ported to Belle II by P. Goldenzweig                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FOXWOLFRAM_H
#define FOXWOLFRAM_H

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

namespace Belle2 {

  class FoxWolfram {
  public:

    /**
     * Constructor
     */
    FoxWolfram();

    /**
     * Destructor
     */
    ~FoxWolfram() {};

    /**
     * Returns the i-th Fox-Wolfram moment
     */
    double H(int i) { return (i < 0 || i > 4) ? 0 : sum[i]; }

    /**
     * Returns the i-th normalized Fox-Wolfram moment
     */
    double R(int i) { return (i < 0 || i > 4 || sum[0] == 0) ? 0 : sum[i] / sum[0]; }

    /**
     * Utility function for Fox-Wolfram moment calculation
     */
    void add(const double mag, const double costh);

  protected:

    double sum[5]; /** Fox-Wolfram moments */

  };


  FoxWolfram::FoxWolfram()
  {
    for (int i = 0; i < 5; i++)
      sum[i] = 0;
  }

  void FoxWolfram::add(const double mag, const double costh)
  {
    double cost2 = costh * costh;

    sum[0] += mag;
    sum[1] += mag * costh;
    sum[2] += mag * (1.5 * cost2 - 0.5);
    sum[3] += mag * costh * (2.5 * cost2 - 1.5);
    sum[4] += mag * (4.375 * cost2 * cost2 - 3.75 * cost2 + 0.375);
  }

  FoxWolfram foxwolfram(const std::vector<TVector3>& momenta)
  {
    FoxWolfram f;

    decltype(momenta.begin()) p;
    decltype(momenta.begin()) q;

    const auto begin = momenta.begin();
    const auto end = momenta.end();


    for (p = begin; p != end; p++) {
      const TVector3 pvec = (*p);
      double pmag = pvec.Mag();
      for (q = p; q != end; q++) {
        const TVector3 qvec = (*q);
        double mag = pmag * qvec.Mag();
        double costh = pvec.Dot(qvec) / mag;
        if (p != q) mag *= 2;
        f.add(mag, costh);
      }
    }
    return f;
  }

} // Belle2 namespace

#endif
