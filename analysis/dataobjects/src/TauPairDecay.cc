/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/TauPairDecay.h>

using namespace Belle2;

void TauPairDecay::addTauPlusIdMode(int pmode)
{
  m_pmode = pmode;
}

void TauPairDecay::addTauMinusIdMode(int mmode)
{
  m_mmode = mmode;
}

void TauPairDecay::addTauPlusMcProng(int pprong)
{
  m_pprong = pprong;
}

void TauPairDecay::addTauMinusMcProng(int mprong)
{
  m_mprong = mprong;
}
