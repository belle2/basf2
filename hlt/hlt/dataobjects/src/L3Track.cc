/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/hlt/dataobjects/L3Track.h>

using namespace Belle2;

//ClassImp(L3Track)

L3Track::L3Track()
{
  for (int i = 0; i < 5; i++) m_helix[i] = 0;
}

L3Track::L3Track(const TVectorD& h)
{
  for (int i = 0; i < 5; i++) m_helix[i] = h[i];
}

void
L3Track::setHelix(const TVectorD& h)
{
  for (int i = 0; i < 5; i++) m_helix[i] = h[i];
}
