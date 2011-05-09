/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Andreas Moll                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/EventMetaData.h>

using namespace std;
using namespace Belle2;

ClassImp(EventMetaData)


bool EventMetaData::operator== (const EventMetaData& eventMetaData) const
{
  return ((m_event == eventMetaData.getEvent()) &&
          (m_run == eventMetaData.getRun()) &&
          (m_experiment == eventMetaData.getExperiment()));
}

bool EventMetaData::operator!= (const EventMetaData& eventMetaData) const
{
  return !(*this == eventMetaData);
}
