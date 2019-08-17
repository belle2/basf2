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
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;

Eigen::Matrix<double, 3, 1> Btube::getTubeCenter() const
{
  return m_tubecenter;
}

Eigen::Matrix<double, 3, 1> Btube::getTubeDirection() const
{
  return m_tubecenter;
}

TMatrixFSym Btube::getTubeMatrix() const
{
  return m_tubematrix;
}

TMatrixFSym Btube::getTubeCenterErrorMatrix() const
{
  return m_tubecentererrormatrix;
}

void Btube::setTubeCenter(const Eigen::Matrix<double, 3, 1>& tubecenter)
{
  m_tubecenter = tubecenter;
}

void Btube::setTubeDirection(const Eigen::Matrix<double, 3, 1>& tubedirection)
{
  m_tubedirection = tubedirection;
}

void Btube::setTubeMatrix(const TMatrixFSym& tubematrix)
{
  m_tubematrix = tubematrix;
}

void Btube::setTubeCenterErrorMatrix(const TMatrixFSym& tubecentererrormatrix)
{
  m_tubecentererrormatrix = tubecentererrormatrix;
}
void  Btube::resetTubeMatrix()
{
  TMatrixFSym temp(3);
  m_tubematrix.ResizeTo(temp);
  m_tubematrix = temp;
}
void  Btube::resetTubeCenterErrorMatrix()
{
  TMatrixFSym temp2(3);
  m_tubecentererrormatrix.ResizeTo(temp2);
  m_tubecentererrormatrix = temp2;
}
