/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/RunMetaData.h>

using namespace std;
using namespace Belle2;

ClassImp(RunMetaData)


bool RunMetaData::operator== (const RunMetaData& eventMetaData) const
{
  return (/*(m_event == eventMetaData.getEvent()) &&
          (m_run == eventMetaData.getRun()) &&
          (m_experiment == eventMetaData.getExperiment())*/
           true
         );
}

bool RunMetaData::operator!= (const RunMetaData& eventMetaData) const
{
  return !(/* *this == eventMetaData*/
           true);
}
