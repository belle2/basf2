/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
