/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dbobjects/BeamSpot.h>

using namespace std;
using namespace Belle2;

void BeamSpot::setVertex(const TVector3& vertex,
                         const TMatrixDSym& covariance)
{
  m_position[0] = vertex.X();
  m_position[1] = vertex.Y();
  m_position[2] = vertex.Z();
  setCovMatrix(m_positionError, covariance);
  m_initialized = false;
}

TMatrixDSym BeamSpot::getCovMatrix(const Double32_t* member)
{

  TMatrixDSym matrix(3);
  for (int iRow = 0; iRow < 3; ++iRow) {
    for (int iCol = iRow; iCol < 3; ++iCol) {
      matrix(iCol, iRow) = matrix(iRow, iCol) = * (member ++);
    }
  }
  return matrix;
}


void BeamSpot::setCovMatrix(Double32_t* matrix, const TMatrixDSym& cov)
{

  for (int iRow = 0; iRow < 3; ++iRow) {
    for (int iCol = iRow; iCol < 3; ++iCol) {
      *(matrix ++)  = cov(iRow, iCol);
    }
  }
}


void Belle2::BeamSpot::initialize(void)
{
  m_positionCached.SetXYZ(m_position[0] ,
                          m_position[1] ,
                          m_position[2]);

  m_positionErrorCached = getCovMatrix(m_positionError);
  m_sizeCached = getCovMatrix(m_size);
  m_initialized = true;
}

