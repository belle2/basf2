/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "analysis/VertexFitting/TreeFitter/FitParams.h"

namespace {

  /** Test fixture. */
  class TreeFitterFitparTest : public ::testing::Test {
  protected:
  };

  TEST_F(TreeFitterFitparTest, Constructor)
  {
    TreeFitter::FitParams fitParDim3(3);

    /** all elments of the statevector initialised to 0  */
    EXPECT_TRUE((fitParDim3.getStateVector().array() == 0.0).all()) << "statevector not initialised to 0,0,...";

    /** all covariance matrix elemtns initalised to 0 */
    EXPECT_TRUE((fitParDim3.getCovariance().array() == 0.0).all()) << "covariance not initialised to 0";
  }

  /** test the data member that holds fit parameters */
  TEST_F(TreeFitterFitparTest, Functions)
  {
    TreeFitter::FitParams fitParDim3(3);

    // cppcheck-suppress constStatement
    fitParDim3.getStateVector() << 1, 2, 3;
    fitParDim3.resetStateVector();
    EXPECT_TRUE((fitParDim3.getStateVector().array() == 0.0).all()) << "fitParDim3.resetStateVector() didn't work";

    // cppcheck-suppress constStatement
    fitParDim3.getCovariance() << 1, 2, 3, 4, 5, 6, 7, 8, 9;
    fitParDim3.resetCovariance();
    EXPECT_TRUE((fitParDim3.getCovariance().array() == 0.0).all()) << "fitParDim3.resetCovariance() didn't work";

    EXPECT_FALSE(fitParDim3.testCovariance()) << "fitParDim3.testCovariance() failed";


  }

}  // namespace
