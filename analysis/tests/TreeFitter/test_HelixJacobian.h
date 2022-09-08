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

#include <framework/gearbox/Const.h>

#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/gearbox/Unit.h>

namespace {

  /** Test fixture. */
  class TreeFitterHelixJacobianTest : public ::testing::Test {

  };

  /** Ensure our helix definition is consistent with the framework */
  TEST_F(TreeFitterHelixJacobianTest, Helix)
  {
    const double eps = 1E-6;
    //random start values to ensure x and p vector are not orthogonal/parallel
    const double px = 0.123214;
    const double py = 1.543;
    const double pz = -2.876;
    const double x = 3.765346;
    const double y = -2.56756;
    const double z = 1.678;

    const int charge = -1;
    const double bfield = Belle2::BFieldManager::getFieldInTesla(Belle2::B2Vector3D(0, 0, 0)).Z();
    const double alpha = 1.0 / (bfield * Belle2::Const::speedOfLight) * 1E4;
    const double aq = charge / alpha;

    Belle2::Helix helix = Belle2::Helix(Belle2::B2Vector3D(x, y, z), Belle2::B2Vector3D(px, py, pz), charge, bfield);

    const double pt = std::sqrt(px * px + py * py);
    const double omega = aq / pt;

    const double phi = atan(py / px);
    const double cosPhi = std::cos(phi);
    const double sinPhi = std::sin(phi);

    const double para = -x * cosPhi - y * sinPhi;
    const double ortho = -y * cosPhi + x * sinPhi;
    const double R2 = para * para + ortho * ortho;
    const double A = 2 * ortho + omega * R2;
    const double U = std::sqrt(1 + omega * A);
    const double d0 = A / (1 + U);
    const double l = 1 / omega * atan((omega * para) / (1 + omega * ortho));
    const double phi0 = phi + atan((omega * para) / (1 + omega * ortho));
    const double tanLambda = pz / pt;
    const double z0 = z + tanLambda * l;

    std::vector<double> h = {d0, phi0, omega, z0, tanLambda};
    std::vector<double> h_framework = {helix.getD0(), helix.getPhi0(), helix.getOmega(), helix.getZ0(), helix.getTanLambda()};
    for (int row = 0; row < 5; ++row) {
      double res = h[row] - h_framework[row];
      EXPECT_TRUE(res < eps) << "row " << row  << " num - ana " << res << " framework " << h_framework[row] << " mine " << h[row];
    }
  }

  /** Ensure our jacobian definition is correct by comparing it to a numerical approach */
  TEST_F(TreeFitterHelixJacobianTest, Parameters)
  {
    const double delta = 1e-6;
    const double eps = 1e-5;

    Eigen::Matrix<double, 5, 6> jacobian_numerical = Eigen::Matrix<double, 5, 6>::Zero(5, 6);
    Eigen::Matrix<double, 5, 6> jacobian_analytical = Eigen::Matrix<double, 5, 6>::Zero(5, 6);

    Belle2::B2Vector3D postmp;
    Belle2::B2Vector3D momtmp;

    //random start values to ensure x and p vector are not orthogonal/parallel
    const double px = 0.523214;
    const double py = -1.543;
    const double pz = -2.876;
    const double x = -3.765346;
    const double y = -2.56756;
    const double z = 5.678;

    const Eigen::Matrix<double, 1, 6> positionAndMom_ = (Eigen::Matrix<double, 1, 6>() << x, y, z, px, py, pz).finished();
    const int charge = -1;
    const double bfield = Belle2::BFieldManager::getFieldInTesla(Belle2::B2Vector3D(0, 0, 0)).Z();

    Belle2::Helix helix = Belle2::Helix(Belle2::B2Vector3D(x, y, z), Belle2::B2Vector3D(px, py, pz), charge, bfield);

    for (int jin = 0; jin < 6; ++jin) {
      for (int i = 0; i < 3; ++i) {
        postmp[i] = positionAndMom_(i);
        momtmp[i] = positionAndMom_(i + 3);
      }
      if (jin < 3) {
        postmp[jin] += delta;
      } else {
        momtmp[jin - 3] += delta;
      }

      Belle2::Helix helixPlusDelta(postmp, momtmp, charge, bfield);

      jacobian_numerical(0, jin) = (helixPlusDelta.getD0() - helix.getD0()) / delta;
      jacobian_numerical(1, jin) = (helixPlusDelta.getPhi0() - helix.getPhi0()) / delta;
      jacobian_numerical(2, jin) = (helixPlusDelta.getOmega() - helix.getOmega()) / delta;
      jacobian_numerical(3, jin) = (helixPlusDelta.getZ0() - helix.getZ0()) / delta;
      jacobian_numerical(4, jin) = (helixPlusDelta.getTanLambda() - helix.getTanLambda()) / delta;
    }

    TreeFitter::HelixUtils::getJacobianToCartesianFrameworkHelix(jacobian_analytical, x, y, z, px, py, pz, bfield, charge);

    for (int row = 0; row < 5; ++row) {
      for (int col = 0; col < 6; ++col) {
        const double num = jacobian_numerical(row, col);
        const double ana = jacobian_analytical(row, col);
        const double res = std::abs(num - ana);
        EXPECT_TRUE(res < eps) << "row " << row << " col " << col << " num - ana " << res << " num " << num << " ana " << ana;
      }
    }
  }

}
