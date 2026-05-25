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

void TauPairDecay::addTauMinusMasses2Body(const std::vector<double>& masses)
{
  m_masses2BMinus = masses;
}

void TauPairDecay::addTauMinusMasses3Body(const std::vector<double>& masses)
{
  m_masses3BMinus = masses;
}

void TauPairDecay::addTauMinusMasses4Body(const std::vector<double>& masses)
{
  m_masses4BMinus = masses;
}

void TauPairDecay::addTauPlusMasses2Body(const std::vector<double>& masses)
{
  m_masses2BPlus = masses;
}

void TauPairDecay::addTauPlusMasses3Body(const std::vector<double>& masses)
{
  m_masses3BPlus = masses;
}

void TauPairDecay::addTauPlusMasses4Body(const std::vector<double>& masses)
{
  m_masses4BPlus = masses;
}
