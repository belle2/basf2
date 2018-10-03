/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Umberto Tamponi (tamponi@to.infn.it)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/ContinuumSuppression/SphericityEigenvalues.h>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Eigenvalues>

#include <string>
#include <sstream>
#include <iostream>

using namespace Belle2;

void SphericityEigenvalues::calculateEigenvalues()
{
  Eigen::Matrix3f sphericityTensor;

  // elements of the matrix, in rows
  // n = r*3+c, using all 0-based indexes:
  // 00 = 0
  // 01 = 1
  // 10 = 3
  // 12 = 5
  // 22 = 8
  // etc...
  // diagonal = 0, 4, 8
  double elements[9] = {0.};

  // normalization
  double norm = 0;

  for (auto p : m_momenta) {
    elements[0] += p.X() * p.X(); // diag
    elements[1] += p.X() * p.Y();
    elements[2] += p.X() * p.Z();

    elements[3] += p.Y() * p.X();
    elements[4] += p.Y() * p.Y(); // diag
    elements[5] += p.Y() * p.Z();

    elements[6] += p.Z() * p.X();
    elements[7] += p.Z() * p.Y();
    elements[8] += p.Z() * p.Z(); // diag
    norm += p.Mag2();
  }

  for (short i = 0; i < 3; i++) {
    for (short j = 0; j < 3; j++) {
      sphericityTensor(i, j) = elements[i * 3 + j] / norm;
    }
  }

  auto eigenVals = sphericityTensor.eigenvalues();
  for (short i = 0; i < 3; i++) {
    m_lambda[i] = eigenVals[i].real();
  }

  Eigen::ComplexEigenSolver<Eigen::MatrixXcf> ces(sphericityTensor);

  for (short i = 0; i < 3; i++) {
    auto eigenVector =  ces.eigenvectors().col(i);
    m_eVector[i].SetX(eigenVector[0].real());
    m_eVector[i].SetY(eigenVector[1].real());
    m_eVector[i].SetZ(eigenVector[2].real());
  }
  return;
}

