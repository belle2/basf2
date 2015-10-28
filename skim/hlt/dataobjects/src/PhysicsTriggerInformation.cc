/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/hlt/dataobjects/PhysicsTriggerInformation.h>

//using namespace std;
using namespace Belle2;

PhysicsTriggerInformation::PhysicsTriggerInformation(): m_ngoodTracks(0), m_ngoodECLClusters(0), m_Pzvis(0.), m_Evis(0.),
  m_Esum(0.),
  m_MaxAngleTT(-10.), m_AngleTT(-10.), m_AngleGG(-10.), m_P1(0.), m_P2(0.), m_E1(0.), m_E2(0.), m_nKLMClusters(0), m_ptsummary(0),
  m_ntottrg(0)
{
  for (int i = 0; i < 16; i++) {
    m_trgresult[i] = 0;
    m_nsubtrg[i] = 0;
  }
}
//ClassImp(PhysicsTriggerInformation)


