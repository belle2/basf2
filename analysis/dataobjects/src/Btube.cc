#include <analysis/dataobjects/Btube.h>

#include <framework/datastore/StoreArray.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;

TVector3 Btube::getTubeCenter()
{
  return m_tubecenter;
}

TMatrixFSym Btube::getTubeMatrix()
{
  return m_tubematrix;
}

void Btube::setTubeCenter(const TVector3& tubecenter)
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
