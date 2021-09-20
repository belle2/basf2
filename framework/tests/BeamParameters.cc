/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/TestHelpers.h>
#include <framework/dbobjects/BeamParameters.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(BeamParameters, BoostUpsilon4SToLab)
  {
    GTEST_SKIP();
    BeamParameters beamparams;
    //some values from Belle
    beamparams.setLER(3.49841, M_PI,  0, std::vector<double>());
    beamparams.setHER(7.99638, 0.022, 0, std::vector<double>());

    const ROOT::Math::PxPyPzEVector upsVec = beamparams.getHER() + beamparams.getLER();
    ROOT::Math::PxPyPzEVector upsVecCMS = beamparams.getLabToCMS() * upsVec;

    const double mUpsilon = 10.5794;
    EXPECT_TRUE(fabs(upsVecCMS.E() - mUpsilon) < 1e-2);
    EXPECT_TRUE(fabs(upsVecCMS.X()) < 1e-15);
    EXPECT_TRUE(fabs(upsVecCMS.Y()) < 1e-15);
    EXPECT_TRUE(fabs(upsVecCMS.Z()) < 2e-15) << fabs(upsVecCMS.Z());

    EXPECT_TRUE(fabs(beamparams.getMass() - mUpsilon) < 1e-2);
  }

  TEST(BeamParameters, BoostIntoCMSAndBack)
  {
    BeamParameters beamparams;
    //some values from Belle
    beamparams.setLER(3.49841, M_PI,  0, std::vector<double>());
    beamparams.setHER(7.99638, 0.022, 0, std::vector<double>());

    auto backAndForth = beamparams.getCMSToLab() * (beamparams.getLabToCMS());
    ROOT::Math::PxPyPzEVector vec(1, 1, 1, 1);
    vec = backAndForth * vec;
    EXPECT_TRUE(fabs(vec.X() - 1) < 1e-15);
    EXPECT_TRUE(fabs(vec.Y() - 1) < 1e-15);
    EXPECT_TRUE(fabs(vec.Z() - 1) < 1e-15);
  }

  /** Check if covariance matrix can be set from TMatrixDSym and that the upper
   * triangle is used */
  TEST(BeamParameters, CovFromMatrix)
  {
    BeamParameters beamparams;
    TMatrixDSym upper(3), sym(3);
    int n(0);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        upper(i, j) = (j >= i) ? ++n : -1;
        if (j >= i) sym(i, j) = sym(j, i) = upper(i, j);
      }
    }
    beamparams.setCovHER(upper);
    EXPECT_EQ(sym, beamparams.getCovHER());
  }

  /** Check that setting Covariance from zero elements results in zero matrix */
  TEST(BeamParameters, ZeroElement)
  {
    BeamParameters beamparams;
    std::vector<double> cov;
    TMatrixDSym matrix(3);
    beamparams.setHER(0, 0, 0, cov);
    beamparams.setVertex(TVector3(0, 0, 0), cov);
    EXPECT_EQ(matrix, beamparams.getCovHER());
    EXPECT_EQ(matrix, beamparams.getCovVertex());
  }

  /** Check that setting covariance from one element gives this element only in
   * (0,0) for beam covariance and in all diagonal places for vertex */
  TEST(BeamParameters, OneElement)
  {
    BeamParameters beamparams;
    for (int i = 0; i < 10; ++i) {
      std::vector<double> cov{1.*i};
      TMatrixDSym beam(3), vertex(3);
      beam(0, 0) = i;
      vertex(0, 0) = i;
      vertex(1, 1) = i;
      vertex(2, 2) = i;
      beamparams.setHER(0, 0, 0, cov);
      beamparams.setVertex(TVector3(0, 0, 0), cov);
      EXPECT_EQ(beam, beamparams.getCovHER());
      EXPECT_EQ(vertex, beamparams.getCovVertex());
    }
  }

  /** Check that setting covariance from three elements always sets the
   * diagonal elements */
  TEST(BeamParameters, ThreeElements)
  {
    BeamParameters beamparams;
    std::vector<double> cov{1., 2., 3.};
    TMatrixDSym matrix(3);
    matrix(0, 0) = 1;
    matrix(1, 1) = 2;
    matrix(2, 2) = 3;
    beamparams.setHER(0, 0, 0, cov);
    beamparams.setVertex(TVector3(0, 0, 0), cov);
    EXPECT_EQ(matrix, beamparams.getCovHER());
    EXPECT_EQ(matrix, beamparams.getCovVertex());
  }

  /** Check that setting the covariance from 6 elements works as expected */
  TEST(BeamParameters, SixElements)
  {
    BeamParameters beamparams;
    std::vector<double> cov{1., 2., 3., 4., 5., 6.};
    TMatrixDSym matrix(3);
    matrix(0, 0) = 1;
    matrix(0, 1) = matrix(1, 0) = 2;
    matrix(0, 2) = matrix(2, 0) = 3;
    matrix(1, 1) = 4;
    matrix(1, 2) = matrix(2, 1) = 5;
    matrix(2, 2) = 6;
    beamparams.setHER(0, 0, 0, cov);
    beamparams.setVertex(TVector3(0, 0, 0), cov);
    EXPECT_EQ(matrix, beamparams.getCovHER());
    EXPECT_EQ(matrix, beamparams.getCovVertex());
  }

  /** Check that setting the covariance from 9 elements works as expected */
  TEST(BeamParameters, NineElements)
  {
    BeamParameters beamparams;
    std::vector<double> cov{1., 2., 3., 4., 5., 6., 7., 8., 9.};
    TMatrixDSym matrix(3);
    matrix(0, 0) = 1;
    matrix(0, 1) = matrix(1, 0) = 2;
    matrix(0, 2) = matrix(2, 0) = 3;
    matrix(1, 1) = 5;
    matrix(1, 2) = matrix(2, 1) = 6;
    matrix(2, 2) = 9;
    beamparams.setHER(0, 0, 0, cov);
    beamparams.setVertex(TVector3(0, 0, 0), cov);
    EXPECT_EQ(matrix, beamparams.getCovHER());
    EXPECT_EQ(matrix, beamparams.getCovVertex());
  }

}  // namespace
