/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
