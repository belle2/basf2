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

void TauPairDecay::addTauMinusEgstar(double megstar)
{
  m_megstar = megstar;
}

void TauPairDecay::addTauPlusEgstar(double pegstar)
{
  m_pegstar = pegstar;
}

void TauPairDecay::addTauMinusDaughters(const std::vector<int>& dau_tauminus)
{
  m_vec_dau_tauminus = dau_tauminus;
}

void TauPairDecay::addTauPlusDaughters(const std::vector<int>& dau_tauplus)
{
  m_vec_dau_tauplus = dau_tauplus;
}
