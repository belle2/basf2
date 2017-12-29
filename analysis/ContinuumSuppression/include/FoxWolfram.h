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
#pragma once

#include <TVector3.h>

#include <vector>


namespace Belle2 {
  /** Class to calculate the Fox-Wolfram moments */
  class FoxWolfram {
  public:

    /// Default Constructor
    FoxWolfram() : sum {0, 0, 0, 0, 0} {};

    /// Constructor which calculates the Fox Wolfram Moments from a given set of momenta.
    explicit FoxWolfram(const std::vector<TVector3>& momenta);

    /// Destructor
    ~FoxWolfram() {};

    /// Returns the i-th Fox-Wolfram moment
    double H(int i) { return (i < 0 || i > 4) ? 0 : sum[i]; }

    /// Returns the i-th normalized Fox-Wolfram moment
    double R(int i) { return (i < 0 || i > 4 || sum[0] == 0) ? 0 : sum[i] / sum[0]; }

  private:
    /// Utility function for Fox-Wolfram moment calculation.
    void add(const double mag, const double costh);

    /// Fox-Wolfram moments
    double sum[5];

  };

} // Belle2 namespace
