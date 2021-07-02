/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <mva/methods/Regression.h>
#include <mva/methods/FastBDT.h>

#include <string>

namespace Belle2 {
  namespace MVA {
    /// Explicit template specification for FastBDTs for regression options
    class RegressionFastBDTOptions : public RegressionOptions<FastBDTOptions> {
    public:
      /// Return a correct name for this method
      std::string getMethod() const override { return "RegressionFastBDT"; }
    };

    /// Explicit template specification for FastBDTs for regression teachers
    using RegressionFastBDTTeacher = RegressionTeacher<FastBDTTeacher, RegressionFastBDTOptions>;

    /// Explicit template specification for FastBDTs for regression experts
    using RegressionFastBDTExpert = RegressionExpert<FastBDTExpert, RegressionFastBDTOptions>;
  }
}
