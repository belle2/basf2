/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/BeamParameters.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;

void BeamParameters::setLER(double energy, double angle, const std::vector<double>& cov)
{
  TLorentzVector vec = getFourVector(energy, angle);
  TMatrixDSym matrix = getCovMatrix(cov, false);
  setLER(vec);
  setCovLER(matrix);
}

void BeamParameters::setHER(double energy, double angle, const std::vector<double>& cov)
{
  TLorentzVector vec = getFourVector(energy, angle);
  TMatrixDSym matrix = getCovMatrix(cov, false);
  setHER(vec);
  setCovHER(matrix);
}

void BeamParameters::setVertex(const TVector3& vertex, const std::vector<double>& cov)
{
  TMatrixDSym matrix = getCovMatrix(cov, true);
  setVertex(vertex);
  setCovVertex(matrix);
}

TLorentzVector BeamParameters::getFourVector(double energy, double angle)
{
  double pz = sqrt(energy * energy - Const::electronMass * Const::electronMass);
  TLorentzVector vec(0, 0, pz, energy);
  if (angle < 0) angle = M_PI + angle;
  vec.RotateY(angle);
  return vec;
}

TMatrixDSym BeamParameters::getCovMatrix(const std::vector<double>& cov, bool common)
{
  TMatrixDSym matrix(3);
  // so let's see how many elements we got
  switch (cov.size()) {
    case 0: // none, ok, no errors then
      break;
    case 1: // either just first value or common value for diagonal elements
      if (!common) {
        matrix(0, 0) = cov[0];
        break;
      }
    case 3: // diagonal form.
      // we can do both at once by using cov[i % cov.size()] which will either
      // loop trough 0, 1, 2 if size is 3 or will always be 0
      for (int i = 0; i < 3; ++i) {
        matrix(i, i) = cov[i % cov.size()];
      }
      break;
    case 6: // upper triangle, i.e. (0, 0), (0, 1), (0, 2), (1, 1), (1, 2), (2, 2)
      for (int i = 0, n = 0; i < 3; ++i) {
        for (int j = i; j < 3; ++j) {
          matrix(i, j) = cov[n++];
        }
      }
      break;
    case 9: // all elements
      for (int i = 0; i < 9; ++i) {
        matrix(i / 3, i % 3) = cov[i];
      }
      break;
    default:
      B2ERROR("Number of elements to set covariance matrix must be either 1, 3, 6 or 9 but "
              << cov.size() << " given");
  }
  return matrix;
}

ClassImp(BeamParameters);
