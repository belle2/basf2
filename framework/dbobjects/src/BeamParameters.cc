/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dbobjects/BeamParameters.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;

namespace {
  /** Access element in triangular matrix including diagonal elements.
   * This function returns the storage index of an element (i,j) in a
   * symmetric matrix including diagonal elements if the elements are
   * stored in a continous array of size n(n+1)/2
   */
  constexpr int getIndex(unsigned int i, unsigned int j)
  {
    //swap indices if i >= j
    return (i < j) ? ((j + 1) * j / 2 + i) : ((i + 1) * i / 2 + j);
  }
}

void BeamParameters::setLER(double energy, double angleX, double angleY, const std::vector<double>& cov)
{
  ROOT::Math::PxPyPzEVector vec = getFourVector(energy, angleX, angleY);
  setLER(vec);
  setCovMatrix(m_covLER, cov, false);
}

void BeamParameters::setHER(double energy, double angleX, double angleY, const std::vector<double>& cov)
{
  ROOT::Math::PxPyPzEVector vec = getFourVector(energy, angleX, angleY);
  setHER(vec);
  setCovMatrix(m_covHER, cov, false);
}

void BeamParameters::setVertex(const TVector3& vertex, const std::vector<double>& cov)
{
  setVertex(vertex);
  setCovMatrix(m_covVertex, cov, true);
}

ROOT::Math::PxPyPzEVector BeamParameters::getFourVector(double energy, double angleX, double angleY)
{
  double p = sqrt(pow(energy, 2) - pow(Const::electronMass, 2));
  if (angleX < 0) {
    angleX = M_PI + angleX;
    angleY = M_PI + angleY;
  }

  double Sign = cos(angleX) < 0 ?  -1 : 1;

  double pzTerm = 1 - pow(sin(angleX), 2) - pow(sin(angleY), 2);
  B2ASSERT("Angular term should be positive", pzTerm >= 0);

  double pz = Sign * p * sqrt(pzTerm);

  return ROOT::Math::PxPyPzEVector(p * sin(angleX), p * sin(angleY), pz, energy);

}

TMatrixDSym BeamParameters::getCovMatrix(const Double32_t* member)
{
  TMatrixDSym matrix(3);
  for (int iRow = 0; iRow < 3; ++iRow) {
    for (int iCol = iRow; iCol < 3; ++iCol) {
      matrix(iCol, iRow) = matrix(iRow, iCol) = member[getIndex(iRow, iCol)];
    }
  }
  return matrix;
}

void BeamParameters::setCovMatrix(Double32_t* matrix, const TMatrixDSym& cov)
{
  for (int iRow = 0; iRow < 3; ++iRow) {
    for (int iCol = iRow; iCol < 3; ++iCol) {
      matrix[getIndex(iRow, iCol)] = cov(iRow, iCol);
    }
  }
}

void BeamParameters::setCovMatrix(Double32_t* matrix, const std::vector<double>& cov, bool common)
{
  std::fill_n(matrix, 6, 0);
  // so let's see how many elements we got
  switch (cov.size()) {
    case 0: // none, ok, no errors then
      break;
    case 1: // either just first value or common value for diagonal elements
      if (!common) {
        matrix[0] = cov[0];
        break;
      }
      // not common value, fall through
      [[fallthrough]];
    case 3: // diagonal form.
      // we can do both at once by using cov[i % cov.size()] which will either
      // loop trough 0, 1, 2 if size is 3 or will always be 0
      for (int i = 0; i < 3; ++i) {
        matrix[getIndex(i, i)] = cov[i % cov.size()];
      }
      break;
    case 6: // upper triangle, i.e. (0, 0), (0, 1), (0, 2), (1, 1), (1, 2), (2, 2)
      for (int iRow = 0, n = 0; iRow < 3; ++iRow) {
        for (int iCol = iRow; iCol < 3; ++iCol) {
          matrix[getIndex(iRow, iCol)] = cov[n++];
        }
      }
      break;
    case 9: // all elements
      for (int iRow = 0; iRow < 3; ++iRow) {
        for (int iCol = iRow; iCol < 3; ++iCol) {
          matrix[getIndex(iRow, iCol)] = cov[iRow * 3 + iCol];
        }
      }
      break;
    default:
      B2ERROR("Number of elements to set covariance matrix must be either 1, 3, 6 or 9 but "
              << cov.size() << " given");
  }
}
