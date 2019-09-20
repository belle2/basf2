/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey, Abi Soffer                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <analysis/dataobjects/Btube.h>

using namespace Belle2;

Eigen::Matrix<double, 3, 1> Btube::getTubeCenter()
{
  return m_tubecenter;
}

TMatrixFSym Btube::getTubeMatrix()
{
  return m_tubematrix;
}

void Btube::setTubeCenter(const Eigen::Matrix<double, 3, 1>& tubecenter)
{
  m_tubecenter = tubecenter;
}

void Btube::setTubeMatrix(const TMatrixFSym& tubematrix)
{
  m_tubematrix = tubematrix;
}

void  Btube::resetTubeMatrix()
{
  TMatrixFSym temp(3);
  m_tubematrix.ResizeTo(temp);
  m_tubematrix = temp;
}
