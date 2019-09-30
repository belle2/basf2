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
