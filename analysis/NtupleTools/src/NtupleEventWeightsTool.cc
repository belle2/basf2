/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleEventWeightsTool.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <string>

using namespace Belle2;
using namespace std;

void NtupleEventWeightsTool::setupTree()
{
  m_fWeight = 1.0;
  m_tree->Branch("weight", &m_fWeight, "weight/F");
}

void NtupleEventWeightsTool::eval(const  Particle*)
{
  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    m_fWeight = eventmetadata->getGeneratedWeight();
  } else {
    m_fWeight = 1.0;
  }
}
