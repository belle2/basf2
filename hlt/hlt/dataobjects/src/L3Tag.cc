/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/hlt/dataobjects/L3Tag.h>

using namespace Belle2;

ClassImp(L3Tag)

L3Tag::L3Tag() : m_SummaryWord(0), m_EnergySum(0.), m_nTracks(0) {}

L3Tag::L3Tag(int summaryWord)
  : m_SummaryWord(summaryWord), m_EnergySum(0.), m_nTracks(0) {}

L3Tag::L3Tag(int summaryWord, double Esum, int nTrks)
  : m_SummaryWord(summaryWord), m_EnergySum(Esum), m_nTracks(nTrks) {}
