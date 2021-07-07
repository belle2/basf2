/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/ContinuumSuppression/SphericityEigenvalues.h>
#include <framework/logging/Logger.h>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <string>

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


  if (m_momenta.size() < 2) {
    B2WARNING("The particle list has less than 2 elements. The sphericity matrix will not be calculated");
    return;
  }

  for (const auto& p : m_momenta) {
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
  Eigen::ComplexEigenSolver<Eigen::MatrixXcf> ces(sphericityTensor);

  // unfortunately Eigen does not provide the eigenvalues in
  // any specific order, so we have to sort them and keep also the correct eigenvector-eigenvalue
  // associations...

  short order[3] = {0, 1, 2};

  std::vector<float> tmpLambda;
  for (short i = 0; i < 3; i++) {
    tmpLambda.push_back(eigenVals[i].real());
  }

  // position of the largest Eigenvalue
  order[0] =  std::distance(tmpLambda.begin(), std::max_element(tmpLambda.begin(), tmpLambda.end()));
  // position of the smallest Eigenvalue
  order[2] =  std::distance(tmpLambda.begin(), std::min_element(tmpLambda.begin(), tmpLambda.end()));
  // position of the middle eigenvalue
  order[1] = (short)(3.1 - (order[0] + order[2]));

  for (short i = 0; i < 3; i++) {
    short n = order[i];
    m_lambda[i] = eigenVals[n].real();
    auto eigenVector =  ces.eigenvectors().col(n);
    m_eVector[i].SetX(eigenVector[0].real());
    m_eVector[i].SetY(eigenVector[1].real());
    m_eVector[i].SetZ(eigenVector[2].real());
  }

  return;
}

