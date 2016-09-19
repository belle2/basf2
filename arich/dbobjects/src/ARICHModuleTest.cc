/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHModuleTest.h>

using namespace Belle2;
using namespace std;

int ARICHModuleTest::getDeadCh(unsigned int i) const
{
  if (i < m_deadCh.size()) return m_deadCh[i];
  else return 0;
}

int ARICHModuleTest::getStrangeCh(unsigned int i) const
{
  if (i < m_deadCh.size()) return m_deadCh[i];
  else return 0;
}

TGraph* ARICHModuleTest::getChipVdiffTH(unsigned int i) const
{
  if (i < 4) return m_chipVdiff_th[i];
  else return NULL;
}

void ARICHModuleTest::setChipVdiffTH(unsigned int i, TGraph* chipVdiff_th)
{
  if (i < 4) m_chipVdiff_th[i] = chipVdiff_th;
}

TGraph* ARICHModuleTest::getChipLeakTH(unsigned int i) const
{
  if (i < 4) return m_chipLeak_th[i];
  else return NULL;
}

void ARICHModuleTest::setChipLeakTH(unsigned int i, TGraph* chipLeak_th)
{
  if (i < 4) m_chipLeak_th[i] = chipLeak_th;
}

TGraph* ARICHModuleTest::getChipVdiff2Dx(unsigned int i) const
{
  if (i < 4) return m_chipVdiff_2Dx[i];
  else return NULL;
}

void ARICHModuleTest::setChipVdiff2Dx(unsigned int i, TGraph* chipVdiff_2Dx)
{
  if (i < 4) m_chipVdiff_2Dx[i] = chipVdiff_2Dx;
}

TGraph* ARICHModuleTest::getChipLeak2Dx(unsigned int i) const
{
  if (i < 4) return m_chipLeak_2Dx[i];
  else return NULL;
}

void ARICHModuleTest::setChipLeak2Dx(unsigned int i, TGraph* chipLeak_2Dx)
{
  if (i < 4) m_chipLeak_2Dx[i] = chipLeak_2Dx;
}

TGraph* ARICHModuleTest::getChipVdiff2Dy(unsigned int i) const
{
  if (i < 4) return m_chipVdiff_2Dy[i];
  else return NULL;
}

void ARICHModuleTest::setChipVdiff2Dy(unsigned int i, TGraph* chipVdiff_2Dy)
{
  if (i < 4) m_chipVdiff_2Dy[i] = chipVdiff_2Dy;
}

TGraph* ARICHModuleTest::getChipLeak2Dy(unsigned int i) const
{
  if (i < 4) return m_chipLeak_2Dy[i];
  else return NULL;
}

void ARICHModuleTest::setChipLeak2Dy(unsigned int i, TGraph* chipLeak_2Dy)
{
  if (i < 4) m_chipLeak_2Dy[i] = chipLeak_2Dy;
}

